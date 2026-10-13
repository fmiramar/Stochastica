#!/usr/bin/env python3
"""Reproduce Round 2 server and documentation checks with explicit SC paths.

Requires installed SuperCollider and a previously built Stochastica extension.
Generated scores, logs, SynthDefs, and audio stay in the supplied output directory.
"""
import argparse
import math
import os
from pathlib import Path
import subprocess
import time

from check_round2_render import read_float_wav


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--sclang', default='sclang')
    parser.add_argument('--scsynth', default='scsynth')
    parser.add_argument('--supernova')
    parser.add_argument('--config', type=Path, required=True)
    parser.add_argument('--plugins', required=True, help='SC plugin search path, including core and Stochastica')
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--endurance', type=float, default=0, help='Optional endurance duration in seconds; use 600 for full verification')
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    args.output = args.output.resolve()
    args.output.mkdir(parents=True, exist_ok=True)
    args.config = args.config.resolve()

    def run(name, command, timeout=180):
        log = args.output / (name + '.log')
        started = time.monotonic()
        with log.open('w') as stream:
            subprocess.run([str(x) for x in command], cwd=root, stdout=stream,
                           stderr=subprocess.STDOUT, timeout=timeout, check=True)
        text = log.read_text(errors='replace')
        for marker in ('ERROR:', 'FAILURE IN SERVER', 'Exception when reading synthdef',
                       'exception in GraphDef_Load', 'Non-rt synthesis failed', 'SCDoc: WARNING'):
            if marker in text:
                raise RuntimeError(f'{name}: {marker}; see {log}')
        print(f'{name}: passed ({time.monotonic()-started:.2f}s)')
        return text

    def language(name, script, *arguments):
        return run(name, [args.sclang, '-D', '-l', args.config, root / script, *arguments])

    run('doc-lint', [os.sys.executable, root / 'tests/server/check_round2_docs.py'])
    text = language('scdoc', 'tests/server/round2_scdoc.scd')
    if 'ROUND2_SCDOC_OK' not in text or 'WARNING' in text:
        raise RuntimeError('SCDoc did not complete without warnings')
    modes = [('integration', 2, 72), ('stress', 10, 73)]
    if args.endurance:
        modes.append(('endurance', args.endurance, 2))
    for name, duration, channels in modes:
        score = args.output / (name + '.osc')
        script = 'tests/performance/round2_endurance.scd' if name == 'endurance' else 'tests/server/round2_nrt.scd'
        language(name + '-score', script, score, duration)
        for server in [args.scsynth] + ([args.supernova] if args.supernova else []):
            label = name + '-' + Path(server).stem
            wav = args.output / (label + '.wav')
            run(label, [server, '-D', '0', '-N', score, '_', wav, '48000', 'WAV', 'float',
                        '-o', channels, '-i', '0', '-w', '4096', '-m', '65536', '-U', args.plugins],
                timeout=max(180, duration*10))
            if name == 'endurance':
                count, rate, samples = read_float_wav(wav)
                assert count == 2 and len(samples)/count >= duration*rate
                assert all(math.isfinite(x) for x in samples), 'non-finite endurance output'
                assert not any(samples[1::2]), 'CheckBadValues detected non-finite raw UGen state'
                assert max(abs(x) for x in samples[::2]) > 0, 'endurance graph was silent'
                print(f'{label}: {len(samples)//2} frames; no non-finite raw channels')
            else:
                command = [os.sys.executable, root / 'tests/server/check_round2_render.py', wav]
                if name == 'stress':
                    command.append('--stress')
                run(label + '-analysis', command)
    print('ROUND2_CHECKS_OK')


if __name__ == '__main__':
    main()
