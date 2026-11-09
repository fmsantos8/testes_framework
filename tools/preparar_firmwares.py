#!/usr/bin/env python3
"""Compile and archive the eight comparison firmwares. Never accesses a board."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import shlex
import shutil
import subprocess
import sys
import time

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / 'evidencias/firmwares'
PROJECTS = [f'{board}_{suffix}' for board in ('esp32', 'nrf52840', 'stm32f407', 'stm32f411')
            for suffix in ('framework', 'sem_framework')]
SOURCE_SUFFIXES = {'.c', '.h', '.s', '.S', '.ld', '.cmake', '.conf', '.overlay', '.json', '.py', '.txt', '.defaults'}


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def utc():
    return datetime.now(timezone.utc).isoformat()


def relative(path):
    return str(path.relative_to(ROOT))


def capture(command, env):
    return subprocess.check_output([str(x) for x in command], env=env, text=True, stderr=subprocess.STDOUT).strip()


def source_hashes(project):
    result = {}
    for path in sorted(project.rglob('*')):
        parts = path.relative_to(project).parts
        if not path.is_file() or any(x in parts for x in ('build', '.git', '.agents', '.vscode', '__pycache__', 'managed_components')):
            continue
        if 'Drivers' in parts and path.name != 'CMakeLists.txt':
            continue
        if path.suffix in SOURCE_SUFFIXES or path.name in ('.gitmodules', 'CMakeLists.txt'):
            result[relative(path)] = digest(path)
    return result


def revision(path, env):
    """Do not mislabel a copied project with the parent repository's HEAD."""
    try:
        top = Path(capture(['git', '-C', path, 'rev-parse', '--show-toplevel'], env)).resolve()
        if top != path.resolve():
            return {'kind': 'copied_tree', 'identity': 'source_sha256'}
        return {'kind': 'git', 'head': capture(['git', '-C', path, 'rev-parse', 'HEAD'], env),
                'status': capture(['git', '-C', path, 'status', '--short'], env)}
    except subprocess.CalledProcessError:
        return {'kind': 'copied_tree', 'identity': 'source_sha256'}


def execute(command, env, log, commands, cwd):
    commands.append({'argv': [str(x) for x in command], 'cwd': str(cwd)})
    log.write('\n$ ' + shlex.join(str(x) for x in command) + '\n')
    log.flush()
    result = subprocess.run([str(x) for x in command], cwd=cwd, env=env, stdout=log, stderr=subprocess.STDOUT)
    if result.returncode:
        raise subprocess.CalledProcessError(result.returncode, command)


def archive(source, destination, records):
    destination.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(source, destination)
    records.append({'build_path': relative(source), 'path': relative(destination),
                    'bytes': destination.stat().st_size, 'sha256': digest(destination)})


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--project', action='append', choices=PROJECTS)
    parser.add_argument('--environment', type=Path, default=OUTPUT / 'ambiente.json')
    args = parser.parse_args()
    config = json.loads(args.environment.read_text())
    env = os.environ.copy()
    env.update({key: str(value) for key, value in config['variables'].items()})
    env['PATH'] = os.pathsep.join(config['path_prefix'] + [env.get('PATH', '')])
    env['PYTHONDONTWRITEBYTECODE'] = '1'
    env['IDF_COMPONENT_MANAGER'] = '0'
    env['CCACHE_ENABLE'] = '0'
    env['CMAKE_BUILD_PARALLEL_LEVEL'] = '4'
    cmake = config['cmake']
    idf = [config['esp_python'], str(Path(env['IDF_PATH']) / 'tools/idf.py')]
    arm = str(Path(env['GNUARMEMB_TOOLCHAIN_PATH']) / 'bin/arm-none-eabi-size')
    OUTPUT.mkdir(parents=True, exist_ok=True)
    manifest_path = OUTPUT / 'manifesto.json'
    manifest = json.loads(manifest_path.read_text()) if manifest_path.exists() else {'builds': {}}
    manifest.update({'updated_utc': utc(), 'physical_validation': False, 'flashing_performed': False,
                     'runner_sha256': digest(Path(__file__)), 'environment_file': relative(args.environment.resolve()),
                     'environment': config, 'tool_versions': {
                         'arm_gcc': capture(['arm-none-eabi-gcc', '--version'], env).splitlines()[0],
                         'xtensa_gcc': capture(['xtensa-esp32-elf-gcc', '--version'], env).splitlines()[0],
                         'cmake': capture([cmake, '--version'], env).splitlines()[0],
                         'ninja': capture(['ninja', '--version'], env),
                         'python': capture([config['nrf_python'], '--version'], env),
                         'idf_python': capture([config['esp_python'], '--version'], env)}})
    manifest['sdk_revisions'] = {
        'esp_idf': capture(['git', '-C', env['IDF_PATH'], 'rev-parse', 'HEAD'], env),
        'stm32cube': capture(['git', '-C', env['STM32F4_SDK_DIR'], 'rev-parse', 'HEAD'], env),
        'stm32_hal': capture(['git', '-C', str(Path(env['STM32F4_SDK_DIR']) / 'Drivers/STM32F4xx_HAL_Driver'), 'rev-parse', 'HEAD'], env),
        'stm32_device': capture(['git', '-C', str(Path(env['STM32F4_SDK_DIR']) / 'Drivers/CMSIS/Device/ST/STM32F4xx'), 'rev-parse', 'HEAD'], env),
    }
    nrf_root = Path(env['NRF_CONNECT_SDK_DIR'])
    for entry in json.loads((ROOT / 'nrf52840_framework/tools/sdk-lock.json').read_text())['repositories']:
        path = nrf_root / entry['path']
        if (path / '.git').exists():
            manifest['sdk_revisions']['nrf/' + entry['path']] = capture(['git', '-C', path, 'rev-parse', 'HEAD'], env)
    failed = False
    for name in args.project or PROJECTS:
        project = ROOT / name
        output = OUTPUT / name
        output.mkdir(parents=True, exist_ok=True)
        build = project / 'build' / ('firmware' if name.startswith('esp32') else 'Debug')
        build_env = env.copy()
        build_env['XDG_CACHE_HOME'] = str(project / 'build/cache')
        before = source_hashes(project)
        record = {'project': name, 'started_utc': utc(), 'configuration': 'default (-Og)' if name.startswith('esp32') else 'Debug (-Og)',
                  'project_revision': revision(project, env), 'source_sha256': before,
                  'build_directory': relative(build), 'commands': [], 'artifacts': [],
                  'configuration_files': [], 'log': relative(output / 'compilacao.log')}
        start = time.monotonic()
        print(f'{name}: compilando...', flush=True)
        try:
            with (output / 'compilacao.log').open('w') as log:
                if name.startswith('esp32'):
                    execute(idf + ['-C', project, '-B', build, '-D', 'SDKCONFIG=' + str(build / 'sdkconfig'),
                                   '-D', 'IDF_TARGET=esp32', 'reconfigure'], build_env, log, record['commands'], project)
                else:
                    definitions = ['-DCMAKE_BUILD_TYPE=Debug'] if name.startswith('stm32') else [
                        '-DEXTRA_CONF_FILE=config/debug.conf', '-DPython3_EXECUTABLE=' + config['nrf_python'],
                        '-DUSER_CACHE_DIR=' + str(project / 'build/cache/zephyr')]
                    execute([cmake, '--fresh', '-S', project, '-B', build, '-G', 'Ninja', *definitions],
                            build_env, log, record['commands'], project)
                execute([cmake, '--build', build, '--clean-first', '--parallel', '4'],
                        build_env, log, record['commands'], project)
                memory = output / 'memoria.txt'
                with memory.open('w') as size_log:
                    if name.startswith('esp32'):
                        execute(idf + ['-C', project, '-B', build, 'size'], build_env, size_log, record['commands'], project)
                    else:
                        elf = build / ('zephyr/zephyr.elf' if name.startswith('nrf') else name + '.elf')
                        execute([arm, elf], build_env, size_log, record['commands'], project)
                        execute([arm, '-A', elf], build_env, size_log, record['commands'], project)
                if name.startswith('esp32'):
                    paths = [build / (name + extension) for extension in ('.elf', '.bin', '.map')]
                    paths += [build / 'bootloader/bootloader.bin', build / 'partition_table/partition-table.bin',
                              build / 'flasher_args.json', build / 'flash_args']
                    configs = [build / 'sdkconfig', build / 'config/sdkconfig.json', build / 'compile_commands.json', build / 'CMakeCache.txt']
                elif name.startswith('nrf'):
                    paths = [build / ('zephyr/zephyr' + extension) for extension in ('.elf', '.hex', '.bin', '.uf2', '.map')]
                    configs = [build / 'zephyr/.config', build / 'zephyr/zephyr.dts', build / 'compile_commands.json', build / 'CMakeCache.txt']
                else:
                    paths = [build / (name + extension) for extension in ('.elf', '.hex', '.bin', '.map')]
                    configs = [build / 'compile_commands.json', build / 'CMakeCache.txt']
                for source in paths:
                    if not source.is_file():
                        raise FileNotFoundError(f'Artefato esperado ausente: {source}')
                    archive(source, output / 'artefatos' / source.relative_to(build), record['artifacts'])
                for source in configs:
                    archive(source, output / 'configuracao' / source.relative_to(build), record['configuration_files'])
                if before != source_hashes(project):
                    raise RuntimeError('Fontes alterados durante a compilação; reconstruir antes de aprovar.')
                record.update(status='APROVADO_COMPILACAO', exit_code=0, memory_report=relative(memory))
                print(f'{name}: APROVADO_COMPILACAO', flush=True)
        except (subprocess.CalledProcessError, OSError, RuntimeError) as error:
            failed = True
            record.update(status='FALHOU', exit_code=getattr(error, 'returncode', 1), error=str(error))
            print(f'{name}: FALHOU; consultar {record["log"]}', flush=True)
        record.update(finished_utc=utc(), seconds=round(time.monotonic() - start, 2))
        manifest['builds'][name] = record
        manifest['updated_utc'] = utc()
        manifest_path.write_text(json.dumps(manifest, indent=2, ensure_ascii=False) + '\n')
    return 1 if failed else 0


if __name__ == '__main__':
    sys.exit(main())
