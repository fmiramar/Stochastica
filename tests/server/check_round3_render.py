#!/usr/bin/env python3
"""Validate raw Round 3 output, including exact seeded pairs on each server."""
import argparse
import math
from check_round2_render import read_float_wav

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('wav')
    parser.add_argument('--duration', type=float, default=2)
    parser.add_argument('--mode', choices=['integration', 'stress', 'endurance'], default='integration')
    args = parser.parse_args()
    channels, rate, samples = read_float_wav(args.wav)
    assert channels == {'integration':112, 'stress':113, 'endurance':2}[args.mode]
    assert len(samples)//channels >= args.duration*rate, 'incomplete render'
    assert all(math.isfinite(x) for x in samples), 'non-finite raw output'
    if args.mode == 'endurance':
        assert not any(samples[1::2]), 'CheckBadValues detected non-finite raw state'
        assert any(samples[::2]), 'silent endurance render'
    else:
        for offset in (28, 84):
            for channel in range(offset, offset+28):
                assert not any(samples[channel::channels]), 'seeded pair mismatch'
        if args.mode == 'stress':
            assert not any(samples[112::channels]), 'non-finite stress state'
        start, stop = int(.15*rate), int(.5*rate)
        for offset in (0, 56):
            for frame in range(start, stop):
                # K2A linearly interpolates between control values. Integer/flag
                # invariants apply at block boundaries, not to that converter's ramps.
                if offset == 56 and frame % 64:
                    continue
                x = samples[frame*channels+offset:frame*channels+offset+28]
                assert all(0 <= v <= 1 for v in x[:3]) and abs(sum(x[:3])-1) < 2e-7, 'simplex'
                assert x[3] in (0,1,2) and x[4] in (110,220,330), 'urn state/value'
                assert 0 <= x[5] <= 1 and 0 <= x[6] <= 1, 'urn diagnostics'
                assert x[7] in (0,1) and x[8] in (0,1,2) and x[10] in (0,1), 'MMPP outputs'
                assert x[15] in (0,1) and x[17] >= 0 and x[17] == int(x[17]), 'jump flag/count'
            assert max(abs(v) for v in samples[offset+11::channels]) > 0, 'missing Matérn output'
    print(f'ROUND3_RENDER_OK {len(samples)//channels} frames; {channels} channels')

if __name__ == '__main__':
    main()
