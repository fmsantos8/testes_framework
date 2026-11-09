#!/usr/bin/env python3
"""Prepare step 7 from preserved evidence; never changes firmware or raw inputs.

Use --check to verify the current package without writing files.
PDF authoring requires reportlab and Pillow; checking uses only the stdlib.
"""
from __future__ import annotations

import argparse
import csv
import hashlib
import io
import json
import subprocess
from collections import Counter
from datetime import datetime, timezone
from decimal import Decimal, ROUND_HALF_UP
from pathlib import Path
from statistics import mean

ROOT = Path(__file__).resolve().parents[1]
BASE = ROOT / 'evidencias/artigo'
FIG = ROOT / 'tcc_2/figuras'
TAB = ROOT / 'tcc_2/tabelas'
BOARDS = ('esp32', 'nrf52840', 'stm32f407', 'stm32f411')
NAMES = dict(zip(BOARDS, ('ESP32', 'nRF52840', 'STM32F407', 'STM32F411')))
FIELDS = ('inicializacao', 'leituras', 'botao_telas', 'uart', 'falha_sensor',
          'recuperacao_sensor', 'falha_display', 'recuperacao_display')


def sha(path):
    h = hashlib.sha256()
    with Path(path).open('rb') as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b''):
            h.update(block)
    return h.hexdigest()


def read_json(path):
    return json.loads(Path(path).read_text())


def rows(path):
    with Path(path).open(newline='') as stream:
        return list(csv.DictReader(stream, delimiter=';'))


def relative(path):
    return str(Path(path).relative_to(ROOT))


def session_path(row):
    return (ROOT / 'evidencias' / row['evidencias']).parent


def load_and_validate():
    functional = rows(ROOT / 'evidencias/resultados-funcionais.csv')
    control = rows(ROOT / 'evidencias/controle.csv')
    conformance = rows(ROOT / 'evidencias/resultados-conformidade.csv')
    code = rows(ROOT / 'evidencias/metricas/codigo.csv')
    memory = rows(ROOT / 'evidencias/metricas/memoria.csv')
    comparison = rows(ROOT / 'evidencias/metricas/comparacao-memoria.csv')
    metrics = read_json(ROOT / 'evidencias/metricas/resultados.json')
    simulation = read_json(ROOT / 'evidencias/simulacao/conformidade/resultados.json')
    simulation_summary = read_json(ROOT / 'evidencias/simulacao/resumo.json')
    counts = Counter(c['status'] for target in simulation['targets'].values() for c in target['cases'].values())
    assert dict(counts) == simulation_summary['conformance']['results'] == {'APROVADO_SIMULADO': 95, 'NAO_APLICAVEL': 1}
    for path, expected in simulation_summary['report_sha256'].items():
        assert sha(ROOT / 'evidencias/simulacao' / path) == expected
    builds = read_json(ROOT / 'evidencias/firmwares/manifesto.json')['builds']
    inputs = set()
    for name in ('controle.csv', 'resultados-funcionais.csv', 'resultados-conformidade.csv',
                 'firmwares/manifesto.json', 'simulacao/resumo.json',
                 'simulacao/conformidade/resultados.json', 'simulacao/equivalencia/results.json',
                 'metricas/resultados.json', 'metricas/codigo.csv', 'metricas/memoria.csv',
                 'metricas/comparacao-memoria.csv', 'metricas/dispersao-camadas.csv',
                 'metricas/dispersao.csv', 'metricas/inventario.csv', 'metricas/reuso-stm32.csv',
                 'metricas/codigo-mantido.csv', 'metricas/identidade-conteudo.csv',
                 'metricas/regras.json'):
        inputs.add(ROOT / 'evidencias' / name)
    inventory = rows(ROOT / 'evidencias/metricas/inventario.csv')
    categories = {str((ROOT / r['arquivo']).resolve()): r['categoria'] for r in inventory}
    missing, different = {}, {}
    # External SDK/toolchain installations may have been removed since step 6.
    # Their preserved counting records remain valid inputs; do not claim a new hash check.
    for path, expected in metrics['input_files'].items():
        if not (ROOT / path).is_file():
            category = categories.get(str((ROOT / path).resolve()))
            assert category in ('sdk_terceiros', 'biblioteca_c_toolchain', 'gerado'), path
            missing[path] = category
            continue
        if sha(ROOT / path) != expected:
            category = categories.get(str((ROOT / path).resolve()))
            assert category in ('sdk_terceiros', 'biblioteca_c_toolchain', 'gerado'), path
            different[path] = category
    for path, expected in metrics['outputs_sha256'].items():
        assert sha(ROOT / 'evidencias/metricas' / path) == expected, path
    metrics['_step7_input_check'] = dict(matched=len(metrics['input_files'])-len(missing)-len(different),
                                       missing_excluded_non_authoral=len(missing),
                                       missing_by_category=dict(Counter(missing.values())),
                                       different_excluded_non_authoral=different)
    author = [r for r in inventory if r['categoria'] == 'autoral']
    for row in code:
        selected = [r for r in author if r['projeto'] == row['projeto']]
        assert len(selected) == int(row['arquivos_autorais'])
        assert sum(int(r['loc']) for r in selected) == int(row['loc_total'])
        assert sum(int(r['loc']) for r in selected if r['comum_global'] == '1') == int(row['loc_comum'])
        assert int(row['loc_total']) == int(row['loc_comum']) + int(row['loc_especifico'])
        assert sum(int(r['depende_sdk']) for r in selected) == int(row['arquivos_sdk_direto'])
        assert abs(100 * int(row['loc_comum']) / int(row['loc_total']) - float(row['pcomp_percent'])) < .0001
        for source in selected:
            assert sha(ROOT / source['arquivo']) == source['sha256'], source['arquivo']
    assert len(functional) == len(control) == len(code) == len(memory) == 8
    assert {(r['placa'], r['versao']) for r in functional} == {(b, v) for b in BOARDS for v in ('com_framework', 'sem_framework')}
    led_rows, signal_rows = [], []
    for row in functional:
        folder = session_path(row)
        session = read_json(folder / 'sessao.json')
        inputs.update((folder / 'sessao.json', folder / 'observacoes.md'))
        assert row['placa'] == session['board'] and row['versao'] == session['variant']
        assert all(row[f] == session['outcomes'][f] for f in FIELDS)
        identity = next(c for c in control if c['placa'] == row['placa'] and c['versao'] == row['versao'])
        assert identity['sha256'] == session['identity']['elf_sha256'] == sha(ROOT / identity['binario'])
        assert identity['resultado'] == 'APROVADO' and all(row[f] == 'APROVADO' for f in FIELDS)
        build = builds[identity['projeto']]
        assert any(a['path'] == identity['binario'] and a['sha256'] == identity['sha256'] for a in build['artifacts'])
        for log in session['uart_logs']:
            inputs.update((folder / log['text'], folder / log['raw']))
        if row['versao'] != 'com_framework':
            continue
        for group in ('GPIO', 'I2C', 'UART', 'TIMER'):
            c = next(c for c in conformance if c['placa'] == row['placa'] and c['grupo'] == group)
            assert c['resultado_fisico'] == session['groups'][group]['result']
        led = session['led']
        intervals = rows(folder / led['table'])
        assert len(intervals) == led['cycles'] >= 10
        periods = [float(r['periodo_ms']) for r in intervals]
        for metric, value in [('media', mean(periods)), ('minimo', min(periods)), ('maximo', max(periods))]:
            assert abs(led['statistics']['periodo_ms'][metric] - value) < 1e-6
        inputs.update((folder / led['table'], folder / led['source'], folder / 'conferencia-sal.json', folder / 'intervalos-sal.csv'))
        led_rows.append(dict(placa=row['placa'], ciclos=led['cycles'],
                             inicio_s=led['window_start_s'], fim_s=led['window_end_s'],
                             periodo_medio_ms=mean(periods), minimo_ms=min(periods), maximo_ms=max(periods),
                             desvio_nominal_percent=100 * (mean(periods) / 1000 - 1),
                             resultado='LIMITADO', fonte=relative(folder / led['table'])))
        signal = read_json(folder / 'conferencia-sal.json')['summary']
        assert signal['sensor_validation']['all_checks_pass'] and signal['led_csv_exact']
        for path, expected in read_json(folder / 'conferencia-sal.json')['source_files_sha256'].items():
            assert sha(folder / path) == expected
            inputs.add(folder / path)
        signal_rows.append(dict(placa=row['placa'], leituras_completas=signal['sensor_validation']['complete_reads'],
                                verificacoes_sensor='APROVADO', registros_uart=signal['uart_lines'],
                                erros_stop_bit=signal['uart_frame_errors'], taxa_amostragem_hz=int(signal['rate_hz']),
                                fonte=relative(folder / 'conferencia-sal.json')))
    assert sum(r['leituras_completas'] for r in signal_rows) == 290
    for row in memory:
        project = row['placa'] + ('_framework' if row['versao'] == 'com_framework' else '_sem_framework')
        path = ROOT / 'evidencias/metricas' / row['fonte_do_numero']
        inputs.add(path)
        data = read_json(path)
        assert int(row['flash_bytes']) == data['flash_bytes'] == (ROOT / data['application_bin']).stat().st_size
        assert sha(ROOT / data['application_bin']) == data['application_bin_sha256']
        assert int(row['ram_bytes']) == data['ram_bytes'] == sum(r['used_bytes'] for r in data['ram_regions'])
        assert data['elf_sha256'] == sha(ROOT / data['elf'])
        assert data['project'] == project
    for row in comparison:
        for kind in ('flash', 'ram'):
            values = {m['versao']: int(m[kind + '_bytes']) for m in memory if m['placa'] == row['placa']}
            assert values['com_framework'] == int(row[kind + '_com'])
            assert values['sem_framework'] == int(row[kind + '_sem'])
            delta = values['com_framework'] - values['sem_framework']
            assert delta == int(row[kind + '_delta_bytes'])
            assert abs(100 * delta / values['sem_framework'] - float(row[kind + '_delta_percent'])) < .0001
    return functional, conformance, code, memory, comparison, metrics, simulation, led_rows, signal_rows, inputs


def integer(n):
    return f'{int(n):,}'.replace(',', '.')


def decimal(n, places=2):
    value = Decimal(str(n)).quantize(Decimal(1).scaleb(-places), rounding=ROUND_HALF_UP)
    return f'{value:.{places}f}'.replace('.', ',')


def percent(n):
    return decimal(n) + '%'


def memory_percent(row, kind):
    # Recompute from integer bytes: never round an already rounded CSV percentage.
    return percent(Decimal(100) * (int(row[kind + '_com']) - int(row[kind + '_sem']))
                   / int(row[kind + '_sem']))


def esc(value):
    return ''.join({'\\': r'\textbackslash{}', '&': r'\&', '%': r'\%', '_': r'\_', '#': r'\#',
                    '{': r'\{', '}': r'\}'}.get(c, c) for c in str(value))


def tabular(headers, data, alignment=None):
    return ('% Gerado por tools/preparar_material_artigo.py; consultar evidencias/artigo/TABELAS.md.\n'
            + r'\begin{tabular}{' + (alignment or 'l' * len(headers)) + '}\n' + r'\hline' + '\n'
            + ' & '.join(map(esc, headers)) + r' \\' + '\n' + r'\hline' + '\n'
            + '\n'.join(' & '.join(map(esc, r)) + r' \\' for r in data)
            + '\n' + r'\hline' + '\n' + r'\end{tabular}' + '\n')


def table_content(state):
    functional, conformance, code, memory, comparison, metrics, simulation, leds, signals, _ = state
    result = {}
    result['funcionamento'] = (['Placa', 'Versão', 'I', 'L', 'B', 'U', 'FS', 'RS', 'FD', 'RD'],
        [[NAMES[r['placa']], 'Com' if r['versao'] == 'com_framework' else 'Sem',
          *['A' if r[f] == 'APROVADO' else r[f] for f in FIELDS]] for r in functional])
    physical, simulation_rows = [], []
    for b in BOARDS:
        physical.append([NAMES[b], *[next(r['resultado_fisico'] for r in conformance if r['placa'] == b and r['grupo'] == g)
                                     .replace('APROVADO_FISICO', 'A').replace('LIMITADO', 'L') for g in ('GPIO', 'I2C', 'UART', 'TIMER')]])
        for group in ('GPIO', 'I2C', 'UART', 'TIMER'):
            cases = {k: v for k, v in simulation['targets'][b]['cases'].items() if k.startswith(group + '-')}
            counts = Counter(c['status'] for c in cases.values())
            simulation_rows.append(dict(placa=b, grupo=group,
                resultado_fisico=next(r['resultado_fisico'] for r in conformance if r['placa'] == b and r['grupo'] == group),
                casos_simulados=','.join(cases), aprovados_simulados=counts['APROVADO_SIMULADO'], nao_aplicaveis=counts['NAO_APLICAVEL']))
    result['conformidade-fisica'] = (['Placa', 'GPIO', 'I2C', 'UART', 'TIMER'], physical)
    result['conformidade-simulada'] = (['Placa', 'GPIO S/NA', 'I2C S/NA', 'UART S/NA', 'TIMER S/NA'],
        [[NAMES[b], *[f"{r['aprovados_simulados']}/{r['nao_aplicaveis']}" for r in simulation_rows if r['placa'] == b]] for b in BOARDS])
    result['codigo-dependencias'] = (['Placa', 'Versão', 'C/H', 'LOC', 'Comum', 'Origem (%)', 'Idêntico (%)', 'C/H SDK', 'SDK (%)'],
        [[NAMES[r['projeto'].split('_')[0]], 'Sem' if '_sem_' in r['projeto'] else 'Com', r['arquivos_autorais'],
          integer(r['loc_total']), integer(r['loc_comum']), percent(r['pcomp_percent']), percent(r['percent_identico_nas_quatro_por_conteudo']),
          r['arquivos_sdk_direto'], percent(r['percent_arquivos_sdk_direto'])] for r in code])
    result['controle-formatacao'] = (['Placa', 'Versão', 'LOC originais', 'LOC formatadas', 'Pcomp formatado'],
        [[NAMES[r['projeto'].split('_')[0]], 'Sem' if '_sem_' in r['projeto'] else 'Com', integer(r['loc_total']),
          integer(r['loc_normalizadas_total']), percent(r['pcomp_normalizado_percent'])] for r in code])
    result['codigo-mantido'] = (['Forma', 'LOC checkouts', 'LOC por origem', 'LOC formatadas por origem', 'Arquivos/versões'],
        [['Com' if f == 'framework' else 'Sem', *[integer(metrics['maintained_totals'][f][k]) for k in
          ('loc_somadas_quatro_checkouts', 'loc_por_origem_e_conteudo', 'loc_normalizadas_por_origem_e_conteudo', 'origens_versoes_distintas')]]
         for f in ('framework', 'sem_framework')])
    for kind in ('flash', 'ram'):
        result[kind] = (['Placa', 'Sem (B)', 'Com (B)', 'Acréscimo (B)', 'Acréscimo (%)'],
            [[NAMES[r['placa']], integer(r[kind + '_sem']), integer(r[kind + '_com']), integer(r[kind + '_delta_bytes']),
              memory_percent(r, kind)] for r in comparison])
    result['led'] = (['Placa', 'Ciclos', 'Média (ms)', 'Mín. (ms)', 'Máx. (ms)', 'Desvio (%)'],
        [[NAMES[r['placa']], r['ciclos'], decimal(r['periodo_medio_ms'], 3), decimal(r['minimo_ms'], 3),
          decimal(r['maximo_ms'], 3), decimal(r['desvio_nominal_percent'], 3) + '%'] for r in leds])
    result['sinais'] = (['Placa', 'Leituras I2C', 'Registros UART', 'Erros stop bit'],
        [[NAMES[r['placa']], r['leituras_completas'], r['registros_uart'], r['erros_stop_bit']] for r in signals])
    return result, simulation_rows


def csv_content(data):
    out = io.StringIO(newline='')
    writer = csv.DictWriter(out, fieldnames=list(data[0]), delimiter=';', lineterminator='\n')
    writer.writeheader(); writer.writerows(data)
    return out.getvalue()


def prepare_text(state):
    tables, sim = table_content(state)
    titles = {'funcionamento': 'Funcionamento das oito versões', 'conformidade-fisica': 'Conformidade física no recorte observado',
              'conformidade-simulada': 'Casos simulados por grupo', 'codigo-dependencias': 'Código autoral e dependências diretas',
              'controle-formatacao': 'Controle da sensibilidade à formatação', 'codigo-mantido': 'Código mantido por origem',
              'flash': 'Flash da imagem da aplicação', 'ram': 'RAM estática e reservada', 'led': 'Periodicidade do LED em erro',
              'sinais': 'Conferência independente das capturas com framework'}
    notes = {
        'funcionamento': 'A = APROVADO, informado pelo responsável nas sessões de 07–08/11/2026. I: inicialização; L: leituras; B: botão/telas; U: UART; FS/RS: falha/recuperação do sensor; FD/RD: falha/recuperação do display. Os 64 campos aprovados resumem o recorte ensaiado. No STM32F407 sem framework, a recuperação do OLED inclui uma manipulação adicional das conexões e erros simultâneos; consultar LIMITACOES.csv e as observações originais.',
        'conformidade-fisica': 'A = APROVADO_FISICO somente para os casos observados; L = LIMITADO. GPIO: níveis e comportamento; I2C: endereços, comando 24 00, STOP, leitura separada e falha/retomada; UART: conteúdo e CR LF decodificados a 115200 8N1. TIMER: LED, evidência indireta; interrupções de 1 ms não medidas. A remoção integral dos módulos interrompe alimentação e comunicação. Não representa aprovação física de todos os contratos.',
        'conformidade-simulada': 'Cada célula mostra APROVADO_SIMULADO/NAO_APLICAVEL para casos distintos do respectivo grupo, por placa; não são repetições estatísticas. SYS, CFG, RES e BSP ficam na matriz completa. No conjunto completo: 95 aprovações e uma não aplicável. TIMER-03 não se aplica ao ESP32. A simulação usa fontes reais dos portes e substitutos do SDK no computador; não executa os binários embarcados. A seção HW da matriz simulada preserva o estado histórico de 02/11; os resultados físicos posteriores estão na tabela anterior e no CSV da campanha.',
        'codigo-dependencias': 'LOC = coluna code do cloc 2.06 nos C/H autorais usados nos builds. Origem (%) corresponde a Pcomp = 100 × comum/(comum + específico), com origem mantida comum às quatro placas. Idêntico (%) considera o mesmo conteúdo, inclusive cópias em origens independentes. Nas versões convencionais, Pcomp = 0 por manutenção em caminhos independentes; existem 11 arquivos autorais idênticos, 486 LOC por versão. No framework, são 44 C/H autorais comuns e 1.815 LOC; o 45º C/H comum é fonte gráfica gerada, excluída de LOC e incluída no binário. Dependência direta: inclusão, tipo ou chamada de SDK revisada no inventário; a contagem absoluta é maior no framework em ESP32/nRF. As dependências ficam nos portes, BSP e integração; nas convencionais também aparecem na aplicação, dispositivos e telemetria.',
        'controle-formatacao': 'Contagens feitas em cópias com a mesma formatação; os fontes compilados foram preservados. Pcomp continua usando origem compartilhada. LOC não mede diretamente complexidade, qualidade ou esforço.',
        'codigo-mantido': 'Origens são contadas uma única vez com seu conteúdo. Cópias convencionais idênticas permanecem origens separadas. O porte STM32 é compartilhado pelas duas placas: sete C/H e 1.080 LOC, específicos do ambiente STM32, fora do comum às quatro placas.',
        'flash': 'Bytes do BIN da aplicação, incluindo dados inicializados e empacotamento/alinhamento. Bootloader separado e áreas externas à imagem excluídos. Acréscimos em relação à versão sem framework, calculados diretamente dos bytes inteiros e arredondados uma única vez para apresentação. ESP32: default (-Og); nRF/STM32: Debug (-Og).',
        'ram': 'Bytes estáticos/reservados por ELF/MAP, incluindo noinit e reservas existentes de stack/heap sem dupla contagem. Não mede pico durante a execução. Os acréscimos abrangem as implementações completas e não isolam o custo de uma chamada abstrata.',
        'led': 'Janela estável de ciclos completos durante falha do sensor; nominal de 1.000 ms. Desvio calculado sobre a média. Não mede diretamente as interrupções de 1 ms e não isola timer, laço, bloqueios ou aglutinação. As janelas, CSVs e valores sem arredondamento estão em led.csv; o roteiro não estabelece tolerância rígida. TIMER permanece LIMITADO nas quatro placas.',
        'sinais': 'Decodificação das transições digitais dos arquivos .sal a 2 MHz, preservada em conferencia-sal.json de cada sessão. Nas 290 leituras completas: comando 24 00, STOP antes da leitura separada de seis bytes, CRC e valores correspondentes à UART. Mensagens completas com CR LF. Nenhum erro de stop bit foi detectado com decodificador configurado a 115200 8N1; baud físico não medido. ESP32: três ERROR adicionais ausentes dos logs atuais permanecem na captura e fora da janela principal. Ver sinais.csv e LIMITACOES.csv.'}
    sources = {'funcionamento': '../resultados-funcionais.csv', 'conformidade-fisica': '../resultados-conformidade.csv',
               'conformidade-simulada': '../simulacao/conformidade/resultados.json',
               'codigo-dependencias': '../metricas/codigo.csv', 'controle-formatacao': '../metricas/codigo.csv',
               'codigo-mantido': '../metricas/resultados.json', 'flash': '../metricas/comparacao-memoria.csv',
               'ram': '../metricas/comparacao-memoria.csv', 'led': 'led.csv', 'sinais': 'sinais.csv'}
    text = ['# Tabelas para o artigo do TCC2', '', 'Fechamento do passo 7 em 09/11/2026. Valores conferidos contra os registros de origem. Com/Sem indicam presença/ausência do framework. As versões finais em LaTeX estão em `tcc_2/tabelas/`; cada arquivo contém somente o ambiente `tabular`, pronto para receber legenda e posição no passo 8.', '']
    outputs = {}
    for name, (headers, data) in tables.items():
        text += ['## ' + titles[name], '', '| ' + ' | '.join(headers) + ' |', '| ' + ' | '.join(['---'] * len(headers)) + ' |']
        text += ['| ' + ' | '.join(map(str, r)) + ' |' for r in data]
        text += ['', notes[name], '', f'Fonte: [{sources[name]}]({sources[name]}). LaTeX: [{name}.tex](../../tcc_2/tabelas/{name}.tex).', '']
        outputs[TAB / (name + '.tex')] = tabular(headers, data, 'llrrrrrrr' if name == 'codigo-dependencias' else None)
    outputs[BASE / 'TABELAS.md'] = '\n'.join(text)
    outputs[BASE / 'conformidade.csv'] = csv_content(sim)
    outputs[BASE / 'led.csv'] = csv_content(state[7])
    outputs[BASE / 'sinais.csv'] = csv_content(state[8])
    # A small editorial selection, preserving literal source lines and explicit gaps.
    folder = session_path(next(r for r in state[0] if r['placa'] == 'stm32f411' and r['versao'] == 'com_framework'))
    source = folder / read_json(folder / 'sessao.json')['uart_logs'][0]['text']
    lines = source.read_text().splitlines()
    blocks = [('Leitura normal e falha do sensor', [29, 30, 31]), ('Recuperação do sensor', [47, 48, 49]),
              ('Falha do display com aquisição mantida', [55, 56, 57, 58]), ('Recuperação do display', [81, 82, 83])]
    selected = []
    uart = ['# Seleção da UART do STM32F411 com framework', '', f'Fonte: [{source.name}]({source.relative_to(BASE, walk_up=True)}).',
            '', 'Horários são de recepção no computador. Cada bloco conserva o texto literal das linhas indicadas; as lacunas entre blocos são cortes editoriais. A ausência de novo ERROR nos blocos finais complementa a recuperação visual confirmada nas observações; sozinha não comprova a tela. O arquivo .bin preserva os bytes CR LF no fio.', '']
    for title, indices in blocks:
        uart += ['## ' + title, '', f'Linhas {indices[0]}–{indices[-1]} do log original.', '', '```text']
        for n in indices:
            selected.append(dict(bloco=title, linha_origem=n, texto=lines[n - 1], fonte=relative(source)))
            uart.append(lines[n - 1])
        uart += ['```', '']
    outputs[BASE / 'UART.md'] = '\n'.join(uart)
    outputs[BASE / 'uart.csv'] = csv_content(selected)
    # Literal lines with gaps are suitable for a LaTeX verbatim listing.
    outputs[TAB / 'uart.tex'] = ('% Recortes editoriais; conferir linhas e lacunas em evidencias/artigo/UART.md.\n'
        + '\n'.join(r'\begin{verbatim}' + '\n' + '\n'.join(lines[n-1] for n in indices) + '\n' + r'\end{verbatim}' for _, indices in blocks) + '\n')
    blocks = []
    for kind, title in (('flash', 'Flash da aplicação'), ('ram', 'RAM estática e reservada')):
        data = [[NAMES[r['placa']], integer(r[kind + '_sem']), integer(r[kind + '_com']),
                 integer(r[kind + '_delta_bytes']), memory_percent(r, kind).removesuffix('%')]
                for r in state[4]]
        body = ('\\begin{tabular}{lrrrr}\n\\hline\n'
                'Placa & Sem & Com & $\\Delta$ B & $\\Delta$ \\% \\\\\n\\hline\n'
                + '\n'.join(' & '.join(row) + r' \\' for row in data)
                + '\n\\hline\n\\end{tabular}\n')
        blocks.append('\\begin{minipage}{.49\\textwidth}\n\\centering\n\\textbf{' + title
                      + '}\\par\\smallskip\n' + body + '\\end{minipage}\n')
    outputs[TAB / 'memoria-artigo.tex'] = ('% Derivado dos bytes inteiros de evidencias/metricas/comparacao-memoria.csv.\n'
                                            + '\\hfill\n'.join(blocks))
    return outputs


def photo_selections(state):
    folders = {r['placa']: session_path(r) for r in state[0] if r['versao'] == 'com_framework'}
    return [dict(figura='montagens', painel='a', placa='esp32', fonte=relative(folders['esp32'] / '00_montagem.jpeg'), crop=[0, 120, 1030, 1599], rotacao=0),
            dict(figura='montagens', painel='b', placa='nrf52840', fonte=relative(folders['nrf52840'] / '00_montagem_substituir.jpeg'), crop=[0, 0, 1600, 1200], rotacao=-90),
            dict(figura='montagens', painel='c', placa='stm32f407', fonte=relative(folders['stm32f407'] / '00_montagem.jpeg'), crop=[0, 60, 1170, 1599], rotacao=0),
            dict(figura='montagens', painel='d', placa='stm32f411', fonte=relative(folders['stm32f411'] / '00_montagem.jpeg'), crop=[200, 0, 815, 1599], rotacao=0),
            dict(figura='telas', painel='a', placa='esp32', fonte=relative(folders['esp32'] / '01_tela_leituras.jpeg'), crop=[410, 850, 840, 1275], rotacao=0),
            dict(figura='telas', painel='b', placa='esp32', fonte=relative(folders['esp32'] / '02_tela_diagnostico.jpeg'), crop=[375, 850, 800, 1275], rotacao=0)]


def make_photos(selections):
    from PIL import Image
    from reportlab.pdfgen import canvas
    from reportlab.pdfbase import pdfmetrics
    from reportlab.pdfbase.ttfonts import TTFont
    pdfmetrics.registerFont(TTFont('DV', '/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf'))
    for name, width, height, positions in [('montagens', 516, 211, [(6, 25, 120, 180), (134, 25, 120, 180), (262, 25, 120, 180), (390, 25, 120, 180)]),
                                          ('telas', 252, 137, [(0, 18, 120, 119), (132, 18, 120, 119)])]:
        c = canvas.Canvas(str(FIG / (name + '.pdf')), pagesize=(width, height), invariant=1)
        c.setTitle('Montagens da prova de conceito' if name == 'montagens' else 'Telas da aplicação')
        for item, (x, y, w, h) in zip([s for s in selections if s['figura'] == name], positions):
            im = Image.open(ROOT / item['fonte'])
            assert 0 <= item['crop'][0] < item['crop'][2] <= im.width and 0 <= item['crop'][1] < item['crop'][3] <= im.height
            # PDF viewport: clipping and rigid rotation only. Original photo pixels are embedded intact.
            left, top, right, bottom = item['crop']
            cw, ch = right-left, bottom-top
            if item['rotacao']:
                cw, ch = ch, cw
            scale = min(w/cw, h/ch)
            vx, vy = x+(w-cw*scale)/2, y+(h-ch*scale)/2
            c.saveState()
            clip=c.beginPath(); clip.rect(vx,vy,cw*scale,ch*scale); c.clipPath(clip,stroke=0)
            c.translate(vx,vy); c.scale(scale,scale)
            if item['rotacao'] == -90:
                c.translate(0, ch); c.rotate(-90)
            c.drawImage(str(ROOT / item['fonte']), -left, -(im.height-bottom), width=im.width, height=im.height)
            c.restoreState()
            c.setFont('DV', 9 if name == 'montagens' else 8)
            label = NAMES[item['placa']] if name == 'montagens' else ('Leituras' if item['painel'] == 'a' else 'Diagnóstico')
            c.drawCentredString(x+w/2, 5, f"({item['painel']}) {label}")
        c.showPage(); c.save()


def make_architecture():
    from reportlab.pdfgen import canvas
    from reportlab.pdfbase import pdfmetrics
    from reportlab.pdfbase.ttfonts import TTFont
    from reportlab.lib.colors import HexColor
    pdfmetrics.registerFont(TTFont('DV', '/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf'))
    pdfmetrics.registerFont(TTFont('DVB', '/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf'))
    c=canvas.Canvas(str(FIG/'arquitetura.pdf'), pagesize=(516,360), invariant=1)
    c.setTitle('Arquitetura implementada do framework')
    def label(x,y,s,size=8,bold=False):
        c.setFillColor(HexColor('#17212b'));c.setFont('DVB' if bold else 'DV',size);c.drawCentredString(x,y,s)
    def box(x,y,w,h,title,lines,shared=False):
        c.setFillColor(HexColor('#eef3f6' if shared else '#ffffff'));c.setStrokeColor(HexColor('#455463'));c.setLineWidth(.65)
        c.rect(x,y,w,h,fill=1,stroke=1); label(x+w/2,y+h-14,title,9,True)
        for i,line in enumerate(lines): label(x+w/2,y+h-27-i*11,line,8)
    def arrow(points,dashed=False):
        import math
        c.setStrokeColor(HexColor('#455463'));c.setFillColor(HexColor('#455463'));c.setLineWidth(.7);c.setDash(3,2) if dashed else c.setDash()
        p=c.beginPath();p.moveTo(*points[0])
        for point in points[1:]:p.lineTo(*point)
        c.drawPath(p);c.setDash()
        x,y=points[-1];a=math.atan2(y-points[-2][1],x-points[-2][0]);p=c.beginPath();p.moveTo(x,y)
        for sign in (-1,1):p.lineTo(x-4*math.cos(a)+sign*2*math.sin(a),y-4*math.sin(a)-sign*2*math.cos(a))
        p.close();c.drawPath(p,fill=1,stroke=0)
    label(161,347,'Código comum e portes de plataforma',9,True)
    label(416,347,'Composição por placa',9,True)
    box(6,284,310,49,'Aplicação', ['Aquisição, telas, botão, LED e telemetria', 'Processamento sequencial; tempo recebido por função'],True)
    box(6,216,310,49,'Componentes', ['SHT31-D, SSD1306, botão, LED e telemetria', 'Recebem handles e configuração'],True)
    box(6,148,310,49,'Contratos públicos da HAL', ['GPIO, I2C, UART, timer e sistema', 'Tipos opacos; sem tipos dos SDKs'],True)
    box(6,75,310,49,'Portes', ['ESP32 / nRF52840 / STM32F4', 'STM32F4 usado por F407 e F411'])
    box(6,11,310,45,'SDKs e hardware', ['ESP-IDF / nRF Connect SDK (Zephyr) / STM32CubeF4 HAL'])
    box(333,268,177,65,'Integração (main)', ['Contextos e injeção de recursos', 'Callback ISR: contador nominal', 'Início do timer e laço da aplicação'])
    box(333,171,177,65,'BSP da placa', ['Pinos, barramentos e polaridades', 'Constrói recursos e sela a arena', 'Buffer persistente da HAL'])
    box(347,75,163,49,'Arena C11', ['hal/memory: alocação e selagem', 'Sem dependência de SDK'],True)
    arrow([(161,284),(161,265)])
    arrow([(161,216),(161,197)])
    arrow([(161,124),(161,148)]);label(235,134,'implementa hal_*',7)
    arrow([(161,75),(161,56)])
    arrow([(316,99),(347,99)])
    arrow([(333,307),(316,307)],True)
    arrow([(422,268),(422,236)],True)
    arrow([(333,203),(324,203),(324,165),(316,165)],True)
    c.showPage();c.save()


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check',action='store_true')
    parser.add_argument('--preserve-figures', action='store_true', help='Atualiza tabelas e manifesto sem regenerar as figuras existentes.')
    args=parser.parse_args()
    state=load_and_validate()
    texts=prepare_text(state)
    selections=photo_selections(state)
    texts[BASE/'selecao-figuras.json']=json.dumps(selections,ensure_ascii=False,indent=2)+'\n'
    if args.check:
        for path,text in texts.items():
            assert path.read_bytes()==text.encode(), f'Tabela/seleção divergente: {path}'
        manifest=read_json(BASE/'manifesto.json')
        assert sha(Path(__file__)) == manifest['generator_sha256'], 'Gerador alterado desde a preparação.'
        for section in ('inputs_sha256','outputs_sha256'):
            for path,expected in manifest[section].items():
                assert sha(ROOT/path)==expected, f'Hash divergente: {path}'
        print(f"Passo 7 conferido: 64 campos funcionais, 16 grupos físicos/simulados, oito métricas de código/memória, 290 leituras I2C; {len(manifest['outputs_sha256'])} arquivos de saída íntegros.")
        return
    for path,text in texts.items():
        path.parent.mkdir(parents=True,exist_ok=True);path.write_text(text)
    FIG.mkdir(parents=True,exist_ok=True)
    if not args.preserve_figures:
        make_photos(selections);make_architecture()
    else:
        assert all((FIG / (name + '.pdf')).is_file() for name in ('montagens', 'telas', 'arquitetura'))
    inputs=state[-1]
    inputs.update(ROOT/s['fonte'] for s in selections)
    for project in ('esp32_framework','nrf52840_framework','stm32f407_framework','stm32f411_framework'):
        inputs.add(ROOT/project/'src/main.c')
        inputs.update((ROOT/project/'src/bsp').rglob('*.c'))
    for source in (ROOT/'esp32_framework/src').rglob('*'):
        if source.suffix in ('.c','.h') and ('app' in source.parts or 'components' in source.parts or 'hal' in source.parts):
            inputs.add(source)
    outputs=set(texts)|set(FIG.glob('*.pdf'))|{BASE/'README.md', BASE/'OBSERVACOES.md', BASE/'LIMITACOES.csv', FIG/'arquitetura.mmd'}
    assert all(p.is_file() for p in outputs)
    manifest=dict(schema=1,step=7,prepared_at=datetime.now(timezone.utc).isoformat(),
                  source_revision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),
                  generator=relative(Path(__file__)), generator_sha256=sha(Path(__file__)),
                  upstream_input_check=state[5]['_step7_input_check'],
                  inputs_sha256={relative(p):sha(p) for p in sorted(inputs)},
                  outputs_sha256={relative(p):sha(p) for p in sorted(outputs)},
                  physical_tests_rerun=False,firmware_changed=False,article_written=False)
    (BASE/'manifesto.json').write_text(json.dumps(manifest,ensure_ascii=False,indent=2)+'\n')
    print('Tabelas, seleção UART, três figuras PDF e manifesto do passo 7 preparados.')


if __name__=='__main__':
    main()
