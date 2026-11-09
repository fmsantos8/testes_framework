#!/usr/bin/env python3
"""Check metric provenance, arithmetic, exclusions and unchanged build inputs."""
import csv
from collections import defaultdict
from datetime import date, datetime, timezone
import hashlib
import json
from pathlib import Path

from preparar_firmwares import ROOT, PROJECTS, source_hashes

BASE = ROOT / 'evidencias/metricas'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def rows(name):
    with (BASE / name).open(newline='', encoding='utf-8') as stream:
        data = list(csv.DictReader(stream, delimiter=';'))
    assert all(None not in row and all(v is not None for v in row.values()) for row in data), name
    return data


def check_physical_control(manifest, path):
    """Allow campaign progress while preserving build identity in every row."""
    with path.open(newline='', encoding='utf-8') as stream:
        control = list(csv.DictReader(stream, delimiter=';'))
    assert len(control) == len(PROJECTS), 'Quantidade de versões em controle.csv'
    seen, states = set(), defaultdict(int)
    for row in control:
        assert row['versao'] in ('com_framework', 'sem_framework'), row
        project = row['placa'] + ('_framework' if row['versao'] == 'com_framework' else '_sem_framework')
        assert project in PROJECTS and project not in seen, project
        seen.add(project)
        build = manifest['builds'][project]
        elf = next(a for a in build['artifacts'] if a['path'].endswith('.elf'))
        assert row['projeto'] == project and row['configuracao'] == build['configuration'], project
        assert row['binario'] == elf['path'] and row['sha256'] == elf['sha256'], project
        status = row['resultado']
        assert status in ('PENDENTE', 'APROVADO', 'FALHOU', 'NAO_EXECUTADO'), project
        if status == 'PENDENTE':
            assert not row['data_ensaio'], project
        else:
            assert date.fromisoformat(row['data_ensaio']).isoformat() == row['data_ensaio'], project
        states[status] += 1
    assert seen == set(PROJECTS)
    return {'approved_versions': states['APROVADO'], 'failed_versions': states['FALHOU'],
            'not_executed_versions': states['NAO_EXECUTADO'], 'pending_versions': states['PENDENTE']}


def main():
    report = json.loads((BASE / 'resultados.json').read_text())
    manifest_path = ROOT / 'evidencias/firmwares/manifesto.json'
    manifest = json.loads(manifest_path.read_text())
    assert sha(manifest_path) == report['build_manifest_sha256']
    assert sha(BASE / 'regras.json') == report['rules_sha256']
    assert sha(ROOT / 'tools/coletar_metricas.py') == report['runner_sha256']
    for name, fingerprint in report['outputs_sha256'].items():
        assert sha(BASE / name) == fingerprint, name
    for name, fingerprint in report['input_files'].items():
        assert sha(Path(name)) == fingerprint, name
    inventory = rows('inventario.csv')
    assert len({(r['projeto'], r['arquivo']) for r in inventory}) == len(inventory)
    assert {r['projeto'] for r in inventory} == set(PROJECTS)
    code = {r['projeto']: r for r in rows('codigo.csv')}
    original_cloc = json.loads((BASE / 'dados/cloc.json').read_text())
    normalized_cloc = json.loads((BASE / 'dados/cloc-normalizado.json').read_text())
    author = [r for r in inventory if r['categoria'] == 'autoral']
    for row in inventory:
        path = ROOT / row['arquivo']
        assert sha(path) == row['sha256'], row['arquivo']
        counted = original_cloc.get(str(path), {'code': 0})
        if str(path) not in original_cloc:
            assert path.stat().st_size == 0
        assert int(row['loc']) == counted['code'], row['arquivo']
        if row['categoria'] == 'autoral':
            copy = BASE / 'dados/normalizado' / row['arquivo']
            assert int(row['loc_normalizadas']) == normalized_cloc[str(copy)]['code']
        else:
            assert row['comum_global'] == '0'
    for name in PROJECTS:
        selected = [r for r in author if r['projeto'] == name]
        common = [r for r in selected if r['comum_global'] == '1']
        total = sum(int(r['loc']) for r in selected)
        lcommon = sum(int(r['loc']) for r in common)
        assert total == int(code[name]['loc_total'])
        assert lcommon == int(code[name]['loc_comum'])
        assert total - lcommon == int(code[name]['loc_especifico'])
        assert abs(float(code[name]['pcomp_percent']) - 100 * lcommon / total) < 0.0001
        assert len(selected) == int(code[name]['arquivos_autorais'])
        assert sum(int(r['depende_sdk']) for r in selected) == int(code[name]['arquivos_sdk_direto'])
        normalized = sum(int(r['loc_normalizadas']) for r in selected)
        assert normalized == int(code[name]['loc_normalizadas_total'])
        normalized_common = sum(int(r['loc_normalizadas']) for r in common)
        assert normalized_common == int(code[name]['loc_normalizadas_comum'])
        assert abs(float(code[name]['pcomp_normalizado_percent']) - 100 * normalized_common / normalized) < 0.0001
        for r in common:
            assert r['condicional_sdk'] == '0'
        if not name.endswith('_sem_framework'):
            core = [r for r in selected if r['camada'] in ('aplicacao', 'componentes', 'contratos', 'arena')]
            assert all(r['depende_sdk'] == '0' for r in core)
        build = manifest['builds'][name]
        assert source_hashes(ROOT / name) == build['source_sha256'], name
        for artifact in build['artifacts'] + build['configuration_files']:
            assert sha(ROOT / artifact['path']) == artifact['sha256']
            assert sha(ROOT / artifact['build_path']) == artifact['sha256']
        memory = json.loads((BASE / 'dados' / (name + '-memoria.json')).read_text())
        bin_name = 'zephyr.bin' if name.startswith('nrf52840') else name + '.bin'
        binary = next(a for a in build['artifacts'] if Path(a['path']).name == bin_name)
        assert memory['application_bin'] == binary['path']
        assert memory['flash_bytes'] == binary['bytes']
        assert memory['flash_bytes'] >= memory['flash_initialized_payload_bytes']
        assert memory['ram_bytes'] == sum(r['used_bytes'] for r in memory['ram_regions'])
        assert all(0 <= r['used_bytes'] <= r['length'] for r in memory['ram_regions'])
    for form in ('framework', 'sem_framework'):
        selected = [r for r in author if r['projeto'] in {b + '_' + form for b in ('esp32', 'nrf52840', 'stm32f407', 'stm32f411')}]
        origins = defaultdict(list)
        for row in selected:
            origins[(row['origem_mantida'], row['sha256'])].append(row)
        assert sum(int(v[0]['loc']) for v in origins.values()) == report['maintained_totals'][form]['loc_por_origem_e_conteudo']
        assert sum(int(v[0]['loc_normalizadas']) for v in origins.values()) == report['maintained_totals'][form]['loc_normalizadas_por_origem_e_conteudo']
        for values in origins.values():
            assert len({r['loc_normalizadas'] for r in values}) == 1
            if any(r['comum_global'] == '1' for r in values):
                assert len({r['projeto'] for r in values}) == 4
    for row in rows('comparacao-memoria.csv'):
        for kind in ('flash', 'ram'):
            a, b = int(row[kind + '_com']), int(row[kind + '_sem'])
            assert a - b == int(row[kind + '_delta_bytes'])
            assert abs(float(row[kind + '_delta_percent']) - 100 * (a - b) / b) < 0.0001
    ports = rows('reuso-stm32.csv')
    assert len(ports) == 7 and sum(int(r['loc']) for r in ports) == report['stm32_port']['loc_unique']
    campaign = check_physical_control(manifest, ROOT / 'evidencias/controle.csv')
    result = {'executed_utc': datetime.now(timezone.utc).isoformat(), 'status': 'APROVADO',
              'projects': 8, 'input_files_checked': len(report['input_files']),
              'outputs_checked': len(report['outputs_sha256']), 'inventory_rows': len(inventory),
              'sources_and_artifacts_unchanged': True, 'physical_validation': False,
              'physical_campaign_records': campaign,
              'metrics_report_sha256': sha(BASE / 'resultados.json'), 'verifier_sha256': sha(Path(__file__))}
    (BASE / 'verificacao.json').write_text(json.dumps(result, indent=2) + '\n')
    print('Métricas conferidas: oito projetos, exclusões, fórmulas, origens, memória e hashes válidos.')
    print(f"Fontes e binários conferidos; controle da campanha: {campaign['approved_versions']} versões aprovadas, "
          f"{campaign['failed_versions']} com falha, {campaign['not_executed_versions']} não executadas e "
          f"{campaign['pending_versions']} pendentes. A verificação das métricas não executa ensaios físicos.")


if __name__ == '__main__':
    main()
