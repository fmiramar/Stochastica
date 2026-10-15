#!/usr/bin/env python3
"""Check the complete inventory and Round 3 docs; export executable examples."""
import argparse
import re
from pathlib import Path

NAMES=['MaternNoise','TDirichletRand','PolyaUrn','MarkovModulatedTrig','LevyFlight','JumpDiffusion','WilsonCowan','Lorenz96']

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--export',type=Path)
    parser.add_argument('--core-help',type=Path,help='Optional installed SC HelpSource for local link validation')
    args=parser.parse_args()
    root=Path(__file__).resolve().parents[2]
    pages=sorted((root/'HelpSource/Classes').glob('*.schelp'))
    guide=root/'HelpSource/Guides/Stochastica.schelp'
    text=guide.read_text();inventory={p.stem for p in pages}
    declared=set()
    for source in (root/'Classes').glob('*.sc'):
        declared.update(re.findall(r'^(\w+)\s*(?::\s*\w+\s*)?\{',source.read_text(),re.M))
    assert declared==inventory,('class/help inventory mismatch',declared^inventory)
    assert inventory <= set(re.findall(r'link::Classes/(\w+)::',text)), 'guide omits a class'
    for label in ('Technical families','Forms of organization','Three meanings of regime',
                  'Four meanings of irregular movement','Four meanings of interaction','Deterministic chaos'):
        assert label in text,label
    blocks=[]
    for page in pages+[guide]:
        source=page.read_text()
        if page!=guide:
            assert source.startswith('class:: '+page.stem+'\n'),page
            assert 'Guides/Stochastica' in re.search(r'^related:: (.*)$',source,re.M).group(1),page
        for target in re.findall(r'link::([^#:\n]+)(?:::|#)',source):
            if target.startswith(('Classes/','Guides/')):
                exists=(root/'HelpSource'/f'{target}.schelp').exists()
                if args.core_help:exists=exists or (args.core_help/f'{target}.schelp').exists()
                if args.core_help:assert exists,(page,target)
        for asset in re.findall(r'image::([^#:\n]+)',source):
            assert (page.parent/asset).exists(),(page,asset)
        examples=re.findall(r'code::\s*\n(.*?)\n::',source,re.S)
        for i,code in enumerate(examples):
            assert code.strip().startswith('(') and code.strip().endswith(')'),(page,i,'outer parentheses')
            # New pages must contain active runnable code, with no disabled examples.
            if page.stem in NAMES:
                for line in code.splitlines():
                    if line.strip().startswith('//'):
                        assert not any(x in line for x in ('.ar(','.kr(','.play','.new(')),(page,i,'disabled code')
                if 'Buffer.' in code:assert 'Routine {' in code and 's.sync;' in code,(page,i,'buffer sync')
            if page.stem in NAMES or page==guide:blocks.append((page.stem,i,code))
        if page.stem in NAMES:
            assert len(examples)>=2 and 'image::round3-' in source,page
            assert source.index('related::')<source.index('description::'),page
    if args.export:
        args.export.mkdir(parents=True,exist_ok=True)
        for j,(name,i,code) in enumerate(blocks):
            code=re.sub(r'\b\d+\.wait;', '0.2.wait;',code)
            (args.export/f'{j:02d}-{name}-{i}.scd').write_text(code+'\n')
    print(f'ROUND3_DOC_LINT_OK: {len(pages)} class pages + guide; {len(blocks)} new/guide runnable examples')

if __name__=='__main__':main()
