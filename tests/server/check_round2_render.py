#!/usr/bin/env python3
"""Check unconditioned NRT channels and exact seeded pairs (standard library only)."""
import argparse
import array
import math
import struct
import sys
from pathlib import Path

def read_float_wav(path):
    raw=Path(path).read_bytes()
    if raw[:4] not in (b'RIFF',b'RF64') or raw[8:12]!=b'WAVE':
        raise ValueError('expected WAV')
    pos=12;fmt=None;data=None
    while pos+8<=len(raw):
        tag,size=struct.unpack_from('<4sI',raw,pos);pos+=8
        if tag==b'fmt ':fmt=raw[pos:pos+size]
        elif tag==b'data':data=raw[pos:pos+size]
        pos+=size+(size&1)
    kind,channels,rate=struct.unpack_from('<HHI',fmt)
    bits=struct.unpack_from('<H',fmt,14)[0]
    assert kind in (3,65534) and bits==32, (kind,bits)
    samples=array.array('f');samples.frombytes(data)
    if sys.byteorder!='little':samples.byteswap()
    return channels,rate,samples

def main():
    parser=argparse.ArgumentParser();parser.add_argument('wav');parser.add_argument('--stress',action='store_true');args=parser.parse_args()
    channels,rate,samples=read_float_wav(args.wav)
    assert channels in (72,73),channels
    frames=len(samples)//channels
    assert frames >= (10 if args.stress else 2)*rate, "render is incomplete"
    assert all(math.isfinite(v) for v in samples),'non-finite raw output'
    for offset in (18,54):
        assert all(samples[i]==0 for c in range(offset,offset+18) for i in range(c,len(samples),channels)), 'seeded pair mismatch'
    if args.stress:
        samples=samples[:int(rate*1.9)*channels]
    for offset in (0,36):
        for c in (6,14,15,17):
            assert all(-math.pi<=v<math.pi for v in samples[offset+c::channels]),'angle outside circle'
        assert all(0<=v<=1 for v in samples[offset+16::channels]),'order parameter range'
        assert max(samples[offset+1::channels])>0,'missing process output'
    print(f'ROUND2_RENDER_OK frames={frames} rate={rate} channels={channels}; finite raw channels and exact seeded pairs')
if __name__=='__main__':main()
