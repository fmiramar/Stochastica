#!/usr/bin/env python3
"""Lint new help examples and export active blocks for the language smoke test."""
import argparse
import re
from pathlib import Path

NAMES=['SelfCorrectingTrig','RandomTelegraph','ShotNoise','TVonMisesRand','MultiOUProcess','MultiHawkesTrig','DoubleWellProcess','KuramotoNetwork']
def main():
    parser=argparse.ArgumentParser();parser.add_argument('--export',type=Path);args=parser.parse_args()
    root=Path(__file__).resolve().parents[2]
    pages=[root/'HelpSource/Classes'/f'{n}.schelp' for n in NAMES]+[root/'HelpSource/Guides/Stochastica.schelp']
    blocks=[]
    for page in pages:
        source=page.read_text()
        if page.stem!='Stochastica':
            assert source.startswith('class:: '+page.stem+'\n'),page
            assert 'subsection:: See also' in source and 'link::Guides/Stochastica::' in source,page
        examples=re.findall(r'code::\s*\n(.*?)\n::',source,re.S)
        assert examples,page
        for i,code in enumerate(examples):
            assert code.strip().startswith('(') and code.strip().endswith(')'),(page,i,'parentheses')
            for line in code.splitlines():
                if line.strip().startswith('//'):
                    assert not any(x in line for x in ('.ar(','.kr(','.play','.new(')),(page,i,'disabled code')
            if 'Buffer.' in code:
                assert 'Routine {' in code and 's.sync;' in code,(page,i,'buffer sync')
            blocks.append((page.stem,i,code))
    if args.export:
        args.export.mkdir(parents=True,exist_ok=True)
        for j,(name,i,code) in enumerate(blocks):
            # Shorten only demonstration cleanup waits; all graph code stays intact.
            code=re.sub(r'\b\d+\.wait;', '0.2.wait;',code)
            (args.export/f'{j:02d}-{name}-{i}.scd').write_text(code+'\n')
    print(f'ROUND2_DOC_LINT_OK: {len(pages)} pages, {len(blocks)} runnable examples')
if __name__=='__main__':main()
