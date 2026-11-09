#!/usr/bin/env python3
"""Compare native implementations with the real framework core at the SDK boundary."""
import argparse
import difflib
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
HOST = Path(__file__).with_name('host')
PROJECTS = ['esp32_sem_framework', 'nrf52840_sem_framework', 'stm32f407_sem_framework', 'stm32f411_sem_framework']

def run(command):
    return subprocess.run([str(x) for x in command], check=True, capture_output=True, text=True).stdout

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=ROOT / 'build/validacao_sem_framework')
    args = parser.parse_args()
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    report = {'kind':'host_simulation', 'physical_validation':False, 'comparisons':[]}
    baseline = ROOT / 'esp32_framework/src'
    for part in ['app','components']:
        for source in (baseline/part).rglob('*'):
            if source.suffix in ['.c','.h']:
                for family in ['nrf52840','stm32f407','stm32f411']:
                    candidate = ROOT/(family+'_framework')/'src'/source.relative_to(baseline)
                    if candidate.read_bytes() != source.read_bytes():
                        raise SystemExit(f'Reference cores differ: {candidate}')
    reference_sources = [*sorted((baseline/'app').rglob('*.c')), *sorted((baseline/'components').glob('*/*.c'))]
    common_flags = ['cc','-std=c11','-g','-O1','-Wall','-Wextra','-Werror','-fsanitize=undefined','-fno-sanitize-recover=all',f'-I{HOST}']
    try:
        with tempfile.TemporaryDirectory(prefix='tcc-equivalence-') as temporary:
            temporary = Path(temporary)
            exe = temporary/'reference'
            includes = [baseline/'app', baseline/'hal/interface', *sorted((baseline/'components').iterdir())]
            command = common_flags + [f'-I{x}' for x in includes if x.is_dir()] + [HOST/'scenario.c',HOST/'reference.c',*reference_sources,'-o',exe]
            run(command)
            reference = {}
            for scenario in [0,1,2]:
                reference[scenario] = run([exe,str(scenario)])
                (output/f'reference-{scenario}.trace').write_text(reference[scenario])
            for project in PROJECTS:
                source = ROOT/project/('Core/Src' if project.startswith('stm32') else 'main' if project.startswith('esp32') else 'src')
                include = source.parent/'Inc' if project.startswith('stm32') else source
                sources = [source/f'{name}.c' for name in ['application','sht31d','ssd1306','telemetry','graphics','fonts','render','bitmaps']]
                exe = temporary/project
                command = common_flags + [f'-I{include}','-DMAIN_H','-DI2C_H','-DUSART_H','-include',HOST/'native_sdk.h',HOST/'scenario.c',HOST/'native.c',*sources,'-o',exe]
                run(command)
                for scenario in [0,1,2]:
                    actual = run([exe,str(scenario)])
                    expected = reference[scenario]
                    (output/f'{project}-{scenario}.trace').write_text(actual)
                    equal = actual == expected
                    result = {'project':project,'scenario':['normal_counter','counter_wrap','queue_saturation'][scenario],'passed':equal,'events':len(actual.splitlines()),'trace_sha256':hashlib.sha256(actual.encode()).hexdigest()}
                    report['comparisons'].append(result)
                    print(f'{project} / {result["scenario"]}: {"PASS" if equal else "FAIL"} ({result["events"]} events)')
                    if equal:
                        (output/f'{project}-{scenario}.diff').unlink(missing_ok=True)
                    if not equal:
                        difference = '\n'.join(difflib.unified_diff(expected.splitlines(),actual.splitlines(),fromfile='framework',tofile=project))
                        (output/f'{project}-{scenario}.diff').write_text(difference+'\n')
                        print(difference[:4000])
            # Record exact authored source inputs, excluding generated artifacts and SDK drivers.
            report['source_sha256'] = {str(p.relative_to(ROOT)):hashlib.sha256(p.read_bytes()).hexdigest()
                for name in PROJECTS for p in sorted((ROOT/name).rglob('*'))
                if p.is_file() and 'build' not in p.relative_to(ROOT/name).parts and 'Drivers' not in p.relative_to(ROOT/name).parts and '__pycache__' not in p.relative_to(ROOT/name).parts}
            report['reference_sha256'] = {str(p.relative_to(ROOT)):hashlib.sha256(p.read_bytes()).hexdigest()
                for p in sorted(set(reference_sources + list((baseline/'app').rglob('*.h')) + list((baseline/'components').rglob('*.h')) + list((baseline/'hal/interface').rglob('*.h'))))}
    except subprocess.CalledProcessError as error:
        print(error.stdout, error.stderr)
        raise SystemExit(error.returncode) from None
    report['test_sha256'] = {str(p.relative_to(ROOT)):hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(Path(__file__).parent.rglob('*')) if p.is_file() and '__pycache__' not in p.parts}
    report['compiler'] = run(['cc','--version']).splitlines()[0]
    (output/'results.json').write_text(json.dumps(report,indent=2)+'\n')
    if not all(x['passed'] for x in report['comparisons']): raise SystemExit(1)

if __name__ == '__main__':
    main()
