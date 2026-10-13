#!/usr/bin/env python3
"""Run installed-language, SCDoc, and both-server Round 3 checks locally."""
import argparse
import subprocess
import sys
import time
from pathlib import Path

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--sclang', default='sclang')
    parser.add_argument('--scsynth', default='scsynth')
    parser.add_argument('--supernova')
    parser.add_argument('--config', type=Path, required=True)
    parser.add_argument('--plugins', required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--endurance', type=float, default=0)
    parser.add_argument('--skip-docs', action='store_true', help='Server-only iteration during development')
    args=parser.parse_args()
    root=Path(__file__).resolve().parents[2]
    out=args.output.resolve();out.mkdir(parents=True,exist_ok=True)
    def run(name, command, timeout=180):
        log=out/(name+'.log');started=time.monotonic()
        with log.open('w') as stream:
            subprocess.run([str(x) for x in command],cwd=root,stdout=stream,stderr=subprocess.STDOUT,check=True,timeout=timeout)
        text=log.read_text(errors='replace')
        for marker in ('ERROR:', 'FAILURE IN SERVER', 'Exception when reading synthdef', 'exception in GraphDef_Load', 'Non-rt synthesis failed', 'SCDoc: WARNING'):
            if marker in text:raise RuntimeError(f'{name}: {marker}; see {log}')
        print(f'{name}: passed ({time.monotonic()-started:.2f}s)',flush=True)
        return text
    def language(name,script,*arguments):
        return run(name,[args.sclang,'-D','-l',args.config.resolve(),root/script,*arguments])
    if not args.skip_docs:
        run('doc-lint',[sys.executable,root/'tests/server/check_round3_docs.py'])
        text=language('scdoc','tests/server/round3_scdoc.scd')
        if 'ROUND3_SCDOC_OK' not in text or 'WARNING' in text:raise RuntimeError('SCDoc did not finish cleanly')
    modes=[('integration',2,112),('stress',10,113)]
    if args.endurance:modes.append(('endurance',args.endurance,2))
    for mode,duration,channels in modes:
        score=out/(mode+'.osc')
        text=language(mode+'-score','tests/server/round3_nrt.scd',score,duration,mode)
        if 'ROUND3_SCORE_OK' not in text:raise RuntimeError('Missing score completion marker')
        for server in [args.scsynth]+([args.supernova] if args.supernova else []):
            name=mode+'-'+Path(server).stem;wav=out/(name+'.wav')
            run(name,[server,'-D','0','-N',score,'_',wav,48000,'WAV','float','-o',channels,'-i',0,
                      '-w',4096,'-m',65536,'-U',args.plugins],timeout=max(180,duration*10))
            run(name+'-analysis',[sys.executable,root/'tests/server/check_round3_render.py',wav,
                                 '--mode',mode,'--duration',duration],timeout=300)
    print('ROUND3_CHECKS_OK',flush=True)

if __name__=='__main__':main()
