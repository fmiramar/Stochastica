#!/usr/bin/env python3
"""Validate GENDY APIs, links, assets and export runnable examples."""
import argparse,re
from pathlib import Path
NAMES=['Gendy1977','Gendy1991','GendyS709','GendyField','GendyPerc','GendyInterp','GendyDiffusion','GendyCoupled']
p=argparse.ArgumentParser();p.add_argument('--export',type=Path);a=p.parse_args()
root=Path(__file__).resolve().parents[2];sc=(root/'Classes/SDGendy.sc').read_text();blocks=[]
for name in NAMES:
    page=root/'HelpSource/Classes'/f'{name}.schelp';s=page.read_text()
    assert s.startswith(f'class:: {name}\n')
    for guide in ['Guides/Stochastica','Guides/Stochastica-GENDY']:assert guide in re.search(r'^related:: (.+)$',s,re.M)[1]
    assert s.index('subsection:: See also')<s.index('classmethods::')
    args=[x.split('=')[0].strip() for x in re.search(name+r' : .*?\*\w+ \{ \|(.*?)\|',sc,re.S)[1].split(',')]
    assert re.findall(r'^argument:: (\w+)$',s,re.M)==args,(name,'documented argument mismatch')
    assert 'image::gendy-' in s
    for asset in re.findall(r'image::([^:]+)::',s):assert (page.parent/asset).is_file()
    examples=re.findall(r'code::\s*\n(.*?)\n::',s,re.S);assert len(examples)>=2
    blocks.extend((name,i,b) for i,b in enumerate(examples))
guide=root/'HelpSource/Guides/Stochastica-GENDY.schelp';s=guide.read_text()
for name in NAMES:assert f'link::Classes/{name}::' in s
assert 'GendyDiffusion' in sc and 'equation 19' in s
blocks.extend(('GENDY-guide',i,b) for i,b in enumerate(re.findall(r'code::\s*\n(.*?)\n::',s,re.S)))
for j,(name,i,b) in enumerate(blocks):
    assert b.strip().startswith('(') and b.strip().endswith(')'),(name,i)
    assert not re.search(r'^\s*//.*(?:\.ar\(|\.kr\(|\.play)',b,re.M)
    if a.export:
        a.export.mkdir(parents=True,exist_ok=True);(a.export/f'{j:02d}-{name}-{i}.scd').write_text(b+'\n')
print(f'GENDY_DOCS_OK: {len(NAMES)} classes, dedicated guide, {len(blocks)} runnable examples')
