#!/usr/bin/env python3
"""Exercise actual HAL port sources on Linux, with explicit SDK test boundaries."""
import argparse
from collections import Counter
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import shlex
import subprocess
import sys

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent
TARGETS = {
    'esp32': ('esp32_framework', 'esp32-wroom-32', None),
    'nrf52840': ('nrf52840_framework', 'nrf52840', None),
    'stm32f407': ('stm32f407_framework', 'stm32f4', 'STM32F407xx'),
    'stm32f411': ('stm32f411_framework', 'stm32f4', 'STM32F411xE'),
}
CASES = {
    'SYS-01': 'Recursos antes da inicialização',
    'SYS-02': 'Argumentos de sistema, frequência indisponível e nova tentativa válida',
    'SYS-03': 'Inicialização única, selagem idempotente e uso posterior',
    'SYS-04': 'Arena insuficiente sem publicar handles',
    'CFG-01': 'Ponteiros e configurações inválidos sem consumir arena',
    'CFG-02': 'Configurações válidas indisponíveis retornam NOT_SUPPORTED',
    'CFG-03': 'Pino, par de pinos, enumeração e identificador inválidos',
    'RES-01': 'Reserva exclusiva de pinos e controladores',
    'GPIO-01': 'Níveis físicos, direção e argumentos de GPIO',
    'I2C-01': 'Endereço reservado, buffers, tamanho zero e limite do porte',
    'I2C-02': 'Escrita/leitura separadas, endereço e timeout configurado',
    'I2C-03': 'Tradução de erro de comunicação e chamada seguinte',
    'I2C-04': 'Tradução de timeout e chamada seguinte',
    'I2C-05': 'Falha de construção no SDK sem publicação de handle',
    'UART-01': 'Argumentos, contagem inicial e transmissão vazia',
    'UART-02': 'Transmissão completa com contagem de bytes',
    'UART-03': 'Timeout sem progresso',
    'UART-04': 'Timeout com prefixo de dois bytes',
    'UART-05': 'Erro de transmissão sem progresso',
    'UART-06': 'Bloco de 129 bytes, incluindo fracionamento interno no nRF',
    'TIMER-01': 'Handle nulo e callback obrigatório',
    'TIMER-02': 'Sem callback antes do início, início após selagem e início repetido',
    'TIMER-03': 'Período ou recurso válido indisponível',
    'BSP-01': 'Composição real da placa, polaridades, arena selada e temporizador parado',
}
HARDWARE = {
    'HW-01': 'Confirmar níveis elétricos, polaridades e estado inicial do LED',
    'HW-02': 'Capturar I2C: 100 kHz, endereços, STOP e intervalo de conversão',
    'HW-03': 'Provocar NACK e barramento retido após partida; registrar códigos e retomada',
    'HW-04': 'Capturar UART 115200 8N1, conteúdo e fim da transmissão',
    'HW-05': 'Medir temporizador de 1 ms, primeiro evento e perda/aglutinação sob carga',
    'HW-06': 'Exercitar UART parcial/timeout real com instrumentação ou registrar limitação',
}
ESP_HEADERS = ['driver/gpio.h','driver/i2c_master.h','driver/uart.h','driver/gptimer.h',
               'freertos/FreeRTOS.h','freertos/task.h','esp_private/esp_gpio_reserve.h',
               'esp_clk_tree.h','esp_pm.h','sdkconfig.h','esp_attr.h','esp_memory_utils.h']
NRF_HEADERS = ['zephyr/kernel.h','zephyr/drivers/gpio.h','zephyr/drivers/pinctrl.h',
               'zephyr/dt-bindings/pinctrl/nrf-pinctrl.h','nrfx.h','nrfx_twim.h',
               'nrfx_uarte.h','nrfx_timer.h','zephyr/drivers/clock_control/nrf_clock_control.h']


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def git(folder, *args):
    return subprocess.check_output(['git','-C',str(folder),*args], text=True).strip()


def execute(command, log, *, input_text=None):
    try:
        run = subprocess.run([str(x) for x in command], input=input_text,
                             capture_output=True, text=True, timeout=60)
        code, content = run.returncode, run.stdout + run.stderr
    except (OSError, subprocess.TimeoutExpired) as error:
        code, content = 1, str(error)
    log.write_text('$ ' + shlex.join(str(x) for x in command) + '\n' + content)
    return code, content


def build_manifest_location(cli):
    if cli:
        return Path(cli).expanduser().resolve()
    current = ROOT / 'evidencias/firmwares/manifesto.json'
    return current if current.exists() else ROOT / 'build/compilacao/manifesto.json'


def sdk_location(cli, manifest):
    if cli:
        return Path(cli).expanduser().resolve()
    configured = os.environ.get('STM32F4_SDK_DIR')
    if configured:
        return Path(configured).expanduser().resolve()
    if manifest.exists():
        environment = json.loads(manifest.read_text()).get('environment', {})
        directory = environment.get('variables', {}).get('STM32F4_SDK_DIR') or environment.get('stm32_sdk_root')
        if directory:
            return Path(directory)
    return None


def structural_checks(report, logs, manifest):
    groups = ['app','components','hal/interface','hal/memory']
    identities = {}
    for group in groups:
        variants = []
        for project,_,_ in TARGETS.values():
            base = ROOT/project/'src'/group
            variants.append({str(p.relative_to(base)):digest(p) for p in sorted(base.rglob('*')) if p.suffix in ['.c','.h']})
        identities[group] = {'identical':all(x==variants[0] for x in variants),
                             'files_per_target':len(variants[0])}
    ports = [ROOT/TARGETS[name][0]/'src/hal/ports/stm32f4' for name in ['stm32f407','stm32f411']]
    hashes = [{str(p.relative_to(base)):digest(p) for p in base.rglob('*') if p.suffix in ['.c','.h']} for base in ports]
    report['structural'] = {'common_files':identities,'stm32_port_identical':hashes[0]==hashes[1]}
    for name,(project,_,_) in TARGETS.items():
        interface = ROOT/project/'src/hal/interface'
        headers = sorted(interface.rglob('*.h'))
        text = ''.join('#include "'+str(p.relative_to(interface))+'"\n' for p in headers)
        text += 'int main(void) { return 0; }\n'
        results = {}
        for language,compiler,standard in [('c',report['cc'],'c11'),('c++',os.environ.get('CXX','c++'),'c++11')]:
            log = logs/(name+'-headers-'+language.replace('+','p')+'.log')
            code,_ = execute([compiler,'-std='+standard,'-Wall','-Wextra','-Werror',
                              '-fsyntax-only','-I',interface,'-x',language,'-'],log,input_text=text)
            results[language]={'status':'APROVADO_COMPILACAO' if code==0 else 'FALHOU','log':'logs/'+log.name}
        report['targets'][name]['public_headers_without_sdk'] = results
    report['production_builds'] = []
    manifest_name = str(manifest.relative_to(ROOT)) if manifest.is_relative_to(ROOT) else str(manifest)
    report['production_manifest'] = manifest_name if manifest.exists() else None
    if manifest.exists():
        data = json.loads(manifest.read_text())
        for label,build in data['builds'].items():
            matching = [name for name,target in report['targets'].items() if target['project']==build['project']]
            if not matching:
                continue
            name = matching[0]
            current = report['targets'][name]['source_sha256']
            prefix = build['project']+'/'
            if 'source_sha256' in build:
                recorded = build['source_sha256']
                source_match = all(recorded.get(path)==value for path,value in current.items())
                snapshot_match = all((ROOT/path).is_file() and digest(ROOT/path)==value for path,value in recorded.items())
            else:
                recorded = build['source']['tracked_file_sha256']
                source_match = all(recorded.get(path.removeprefix(prefix))==value for path,value in current.items())
                snapshot_match = all((ROOT/build['project']/path).is_file() and digest(ROOT/build['project']/path)==value for path,value in recorded.items())
            artifacts_match = bool(build['artifacts']) and all((ROOT/a['path']).is_file() and digest(ROOT/a['path'])==a['sha256'] for a in build['artifacts'])
            expected_sdk = data.get('sdk_revisions', {}).get('stm32cube')
            expected_sdk = expected_sdk or data.get('environment', {}).get('stm32cube_revisions', {}).get('root')
            actual_sdk = report['targets'][name].get('sdk_revision')
            sdk_match = actual_sdk == expected_sdk if actual_sdk and expected_sdk else None
            approved = build['exit_code']==0 and source_match and snapshot_match and artifacts_match and sdk_match is not False
            report['production_builds'].append({'build':label,'manifest':manifest_name,
                'source_matches_tested_sources':source_match,'production_source_snapshot_current':snapshot_match,
                'artifact_hashes_match':artifacts_match,'stm32_sdk_matches_build':sdk_match,
                'status':'APROVADO_COMPILACAO' if approved else 'DIVERGENTE'})


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--stm32-sdk', help='STM32CubeF4 externo, também aceito por STM32F4_SDK_DIR')
    parser.add_argument('--cc', default=os.environ.get('CC','cc'))
    parser.add_argument('--build-manifest', help='Manifesto dos builds; prefere evidencias/firmwares/manifesto.json quando disponível')
    parser.add_argument('--output', type=Path, default=HERE/'resultados')
    args = parser.parse_args()
    manifest = build_manifest_location(args.build_manifest)
    if args.build_manifest and not manifest.is_file():
        parser.error(f'Manifesto não encontrado: {manifest}')
    output = args.output.resolve(); output.mkdir(parents=True, exist_ok=True)
    build = HERE/'build'; build.mkdir(exist_ok=True)
    logs = output/'logs'; logs.mkdir(exist_ok=True)
    sdk = sdk_location(args.stm32_sdk, manifest)
    report = {'executed_utc':datetime.now(timezone.utc).isoformat(),
              'method':'host simulation of actual port sources; hardware not exercised',
              'cc':args.cc,'compiler':subprocess.check_output([args.cc,'--version'],text=True).splitlines()[0],
              'test_source_sha256':{str(p.relative_to(HERE)):digest(p) for p in sorted((HERE/'host').rglob('*')) if p.is_file()},
              'runner_sha256':digest(Path(__file__)),
              'capabilities_sha256':digest(HERE/'CAPACIDADES.md'), 'targets':{}}
    snapshots = {}
    for name,(project,port_name,device) in TARGETS.items():
        project = ROOT/project; source = project/'src'; port = source/'hal/ports'/port_name
        tracked = [source/'hal/memory/hal_memory.c',source/'hal/memory/hal_memory.h'] + sorted(port.rglob('*.c')) + sorted(port.rglob('*.h')) + sorted((source/'hal/interface').rglob('*.h'))
        tracked += sorted((source/'bsp').rglob('*.c')) + sorted((source/'bsp').rglob('*.h'))
        source_hashes = {str(p.relative_to(ROOT)):digest(p) for p in tracked}
        snapshots[name]=source_hashes
        record = {'project':project.name,'project_revision':git(project,'rev-parse','HEAD'),
                  'contract_revision':git(source/'hal/interface','rev-parse','HEAD'),
                  'port_revision':git(port,'rev-parse','HEAD'),
                  'memory_revision':git(source/'hal/memory','rev-parse','HEAD'),
                  'source_sha256':source_hashes,'cases':{},
                  'hardware':{key:{'status':'NAO_EXECUTADO','reason':'Exige placa e instrumentação; seguir roteiro de bancada.'} for key in HARDWARE}}
        report['targets'][name]=record
        adapter_name = 'stm32f4' if device else name
        adapter = HERE/'host'/adapter_name
        include = build/name/'include'; include.mkdir(parents=True,exist_ok=True)
        for header in (ESP_HEADERS if name=='esp32' else NRF_HEADERS if name=='nrf52840' else []):
            path = include/header;path.parent.mkdir(parents=True,exist_ok=True)
            path.write_text('#include "sdk.h"\n')
        sources = sorted(port.rglob('*.c')) + sorted((source/'bsp').rglob('*.c')) + [source/'hal/memory/hal_memory.c', HERE/'host/common.c', adapter/'adapter.c']
        command = [args.cc,'-std=c11','-O1','-g','-Wall','-Wextra','-Werror','-Wno-unused-parameter',
                   '-ffunction-sections','-fdata-sections','-fsanitize=undefined','-fno-sanitize-recover=all']
        for folder in [include,adapter,HERE/'host',source/'hal/interface',source/'hal/memory',source/'bsp',port]:
            command += ['-I',folder]
        if device:
            if sdk is None or not (sdk/'Drivers/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal.h').is_file():
                record['compile_status']='BLOQUEADO';record['compile_reason']='STM32CubeF4 indisponível; use --stm32-sdk.'
                for key in CASES:record['cases'][key]={'status':'BLOQUEADO','reason':record['compile_reason']}
                continue
            command += ['-D'+device,'-DUSE_HAL_DRIVER','-I',project/'config',
                        '-Wno-pointer-to-int-cast','-Wno-int-to-pointer-cast','-Wno-overflow']
            for folder in ['STM32F4xx_HAL_Driver/Inc','CMSIS/Device/ST/STM32F4xx/Include']:
                command += ['-isystem',sdk/'Drivers'/folder]
            sources += [sdk/'Drivers/STM32F4xx_HAL_Driver/Src'/('stm32f4xx_hal_'+module+'.c') for module in ['gpio','uart','i2c','i2c_ex','tim']]
            for symbol in ['HAL_I2C_Init','HAL_I2C_Master_Transmit','HAL_I2C_Master_Receive','HAL_UART_Transmit']:
                command += ['-Wl,--wrap='+symbol]
            record['sdk_revision']=git(sdk,'rev-parse','HEAD')
        binary = build/name/'contract-tests'
        command += sources + ['-Wl,--gc-sections','-o',binary]
        code,content = execute(command,logs/(name+'-compile.log'))
        record['compile_status']='APROVADO' if code==0 else 'FALHOU'
        record['compile_log']=str((logs/(name+'-compile.log')).relative_to(output))
        for key in CASES:
            if code:
                record['cases'][key]={'status':'BLOQUEADO','reason':'Compilação do ensaio falhou.'}
                continue
            log=logs/(name+'-'+key+'.log')
            status,text=execute([binary,key],log)
            result='APROVADO_SIMULADO' if status==0 else 'NAO_APLICAVEL' if status==77 else 'FALHOU'
            record['cases'][key]={'status':result,'exit_code':status,'log':str(log.relative_to(output))}
        totals=Counter(x['status'] for x in record['cases'].values())
        print(name,dict(totals),flush=True)
        if code:print(content[-3000:],flush=True)
    for name,hashes in snapshots.items():
        report['targets'][name]['sources_unchanged']=all(digest(ROOT/p)==expected for p,expected in hashes.items())
    structural_checks(report, logs, manifest)
    report['full_physical_conformance_demonstrated']=False
    (output/'resultados.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n')
    lines=['# Resultados de conformidade dos portes','',
           'Gerado por `python3 conformidade/run.py`. Execução UTC: '+report['executed_utc']+'.','',
           'Resultados simulados não comprovam operação física ou temporização real.','',
           '| Caso | Objetivo | ESP32 | nRF52840 | STM32F407 | STM32F411 |',
           '| --- | --- | --- | --- | --- | --- |']
    for key,title in CASES.items():
        cells=[report['targets'][name]['cases'][key]['status'] for name in TARGETS]
        lines.append('| '+ ' | '.join([key,title]+cells)+' |')
    lines += ['','## Ensaios físicos','', '| Caso | Objetivo | Estado nas quatro placas |','| --- | --- | --- |']
    for key,title in HARDWARE.items():lines.append(f'| {key} | {title} | NAO_EXECUTADO |')
    lines += ['','Revisões, hashes, códigos de saída e caminhos dos logs: [resultados.json](resultados.json).',
              'Procedimento e limitações: [roteiro]('+os.path.relpath(HERE/'README.md',output)+').','',
              '## Evidência estrutural e de compilação','',
              '- Identidade dos arquivos comuns: '+str(all(x['identical'] for x in report['structural']['common_files'].values()))+'.',
              '- Identidade do porte STM32F4: '+str(report['structural']['stm32_port_identical'])+'.',
              '- Cabeçalhos públicos compilados em C11/C++11 sem diretórios dos SDKs; resultados por alvo no JSON.',
              '- Builds de produção conferidos por hashes e por correspondência com os fontes ensaiados: '+str(len(report['production_builds']))+'.','',
              '| Build de produção | Resultado |','| --- | --- |']
    for production in report['production_builds']:
        lines.append('| '+production['build']+' | '+production['status']+' |')
    lines.append('')
    (output/'MATRIZ.md').write_text('\n'.join(lines))
    failed=any(x['status'] in ['FALHOU','BLOQUEADO'] for r in report['targets'].values() for x in r['cases'].values())
    failed |= any(x['status']=='FALHOU' for r in report['targets'].values() for x in r['public_headers_without_sdk'].values())
    failed |= not all(x['identical'] for x in report['structural']['common_files'].values())
    failed |= not report['structural']['stm32_port_identical']
    failed |= not all(r['sources_unchanged'] for r in report['targets'].values())
    failed |= any(r['status']=='DIVERGENTE' for r in report['production_builds'])
    return 1 if failed else 0


if __name__=='__main__':
    sys.dont_write_bytecode=True
    raise SystemExit(main())
