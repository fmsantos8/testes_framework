#!/usr/bin/env python3
"""Verify preserved evidence and, optionally, the published firmware sources."""
import argparse
import csv
import hashlib
import json
from pathlib import Path
from decimal import Decimal, ROUND_HALF_UP

ROOT = Path(__file__).resolve().parents[1]

def sha(path):
    h = hashlib.sha256()
    with path.open('rb') as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b''):
            h.update(chunk)
    return h.hexdigest()

def rows(path):
    with path.open(newline='', encoding='utf-8') as stream:
        return list(csv.DictReader(stream, delimiter=';'))

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--fontes', type=Path, help='Diretório que contém os oito projetos obtidos por obter_fontes.py.')
    args = parser.parse_args()
    inventory = json.loads((ROOT / 'integridade.json').read_text())
    for name, record in inventory['files'].items():
        path = ROOT / name
        assert path.is_file(), 'Arquivo ausente: ' + name
        assert path.stat().st_size == record['bytes'] and sha(path) == record['sha256'], 'Arquivo divergente: ' + name
    builds = json.loads((ROOT / 'evidencias/firmwares/manifesto.json').read_text())['builds']
    for name, record in builds.items():
        for item in record['artifacts'] + record['configuration_files']:
            path = ROOT / item['path']
            assert sha(path) == item['sha256'], 'Artefato divergente: ' + str(path)
    comparisons = rows(ROOT / 'evidencias/metricas/comparacao-memoria.csv')
    for record in comparisons:
        for kind in ('flash', 'ram'):
            a, b = int(record[kind + '_com']), int(record[kind + '_sem'])
            assert a - b == int(record[kind + '_delta_bytes'])
            exact = Decimal(100) * (a - b) / b
            assert abs(exact - Decimal(record[kind + '_delta_percent'])) < Decimal('.0001')
            rounded = exact.quantize(Decimal('.01'), rounding=ROUND_HALF_UP)
            print(record['placa'], kind, f'{rounded}%')
    functional = rows(ROOT / 'evidencias/resultados-funcionais.csv')
    fields = ('inicializacao', 'leituras', 'botao_telas', 'uart', 'falha_sensor', 'recuperacao_sensor', 'falha_display', 'recuperacao_display')
    assert len(functional) == 8
    assert all(row[field] == 'APROVADO' for row in functional for field in fields)
    signals = rows(ROOT / 'evidencias/artigo/sinais.csv')
    assert sum(int(r['leituras_completas']) for r in signals) == 290
    print('Evidências íntegras:', len(inventory['files']), 'arquivos; 64 campos funcionais; 290 leituras I2C.')
    if args.fontes:
        publication = json.loads((ROOT / 'manifesto-publicacao.json').read_text())
        count = 0
        for name, record in publication['projects'].items():
            for relative, expected in record['historical_source_sha256'].items():
                path = args.fontes / name / relative
                if relative in record['metadata_changes']:
                    expected = record['metadata_changes'][relative]['published_sha256']
                assert path.is_file() and sha(path) == expected, 'Fonte divergente: ' + str(path)
                count += 1
        print('Fontes publicados conferidos:', count, 'arquivos; diferenças de URL em .gitmodules identificadas no manifesto.')
    print('Conferência concluída. Não foram compilados firmwares nem executados ensaios físicos.')

if __name__ == '__main__':
    main()
