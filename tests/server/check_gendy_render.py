#!/usr/bin/env python3
import argparse,math
from check_round2_render import read_float_wav
p=argparse.ArgumentParser();p.add_argument('wav');p.add_argument('--duration',type=float,default=3);p.add_argument('--mode',default='integration');a=p.parse_args()
c,rate,x=read_float_wav(a.wav)
assert c=={'integration':25,'stress':26,'endurance':2}[a.mode]
assert len(x)//c>=a.duration*rate
assert all(math.isfinite(v) for v in x)
if a.mode=='endurance':
    assert not any(x[1::2]);assert any(x[::2])
else:
    for j in range(11,25):assert not any(x[j::c]),('seed/reset/zero-extension mismatch',j)
    for j in range(7):
        y=x[j::c];assert max(abs(v) for v in y)<=1.000001;assert max(abs(v) for v in y)>.01,('silent waveform',j)
    for j in range(7,11):assert all(0<=v<=1 for v in x[j::c])
    if a.mode=='stress':assert not any(x[25::c]),'stress non-finite state'
print(f'GENDY_RENDER_OK: {len(x)//c} frames, {c} channels')
