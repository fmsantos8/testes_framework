#!/usr/bin/env python3
"""Verify archived build identities and paired configurations; no board access."""
import hashlib
import json
from pathlib import Path
import re
import shlex
from datetime import datetime, timezone

from preparar_firmwares import PROJECTS, ROOT, OUTPUT, source_hashes


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def kconfig(path):
    return dict(line.split('=', 1) for line in path.read_text().splitlines() if line.startswith('CONFIG_') and '=' in line)


def main():
    manifest = json.loads((OUTPUT / 'manifesto.json').read_text())
    assert set(manifest['builds']) == set(PROJECTS), 'São necessários os oito builds.'
    report = {'executed_utc': datetime.now(timezone.utc).isoformat(), 'physical_validation': False,
              'flashing_performed': False, 'builds': {}, 'pairs': {}, 'common_sources': {}}
    commands = {}
    for name in PROJECTS:
        record = manifest['builds'][name]
        assert record['status'] == 'APROVADO_COMPILACAO', name
        assert source_hashes(ROOT / name) == record['source_sha256'], f'Fontes alterados: {name}'
        checked = 0
        for item in record['artifacts'] + record['configuration_files']:
            assert sha(ROOT / item['path']) == item['sha256'], f'Cópia alterada: {item["path"]}'
            assert sha(ROOT / item['build_path']) == item['sha256'], f'Build alterado: {item["build_path"]}'
            checked += 1
        build = ROOT / record['build_directory']
        all_commands = json.loads((build / 'compile_commands.json').read_text())
        entry = next(x for x in all_commands if Path(x['file']).name == 'main.c' and Path(x['file']).is_relative_to(ROOT / name))
        tokens = shlex.split(entry['command'])
        assert [x for x in tokens if re.fullmatch(r'-O[0-3gsz]', x)] == ['-Og'], name
        assert not any(x.startswith('-DNDEBUG') for x in tokens), name
        assert '-flto' not in tokens, name
        commands[name] = {'compiler': tokens[0], 'optimization': '-Og', 'ndebug_defined': False,
                          'fpu_options': [x for x in tokens if x.startswith(('-mfpu=', '-mfloat-abi='))]}
        elf = next(x for x in record['artifacts'] if x['path'].endswith('.elf'))
        report['builds'][name] = {'status': 'APROVADO_COMPILACAO', 'source_files': len(record['source_sha256']),
                                 'artifacts_and_configs_checked': checked, 'elf': elf['path'],
                                 'elf_sha256': elf['sha256'], **commands[name]}
    for board in ('esp32', 'nrf52840', 'stm32f407', 'stm32f411'):
        a, b = board + '_framework', board + '_sem_framework'
        assert commands[a]['compiler'] == commands[b]['compiler'], board
        assert commands[a]['fpu_options'] == commands[b]['fpu_options'], board
        if board == 'esp32':
            configs = [json.loads((ROOT / x / 'build/firmware/config/sdkconfig.json').read_text()) for x in (a, b)]
            assert configs[0] == configs[1], 'Configurações efetivas do ESP32 diferentes.'
            report['pairs'][board] = {'effective_sdkconfig_identical': True, 'optimization': '-Og',
                                      'cpu_mhz': configs[0]['ESP_DEFAULT_CPU_FREQ_MHZ'],
                                      'main_stack_bytes': configs[0]['ESP_MAIN_TASK_STACK_SIZE'],
                                      'power_management': configs[0]['PM_ENABLE']}
        elif board == 'nrf52840':
            configs = [kconfig(ROOT / x / 'build/Debug/zephyr/.config') for x in (a, b)]
            diff = {key: [configs[0].get(key), configs[1].get(key)] for key in configs[0].keys() | configs[1].keys()
                    if configs[0].get(key) != configs[1].get(key)}
            assert set(diff) <= {'CONFIG_DT_HAS_GPIO_KEYS_ENABLED'}, diff
            report['pairs'][board] = {'effective_kconfig_differences': diff,
                'difference_explanation': 'Marcador derivado do nó gpio-keys da versão convencional; não habilita um serviço adicional neste build.',
                'same_newlib_fpu_clock_and_services': True, 'optimization': '-Og',
                'main_stack_bytes': int(configs[0]['CONFIG_MAIN_STACK_SIZE']),
                'workqueue_stack_bytes': int(configs[0]['CONFIG_SYSTEM_WORKQUEUE_STACK_SIZE']),
                'heap_bytes': int(configs[0]['CONFIG_HEAP_MEM_POOL_SIZE'])}
        else:
            reserves = []
            for name in (a, b):
                ninja = (ROOT / name / 'build/Debug/build.ninja').read_text()
                link = next(line for line in ninja.splitlines() if line.strip().startswith('LINK_FLAGS ='))
                assert '--specs=nano.specs' in link and '--specs=nosys.specs' in link, name
                assert '-flto' not in link, name
                script = next(x[2:] for x in shlex.split(link.split('=', 1)[1]) if x.startswith('-T'))
                content = Path(script).read_text()
                reserves.append({key: int(re.search(key + r'\s*=\s*(0x[0-9a-fA-F]+|\d+)', content).group(1), 0)
                                 for key in ('_Min_Heap_Size', '_Min_Stack_Size')})
            assert reserves[0] == reserves[1], board
            report['pairs'][board] = {'optimization': '-Og', 'libc': 'newlib-nano',
                                      'assertions_enabled': True, 'lto': False, 'reservations': reserves[0]}
    for group in ('app', 'components', 'hal/interface', 'hal/memory'):
        trees = []
        for board in ('esp32', 'nrf52840', 'stm32f407', 'stm32f411'):
            base = ROOT / (board + '_framework') / 'src' / group
            trees.append({str(p.relative_to(base)): sha(p) for p in base.rglob('*') if p.suffix in ('.c', '.h')})
        assert trees[0] and all(tree == trees[0] for tree in trees), group
        report['common_sources'][group] = {'identical': True, 'files_per_target': len(trees[0])}
    ports = [ROOT / (board + '_framework/src/hal/ports/stm32f4') for board in ('stm32f407', 'stm32f411')]
    trees = [{str(p.relative_to(base)): sha(p) for p in base.rglob('*') if p.suffix in ('.c', '.h')} for base in ports]
    assert trees[0] and trees[0] == trees[1], 'Portes STM32 divergentes.'
    report['stm32_port'] = {'identical': True, 'files_per_target': len(trees[0])}
    report['status'] = 'APROVADO'
    report['verifier_sha256'] = sha(Path(__file__))
    (OUTPUT / 'verificacao.json').write_text(json.dumps(report, indent=2, ensure_ascii=False) + '\n')
    print('Oito builds e seus arquivos arquivados conferidos; quatro pares comparáveis no recorte verificado.')
    print('Fontes comuns e porte STM32 idênticos. Nenhuma gravação ou validação física realizada.')


if __name__ == '__main__':
    main()
