#!/usr/bin/env python3
"""Obtain the exact published firmware revisions without building or flashing."""
import argparse
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]

def git(*args):
    subprocess.run(['git', *map(str, args)], check=True)

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--destino', type=Path, default=ROOT)
    parser.add_argument('--projeto', action='append')
    args = parser.parse_args()
    publication = json.loads((ROOT / 'manifesto-publicacao.json').read_text())
    selected = args.projeto or list(publication['projects'])
    for name in selected:
        if name not in publication['projects']:
            parser.error('Projeto desconhecido: ' + name)
        record = publication['projects'][name]
        target = args.destino.resolve() / name
        if target.exists():
            raise SystemExit('Destino já existe; preserve-o ou escolha outro --destino: ' + str(target))
        git('clone', '--no-checkout', record['url'], target)
        git('-C', target, 'checkout', '--detach', record['commit'])
        if (target / '.gitmodules').is_file():
            git('-C', target, 'submodule', 'update', '--init', '--recursive')
        print(name, record['commit'])

if __name__ == '__main__':
    main()
