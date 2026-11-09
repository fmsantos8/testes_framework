#!/usr/bin/env python3
"""Collect code, direct SDK dependencies and static memory from frozen builds."""
import csv
from collections import defaultdict
from datetime import datetime, timezone
from functools import lru_cache
import gzip
import hashlib
import json
from pathlib import Path
import re
import shlex
import subprocess
import sys

from elftools.elf.elffile import ELFFile
from preparar_firmwares import PROJECTS, ROOT, source_hashes

OUT = ROOT / 'evidencias/metricas'
MANIFEST = ROOT / 'evidencias/firmwares/manifesto.json'
RULES = OUT / 'regras.json'
BOARDS = ('esp32', 'nrf52840', 'stm32f407', 'stm32f411')
TOKEN = re.compile(r'/\*.*?\*/|//[^\n]*|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'', re.S)
SDK_IDENTIFIERS = re.compile(
    r'\b(?:esp_\w+|ESP_\w+|IRAM_ATTR|DRAM_ATTR|RTC_DATA_ATTR|SOC_\w+|gpio_\w+|GPIO_\w+|'
    r'i2c_master_\w+|I2C_\w+|uart_\w+|UART_\w+|gptimer_\w+|GPTIMER_\w+|'
    r'TickType_t|pdMS_TO_TICKS|portMAX_DELAY|xTaskGetTickCount|vTaskDelay|'
    r'HAL_[A-Za-z0-9_]*[a-z][A-Za-z0-9_]*|HAL_OK|HAL_ERROR|HAL_BUSY|HAL_TIMEOUT|HAL_MAX_DELAY|'
    r'__HAL_\w+|RCC_\w+|FLASH_\w+|PWR_\w+|TIM_\w+|USART_\w+|'
    r'\w+_TypeDef|\w+_IRQn|NVIC_\w+|SysTick|SystemCoreClock|SystemCoreClockUpdate|'
    r'GPIO[A-I]|TIM[2-5]|I2C[1-3]|USART[1-3]|UART[4-8]|STM32F\w+|'
    r'__disable_irq|__enable_irq|__get_PRIMASK|__set_PRIMASK|__DMB|__DSB|__ISB|'
    r'nrfx_\w+|NRFX_\w+|nrf_\w+|NRF_\w+|nrfy_\w+|k_\w+|K_\w+|DT_\w+|'
    r'PINCTRL_\w+|GPIO_DT_\w+|irq_lock|irq_unlock|IRQ_CONNECT|BUILD_ASSERT|'
    r'z_nrf_\w+|onoff_\w+|sys_notify_\w+|CLOCK_CONTROL_\w+|CONFIG_\w+)\b')
STANDARD_HEADERS = set(('assert complex ctype errno fenv float inttypes iso646 limits locale math setjmp signal '
                        'stdalign stdarg stdatomic stdbool stddef stdint stdio stdlib stdnoreturn string '
                        'tgmath threads time uchar wchar wctype unistd').split())


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def rel(path):
    return str(path.relative_to(ROOT)) if path.is_relative_to(ROOT) else str(path)


def run(command):
    return subprocess.check_output([str(x) for x in command], text=True, stderr=subprocess.STDOUT)


def write_csv(name, rows, fields=None):
    fields = fields or list(rows[0])
    with (OUT / name).open('w', newline='', encoding='utf-8') as stream:
        writer = csv.DictWriter(stream, fieldnames=fields, delimiter=';', lineterminator='\n')
        writer.writeheader()
        writer.writerows(rows)


def filtered(text, literals=False):
    def replace(match):
        value = match.group()
        remove = value.startswith(('/*', '//')) or literals
        return ''.join('\n' if x == '\n' else ' ' for x in value) if remove else value
    return TOKEN.sub(replace, text)


@lru_cache(maxsize=None)
def resolved(value):
    return Path(value).resolve()


@lru_cache(maxsize=None)
def git_origin(folder):
    root = Path(run(['git', '-C', folder, 'rev-parse', '--show-toplevel']).strip())
    result = subprocess.run(['git', '-C', root, 'config', '--get', 'remote.origin.url'], capture_output=True, text=True)
    url = result.stdout.strip() or 'workspace:' + root.name
    url = re.sub(r'^git@github.com:', 'https://github.com/', url).removesuffix('.git')
    return root, url


def origin(path):
    root, url = git_origin(path.parent)
    return url + '::' + str(path.relative_to(root))


def layer(local, framework):
    if framework:
        for prefix, label in [('src/app/', 'aplicacao'), ('src/components/', 'componentes'),
                              ('src/hal/interface/', 'contratos'), ('src/hal/memory/', 'arena'),
                              ('src/hal/ports/', 'porte'), ('src/bsp/', 'bsp')]:
            if local.startswith(prefix):
                return label
        return 'integracao'
    name = Path(local).stem
    if name == 'application':
        return 'aplicacao'
    if name in ('sht31d', 'ssd1306'):
        return 'dispositivos'
    if name in ('graphics', 'render', 'fonts', 'bitmaps'):
        return 'apresentacao'
    if name == 'telemetry':
        return 'telemetria'
    return 'integracao'


def collect_files(name, build, ninja):
    directory = ROOT / build['build_directory']
    commands = json.loads((directory / 'compile_commands.json').read_text())
    sources = {resolved(item['file']) for item in commands}
    command = [ninja, '-C', directory, '-t', 'deps']
    text = run(command)
    (OUT / 'dados' / (name + '-dependencias.txt.gz')).write_bytes(gzip.compress(text.encode(), mtime=0))
    raw = {line.strip() for line in text.splitlines() if line.startswith('    ')}
    files = sources | {resolved(str(Path(value) if Path(value).is_absolute() else directory / value)) for value in raw}
    return {p for p in files if p.is_file() and p.suffix in ('.c', '.h')}, sources, command


def classify(path, project, rules, sdk_roots, compiler_roots):
    if path.is_relative_to(project / 'build'):
        return 'gerado', 'Produto de compilação ou configuração gerada pelo SDK.'
    if path.is_relative_to(project):
        local = str(path.relative_to(project))
        if local in rules['exclusoes_loc_por_caminho']:
            if local == 'Core/Src/system_stm32f4xx.c':
                template = sdk_roots[0] / 'Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.c'
                assert sha(path) == sha(template), 'Revisar a proveniência do CMSIS copiado: ' + str(path)
            return rules['exclusoes_loc_por_caminho'][local]
        if 'Drivers' in path.relative_to(project).parts:
            return 'sdk_terceiros', 'Driver de fabricante dentro do projeto.'
        if 'tests' in path.relative_to(project).parts:
            return 'teste', 'Arquivo de teste, excluído da métrica autoral.'
        return 'autoral', 'C/H do projeto, compilado ou incluído, sem identificação de geração/fabricante.'
    if any(path.is_relative_to(root) for root in compiler_roots):
        return 'biblioteca_c_toolchain', 'Header/biblioteca do compilador; não é dependência de fabricante da aplicação.'
    if any(path.is_relative_to(root) for root in sdk_roots):
        return 'sdk_terceiros', 'Arquivo proveniente do SDK fixado.'
    return 'sdk_terceiros', 'Dependência externa ao código do projeto.'


def sdk_references(path, files, categories):
    text = path.read_text(errors='replace')
    comments_removed = filtered(text)
    identifiers = filtered(text, literals=True)
    identifiers = re.sub(r'^[ \t]*#[ \t]*include[^\n]*', lambda m: ' ' * len(m.group()), identifiers, flags=re.M)
    local_macros = set(re.findall(r'^\s*#\s*define\s+(\w+)', comments_removed, re.M))
    evidence = []
    for match in re.finditer(r'^\s*#\s*include\s*[<"]([^>"\n]+)[>"]', comments_removed, re.M):
        header = match.group(1)
        if Path(header).stem in STANDARD_HEADERS or header in ('sys/types.h', 'sys/stat.h', 'sys/time.h', 'sys/times.h'):
            continue
        local = (path.parent / header).resolve()
        candidates = [local] if local in files else [p for p in files if str(p).endswith('/' + header)]
        own = [p for p in candidates if categories[p][0] == 'autoral']
        if not own and any(categories[p][0] == 'sdk_terceiros' for p in candidates):
            evidence.append({'kind': 'include', 'line': comments_removed[:match.start(1)].count('\n') + 1, 'value': header})
    seen = set()
    for match in SDK_IDENTIFIERS.finditer(identifiers):
        if match.group() in seen or match.group() in local_macros:
            continue
        seen.add(match.group())
        evidence.append({'kind': 'identifier', 'line': identifiers[:match.start()].count('\n') + 1, 'value': match.group()})
    conditions = [line.strip() for line in identifiers.splitlines()
                  if re.match(r'\s*#\s*(if|ifdef|ifndef|elif)\b', line)
                  and any(m.group() not in local_macros for m in SDK_IDENTIFIERS.finditer(line))]
    return evidence, conditions


def memory(name, build):
    artifacts = {extension: ROOT / next(a['path'] for a in build['artifacts'] if a['path'].endswith(extension))
                 for extension in ('.elf', '.map')}
    binary_name = 'zephyr.bin' if name.startswith('nrf52840') else name + '.bin'
    artifacts['.bin'] = ROOT / next(a['path'] for a in build['artifacts'] if Path(a['path']).name == binary_name)
    text = artifacts['.map'].read_text()
    part = text.split('Memory Configuration', 1)[1].split('Linker script and memory map', 1)[0]
    regions = {m[0]: {'origin': int(m[1], 16), 'length': int(m[2], 16)}
               for m in re.findall(r'^([\w*]+)\s+(0x[0-9a-f]+)\s+(0x[0-9a-f]+)', part, re.M)}
    if name.startswith('esp32'):
        ram_names = ['iram0_0_seg', 'dram0_0_seg', 'rtc_iram_seg', 'rtc_data_seg', 'rtc_fast_reserved_seg',
                     'rtc_slow_seg', 'rtc_slow_reserved_seg', 'extern_ram_seg']
    else:
        ram_names = ['RAM', 'CCMRAM']
    sections, loads = [], []
    with artifacts['.elf'].open('rb') as stream:
        elf = ELFFile(stream)
        for section in elf.iter_sections():
            if section['sh_flags'] & 2 and section['sh_size']:
                sections.append({'name': section.name, 'type': section['sh_type'],
                                 'address': section['sh_addr'], 'size': section['sh_size']})
        for segment in elf.iter_segments():
            if segment['p_type'] == 'PT_LOAD':
                loads.append({key: segment[key] for key in ('p_vaddr', 'p_paddr', 'p_filesz', 'p_memsz')})
    ram = []
    for region_name in ram_names:
        if region_name not in regions:
            continue
        region = regions[region_name]
        inside = [s for s in sections if region['origin'] <= s['address'] and s['address'] + s['size'] <= region['origin'] + region['length']]
        used = max((s['address'] + s['size'] - region['origin'] for s in inside), default=0)
        ram.append({'region': region_name, **region, 'used_bytes': used, 'sections': inside})
    payload = sum(x['p_filesz'] for x in loads)
    binary_bytes = artifacts['.bin'].stat().st_size
    assert binary_bytes >= payload, 'Imagem menor que o payload inicializado: ' + name
    if 'FLASH' in regions:
        flash = regions['FLASH']
        end = max(x['p_paddr'] + x['p_filesz'] for x in loads if x['p_filesz'] and flash['origin'] <= x['p_paddr'] < flash['origin'] + flash['length'])
        assert end - flash['origin'] == binary_bytes, name
        log = (ROOT / build['log']).read_text()
        reported_ram = re.findall(r'^\s+RAM:\s+(\d+)\s+(B|KB)\s', log, re.M)[-1]
        reported_ram = int(reported_ram[0]) * (1024 if reported_ram[1] == 'KB' else 1)
        assert reported_ram == sum(x['used_bytes'] for x in ram), name
    else:
        raw = (ROOT / build['memory_report']).read_text()
        assert int(re.findall(r'Total image size:\s*(\d+)', raw)[-1]) == payload, name
        reported_ram = sum(int(value) for _, value in re.findall(r'[│┃]\s*(IRAM|DRAM|RTC SLOW)\s*[│┃]\s*(\d+)', raw))
        extra = sum(s['size'] for s in sections if s['name'] in ('.noinit', '.iram0.text_end'))
        assert reported_ram and sum(x['used_bytes'] for x in ram) - reported_ram == extra, name
    result = {'project': name, 'elf': rel(artifacts['.elf']), 'elf_sha256': sha(artifacts['.elf']),
              'application_bin': rel(artifacts['.bin']), 'application_bin_sha256': sha(artifacts['.bin']),
              'flash_bytes': binary_bytes, 'flash_initialized_payload_bytes': payload,
              'image_alignment_and_packaging_bytes': binary_bytes - payload,
              'ram_bytes': sum(x['used_bytes'] for x in ram), 'ram_regions': ram,
              'sdk_or_linker_ram_summary_bytes': reported_ram,
              'extra_ram_noinit_and_alignment_bytes': sum(x['used_bytes'] for x in ram) - reported_ram,
              'load_segments': loads, 'allocated_sections': sections, 'map_sha256': sha(artifacts['.map']),
              'raw_report': build['memory_report'], 'configuration': build['configuration']}
    (OUT / 'dados' / (name + '-memoria.json')).write_text(json.dumps(result, indent=2) + '\n')
    return result


def normalized_counts(rows, formatter, cloc):
    style = {'BasedOnStyle': 'LLVM', 'IndentWidth': 4, 'ColumnLimit': 120,
             'AllowShortFunctionsOnASingleLine': 'None', 'AllowShortIfStatementsOnASingleLine': 'Never',
             'AllowShortLoopsOnASingleLine': False, 'AllowShortBlocksOnASingleLine': 'Never',
             'AllowShortCaseLabelsOnASingleLine': False, 'SortIncludes': 'Never', 'ReflowComments': False}
    (OUT / 'dados/estilo-clang-format.json').write_text(json.dumps(style, indent=2) + '\n')
    paths = []
    for row in rows:
        source = ROOT / row['arquivo']
        destination = OUT / 'dados/normalizado' / row['arquivo']
        destination.parent.mkdir(parents=True, exist_ok=True)
        text = re.sub(r'^[ \t]*//[ \t]*clang-format (off|on)[^\n]*', '', source.read_text(), flags=re.M)
        command = [str(formatter), '--style=' + json.dumps(style), '--assume-filename=' + source.name]
        result = subprocess.run(command, input=text, capture_output=True, text=True, check=True)
        destination.write_text(result.stdout)
        paths.append(destination)
    listing = OUT / 'dados/arquivos-normalizados.txt'
    listing.write_text('\n'.join(str(p) for p in paths) + '\n')
    command = ['perl', cloc, '--by-file', '--json', '--skip-uniqueness', '--force-lang=C,h',
               '--list-file=' + str(listing), '--out=' + str(OUT / 'dados/cloc-normalizado.json')]
    (OUT / 'dados/cloc-normalizado.log').write_text(run(command))
    counted = json.loads((OUT / 'dados/cloc-normalizado.json').read_text())
    for row, path in zip(rows, paths):
        row['loc_normalizadas'] = counted[str(path)]['code']
    return {'version': run([formatter, '--version']).strip(), 'sha256': sha(formatter),
            'style': style, 'cloc_command': [str(x) for x in command]}


def write_summary(report):
    integer = lambda n: f'{n:,}'.replace(',', '.')
    percent = lambda n: f'{n:.2f}'.replace('.', ',') + '%'
    lines = ['# Resultados computacionais do passo 6', '',
        'Valores extraídos dos oito builds identificados no passo 2. Nenhuma placa foi gravada ou executada.', '',
        '## Código autoral original', '',
        '| Projeto | C/H autorais | LOC total | Comum | Específico | Pcomp |',
        '| --- | ---: | ---: | ---: | ---: | ---: |']
    for row in report['code']:
        lines.append(f"| {row['projeto']} | {row['arquivos_autorais']} | {integer(row['loc_total'])} | {integer(row['loc_comum'])} | {integer(row['loc_especifico'])} | {percent(row['pcomp_percent'])} |")
    lines += ['', 'O comum global compreende 44 C/H autorais e 1.815 LOC, usados a partir das mesmas origens nas quatro placas. O arquivo de fontes geradas participa da comparação de identidade, mas é excluído de LOC: os 45 C/H comuns finais são idênticos.', '',
        'Nas versões convencionais, o Pcomp estrito é zero porque os arquivos estão mantidos em caminhos independentes. Há, porém, 11 arquivos autorais idênticos nas quatro versões, somando 486 LOC em cada uma. Isso evidencia conteúdo passível de reúso; zero na métrica de origem não significa ausência de lógica portável ou impossibilidade de compartilhá-la.', '',
        '## Controle de formatação', '',
        'Formatação uniforme foi aplicada somente a cópias, preservando os fontes compilados. Os dois resultados são publicados; LOC não é medida direta de complexidade ou esforço.', '',
        '| Projeto | LOC original | LOC em cópia formatada | Pcomp em cópia formatada |',
        '| --- | ---: | ---: | ---: |']
    for row in report['code']:
        lines.append(f"| {row['projeto']} | {integer(row['loc_total'])} | {integer(row['loc_normalizadas_total'])} | {percent(row['pcomp_normalizado_percent'])} |")
    lines += ['', '## Código mantido no conjunto das quatro placas', '',
        '| Forma | Soma dos checkouts | LOC por origem/conteúdo | LOC formatadas por origem/conteúdo | Arquivos/versões por origem |',
        '| --- | ---: | ---: | ---: | ---: |']
    for form, row in report['maintained_totals'].items():
        lines.append(f"| {form} | {integer(row['loc_somadas_quatro_checkouts'])} | {integer(row['loc_por_origem_e_conteudo'])} | {integer(row['loc_normalizadas_por_origem_e_conteudo'])} | {row['origens_versoes_distintas']} |")
    lines += ['', f"O porte STM32 representa {report['stm32_port']['files']} arquivos e {integer(report['stm32_port']['loc_unique'])} LOC mantidos em uma origem e usados pelas duas placas. Ele é específico do ambiente STM32 na métrica global.", '',
        'O conjunto com framework tem mais LOC autorais, mesmo após descontar as origens compartilhadas. A contribuição observada está no reúso e nas fronteiras de dependência; os resultados não demonstram redução do volume total de código, tempo de desenvolvimento ou esforço de manutenção.', '',
        '## Dependências diretas dos SDKs', '',
        '| Projeto | Arquivos com SDK / autorais | Percentual |',
        '| --- | ---: | ---: |']
    for row in report['code']:
        lines.append(f"| {row['projeto']} | {row['arquivos_sdk_direto']} / {row['arquivos_autorais']} | {percent(row['percent_arquivos_sdk_direto'])} |")
    lines += ['', 'Nos quatro projetos com framework não foram encontradas dependências diretas de SDK em aplicação, componentes, contratos ou arena. As referências estão nos portes, BSP e integração. Nas versões convencionais também aparecem em aplicação, dispositivos e telemetria. A contagem absoluta de arquivos com SDK é maior no framework em ESP32/nRF; percentuais e número de diretórios dependem da decomposição e não devem ser interpretados isoladamente como qualidade.', '',
        '## Memória — bytes da aplicação', '',
        '| Placa | Flash sem | Flash com | Acréscimo flash | RAM sem | RAM com | Acréscimo RAM |',
        '| --- | ---: | ---: | ---: | ---: | ---: | ---: |']
    for row in report['memory_comparisons']:
        lines.append(f"| {row['placa']} | {integer(row['flash_sem'])} | {integer(row['flash_com'])} | {integer(row['flash_delta_bytes'])} ({percent(row['flash_delta_percent'])}) | {integer(row['ram_sem'])} | {integer(row['ram_com'])} | {integer(row['ram_delta_bytes'])} ({percent(row['ram_delta_percent'])}) |")
    lines += ['', 'Flash corresponde ao BIN da aplicação, com inicialização de RAM e empacotamento/alinhamento, excluindo bootloader separado e áreas externas à imagem. RAM soma as regiões estáticas/reservadas indicadas por ELF/MAP, incluindo noinit e reservas existentes, sem somar stacks/heaps duas vezes. Não representa pico em execução.', '',
        'O framework aumentou flash e RAM nos quatro pares. Os maiores acréscimos relativos de flash ocorreram nos STM32. As diferenças abrangem a implementação completa, incluindo validações, contextos e capacidades dos portes; não isolam o custo de uma chamada abstrata.', '',
        '## Dados e reprodução', '',
        'Consulte [README.md](README.md) para regras, exclusões, arquivos e comandos. Os CSVs preservam números sem separador de milhar e com ponto decimal; esta síntese usa a apresentação em português.', '']
    (OUT / 'RESUMO.md').write_text('\n'.join(lines), encoding='utf-8')


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / 'dados').mkdir(exist_ok=True)
    rules = json.loads(RULES.read_text())
    manifest = json.loads(MANIFEST.read_text())
    env = manifest['environment']
    sdk_roots = [resolved(env['variables'][x]) for x in ('STM32F4_SDK_DIR', 'NRF_CONNECT_SDK_DIR', 'IDF_PATH')]
    compiler_roots = [resolved(env['variables']['GNUARMEMB_TOOLCHAIN_PATH']),
                      resolved(Path(env['path_prefix'][2]).parent)]
    ninja = next(str(Path(p) / 'ninja') for p in env['path_prefix'] if (Path(p) / 'ninja').exists())
    cloc = ROOT / 'build/deps/cloc-2.06.pl'
    assert run(['perl', cloc, '--version']).strip() == '2.06'
    files_per_project, sources, dependency_commands = {}, {}, {}
    for name in PROJECTS:
        build = manifest['builds'][name]
        assert build['status'] == 'APROVADO_COMPILACAO', name
        assert source_hashes(ROOT / name) == build['source_sha256'], 'Fontes alterados: ' + name
        for item in build['artifacts'] + build['configuration_files']:
            assert sha(ROOT / item['path']) == item['sha256'], item['path']
            assert sha(ROOT / item['build_path']) == item['sha256'], item['build_path']
        files_per_project[name], sources[name], dependency_commands[name] = collect_files(name, build, ninja)
        print(name, 'inventário:', len(files_per_project[name]), 'C/H', flush=True)
    all_files = sorted(set.union(*files_per_project.values()))
    listing = OUT / 'dados/todos-c-h.txt'
    listing.write_text('\n'.join(str(p) for p in all_files) + '\n')
    cloc_command = ['perl', cloc, '--by-file', '--json', '--skip-uniqueness', '--force-lang=C,h',
                    '--list-file=' + str(listing), '--out=' + str(OUT / 'dados/cloc.json')]
    print('Contando linhas com cloc 2.06...', flush=True)
    (OUT / 'dados/cloc.log').write_text(run(cloc_command))
    counts = json.loads((OUT / 'dados/cloc.json').read_text())
    inventories, details = [], []
    for name in PROJECTS:
        project = ROOT / name
        categories = {p: classify(p, project, rules, sdk_roots, compiler_roots) for p in files_per_project[name]}
        for path in sorted(files_per_project[name]):
            category, why = categories[path]
            if str(path) not in counts:
                assert path.stat().st_size == 0, 'cloc não contabilizou arquivo não vazio: ' + str(path)
                code = {'code': 0, 'comment': 0, 'blank': 0}
            else:
                code = counts[str(path)]
            local = str(path.relative_to(project)) if path.is_relative_to(project) else rel(path)
            evidence, conditions = sdk_references(path, files_per_project[name], categories) if category == 'autoral' else ([], [])
            row = {'projeto': name, 'arquivo': rel(path), 'origem_mantida': origin(path) if category == 'autoral' else '',
                   'categoria': category, 'loc': code['code'], 'depende_sdk': int(bool(evidence)),
                   'comentarios': code['comment'], 'vazias': code['blank'], 'sha256': sha(path),
                   'papel': 'fonte_compilada' if path in sources[name] else 'cabecalho_utilizado',
                   'camada': layer(local, not name.endswith('_sem_framework')) if category == 'autoral' else '',
                   'diretorio': str(Path(local).parent), 'motivo': why,
                   'condicional_sdk': int(bool(conditions)), 'comum_global': 0}
            inventories.append(row)
            if category == 'autoral':
                details.append({'projeto': name, 'arquivo': rel(path), 'depende_sdk': bool(evidence),
                                'evidencias': evidence, 'condicionais_sdk': conditions})
    author = [x for x in inventories if x['categoria'] == 'autoral']
    print('Controle de formatação em cópias dos arquivos autorais...', flush=True)
    formatter = Path(env['variables']['IDF_TOOLS_PATH']) / 'esp-clang/esp-21.1.3_20260408/esp-clang/bin/clang-format'
    normalization = normalized_counts(author, formatter, cloc)
    for row in inventories:
        row.setdefault('loc_normalizadas', '')
    common_sets = {}
    maintained_rows, content_rows = [], []
    for suffix in ('framework', 'sem_framework'):
        selected = [x for x in author if x['projeto'] in {b + '_' + suffix for b in BOARDS}]
        by_origin, by_content = defaultdict(list), defaultdict(list)
        for row in selected:
            by_origin[(row['origem_mantida'], row['sha256'])].append(row)
            by_content[row['sha256']].append(row)
        common = {key for key, rows in by_origin.items() if len({r['projeto'] for r in rows}) == 4 and not any(r['condicional_sdk'] for r in rows)}
        common_sets[suffix] = common
        for row in selected:
            row['comum_global'] = int((row['origem_mantida'], row['sha256']) in common)
        for (source, fingerprint), rows in sorted(by_origin.items()):
            assert len({r['loc'] for r in rows}) == 1
            maintained_rows.append({'forma': suffix, 'origem_mantida': source, 'sha256': fingerprint,
                'loc_unicas': rows[0]['loc'], 'loc_normalizadas_unicas': rows[0]['loc_normalizadas'],
                'ocorrencias': len(rows), 'projetos': ','.join(sorted({r['projeto'] for r in rows})),
                'comum_global': int((source, fingerprint) in common)})
        for fingerprint, rows in sorted(by_content.items()):
            if len({r['projeto'] for r in rows}) < 2:
                continue
            content_rows.append({'forma': suffix, 'sha256': fingerprint, 'loc_por_arquivo': rows[0]['loc'],
                'projetos': ','.join(sorted({r['projeto'] for r in rows})),
                'origens_mantidas_distintas': len({r['origem_mantida'] for r in rows}),
                'arquivos': '|'.join(r['arquivo'] for r in rows),
                'comum_quatro_por_conteudo': int(len({r['projeto'] for r in rows}) == 4 and not any(r['condicional_sdk'] for r in rows))})
    code_rows, dispersion_rows = [], []
    for name in PROJECTS:
        selected = [r for r in author if r['projeto'] == name]
        total = sum(r['loc'] for r in selected)
        common = sum(r['loc'] for r in selected if r['comum_global'])
        same_content = {r['sha256'] for r in content_rows if r['forma'] == ('sem_framework' if name.endswith('_sem_framework') else 'framework') and r['comum_quatro_por_conteudo']}
        code_rows.append({'projeto': name, 'arquivos_autorais': len(selected), 'loc_total': total,
            'loc_comum': common, 'loc_especifico': total - common, 'pcomp_percent': round(100 * common / total, 4),
            'loc_normalizadas_total': sum(r['loc_normalizadas'] for r in selected),
            'loc_normalizadas_comum': sum(r['loc_normalizadas'] for r in selected if r['comum_global']),
            'pcomp_normalizado_percent': round(100 * sum(r['loc_normalizadas'] for r in selected if r['comum_global']) / sum(r['loc_normalizadas'] for r in selected), 4),
            'loc_identico_nas_quatro_por_conteudo': sum(r['loc'] for r in selected if r['sha256'] in same_content),
            'percent_identico_nas_quatro_por_conteudo': round(100 * sum(r['loc'] for r in selected if r['sha256'] in same_content) / total, 4),
            'arquivos_sdk_direto': sum(r['depende_sdk'] for r in selected),
            'percent_arquivos_sdk_direto': round(100 * sum(r['depende_sdk'] for r in selected) / len(selected), 4)})
        for directory in sorted({r['diretorio'] for r in selected}):
            subset = [r for r in selected if r['diretorio'] == directory]
            dispersion_rows.append({'projeto': name, 'diretorio': directory, 'arquivos_autorais': len(subset),
                                     'arquivos_sdk_direto': sum(r['depende_sdk'] for r in subset)})
        (OUT / ('arquivos-' + name + '.txt')).write_text('\n'.join(r['arquivo'] for r in selected) + '\n')
    write_csv('inventario.csv', inventories)
    write_csv('codigo.csv', code_rows)
    write_csv('codigo-mantido.csv', maintained_rows)
    write_csv('identidade-conteudo.csv', content_rows)
    write_csv('dispersao.csv', dispersion_rows)
    layers, exclusions = [], []
    for name in PROJECTS:
        for label in sorted({r['camada'] for r in author if r['projeto'] == name}):
            subset = [r for r in author if r['projeto'] == name and r['camada'] == label]
            layers.append({'projeto': name, 'camada': label, 'arquivos': len(subset),
                           'loc': sum(r['loc'] for r in subset), 'arquivos_sdk_direto': sum(r['depende_sdk'] for r in subset)})
        for category in sorted({r['categoria'] for r in inventories if r['projeto'] == name}):
            subset = [r for r in inventories if r['projeto'] == name and r['categoria'] == category]
            exclusions.append({'projeto': name, 'categoria': category, 'arquivos': len(subset),
                               'loc': sum(r['loc'] for r in subset), 'incluido_metrica_autoral': category == 'autoral'})
    write_csv('dispersao-camadas.csv', layers)
    write_csv('categorias.csv', exclusions)
    (OUT / 'dependencias-sdk.json').write_text(json.dumps(details, indent=2, ensure_ascii=False) + '\n')
    memories = [memory(name, manifest['builds'][name]) for name in PROJECTS]
    write_csv('memoria.csv', [{'placa': r['project'].split('_')[0], 'versao': 'sem_framework' if r['project'].endswith('_sem_framework') else 'com_framework',
        'configuracao': r['configuration'], 'flash_bytes': r['flash_bytes'], 'ram_bytes': r['ram_bytes'],
        'fonte_do_numero': 'dados/' + r['project'] + '-memoria.json',
        'observacoes': 'Imagem .bin da aplicação; RAM estática/reservada por região. Bootloader separado excluído; não mede pico.'} for r in memories])
    comparisons = []
    for board in BOARDS:
        a, b = [next(r for r in memories if r['project'] == board + '_' + suffix) for suffix in ('framework', 'sem_framework')]
        comparisons.append({'placa': board, 'flash_com': a['flash_bytes'], 'flash_sem': b['flash_bytes'],
            'flash_delta_bytes': a['flash_bytes'] - b['flash_bytes'],
            'flash_delta_percent': round(100 * (a['flash_bytes'] - b['flash_bytes']) / b['flash_bytes'], 4),
            'ram_com': a['ram_bytes'], 'ram_sem': b['ram_bytes'], 'ram_delta_bytes': a['ram_bytes'] - b['ram_bytes'],
            'ram_delta_percent': round(100 * (a['ram_bytes'] - b['ram_bytes']) / b['ram_bytes'], 4)})
    write_csv('comparacao-memoria.csv', comparisons)
    port = [r for r in author if '/src/hal/ports/stm32f4/' in r['arquivo']]
    grouped = defaultdict(list)
    for row in port:
        grouped[row['origem_mantida']].append(row)
    assert len(grouped) == 7 and all(len(rows) == 2 and len({r['sha256'] for r in rows}) == 1 for rows in grouped.values())
    write_csv('reuso-stm32.csv', [{'origem_mantida': key, 'sha256': rows[0]['sha256'], 'loc': rows[0]['loc'],
        'mesma_origem_e_conteudo': True, 'f407': rows[0]['arquivo'], 'f411': rows[1]['arquivo']} for key, rows in sorted(grouped.items())])
    identities = {}
    for group in ('app', 'components', 'hal/interface', 'hal/memory'):
        trees = []
        for board in BOARDS:
            base = ROOT / (board + '_framework') / 'src' / group
            trees.append({str(p.relative_to(base)): sha(p) for p in base.rglob('*') if p.suffix in ('.c', '.h')})
        assert trees[0] and all(tree == trees[0] for tree in trees)
        identities[group] = {'files_per_project': len(trees[0]), 'divergent_files': 0, 'identical': True}
    report = {'executed_utc': datetime.now(timezone.utc).isoformat(), 'physical_validation': False,
        'runner_sha256': sha(Path(__file__)), 'rules_sha256': sha(RULES), 'build_manifest_sha256': sha(MANIFEST),
        'cloc': {'version': '2.06', 'sha256': sha(cloc), 'command': [str(x) for x in cloc_command]},
        'normalization': normalization,
        'dependency_commands': {n: [str(x) for x in c] for n, c in dependency_commands.items()},
        'code': code_rows, 'memory_comparisons': comparisons,
        'common_source_identity': identities,
        'maintained_totals': {form: {'loc_somadas_quatro_checkouts': sum(r['loc_total'] for r in code_rows if r['projeto'] in {b + '_' + form for b in BOARDS}),
            'loc_por_origem_e_conteudo': sum(r['loc_unicas'] for r in maintained_rows if r['forma'] == form),
            'loc_normalizadas_por_origem_e_conteudo': sum(r['loc_normalizadas_unicas'] for r in maintained_rows if r['forma'] == form),
            'origens_versoes_distintas': sum(r['forma'] == form for r in maintained_rows)} for form in ('framework', 'sem_framework')},
        'stm32_port': {'files': len(grouped), 'loc_unique': sum(rows[0]['loc'] for rows in grouped.values()), 'shared_both_boards': True},
        'input_files': {str(p): sha(p) for p in all_files},
        'outputs_sha256': {str(p.relative_to(OUT)): sha(p) for p in sorted(OUT.rglob('*')) if p.is_file() and p.suffix in ('.csv', '.json', '.txt', '.gz') and p.name not in ('resultados.json', 'verificacao.json')}}
    (OUT / 'resultados.json').write_text(json.dumps(report, indent=2, ensure_ascii=False) + '\n')
    write_summary(report)
    print(json.dumps({'codigo': code_rows, 'memoria': comparisons, 'mantido': report['maintained_totals'], 'porte_stm32': report['stm32_port']}, indent=2), flush=True)


if __name__ == '__main__':
    main()
