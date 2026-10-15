#!/usr/bin/env python3
"""Original DDSS reconstruction figure from the actual C++ engine and baseline."""
import argparse,csv
from collections import defaultdict
from pathlib import Path
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
p=argparse.ArgumentParser();p.add_argument('csv',type=Path)
p.add_argument('--output',type=Path,default=Path('HelpSource/Classes/gendy-gendydiffusion.png'));a=p.parse_args()
audio=defaultdict(list);points=defaultdict(list)
with a.csv.open() as f:
    for row in csv.DictReader(f):
        v=int(row['variant'])
        if row['kind']=='audio':audio[v].append(float(row['amplitude']))
        else:points[v].append([float(row[k]) for k in ('time','index','radius','amplitude')])
audio={v:np.asarray(x) for v,x in audio.items()};points={v:np.asarray(x).reshape(-1,36,4) for v,x in points.items()}
plt.rcParams.update({'font.size':10,'axes.spines.top':False,'axes.spines.right':False,'axes.grid':True,'grid.alpha':.18})
fig,ax=plt.subplots(2,2,figsize=(11,7));colors=['#587a39','#196d8a','#c55b35']
labels=['d=1: zero radial drift','d=2: DDSS reconstruction','simplified DSS (Gaussian)']
for v in (0,1):
    x=points[v];ax[0,0].plot(x[:,0,0],x[:,:,2].mean(axis=1),color=colors[v],label=labels[v])
    ax[0,1].plot(x[:,0,0],x[:,7,2],color=colors[v],label=labels[v])
ax[0,0].set(xlabel='audio seconds',ylabel='mean physical radius (36 particles)');ax[0,0].legend(fontsize=8)
ax[0,1].set(xlabel='audio seconds',ylabel='physical radius of particle 7');ax[0,1].legend(fontsize=8)
for v in (1,2):
    ax[1,0].plot(range(38),np.r_[0,points[v][-1,:,3],0],'.-',lw=1,color=colors[v],label=labels[v])
    # Equal analysis windows/scaling; mean subtraction is analysis-only.
    y=audio[v][48000:];y=(y-y.mean())*np.hanning(len(y))
    power=np.abs(np.fft.rfft(y))**2;freq=np.fft.rfftfreq(len(y),1/48000)
    edges=np.geomspace(40,24000,130);f=[];db=[]
    for lo,hi in zip(edges[:-1],edges[1:]):
        mask=(freq>=lo)&(freq<hi);f.append(np.sqrt(lo*hi));db.append(10*np.log10(power[mask].mean()/24000**2+1e-14))
    ax[1,1].semilogx(f,db,color=colors[v],label=labels[v])
ax[1,0].set(xlabel='breakpoint (including zero endpoints)',ylabel='amplitude at t≈2 s');ax[1,0].legend(fontsize=8)
ax[1,1].set(xlabel='Hz',ylabel='spectral level (dB)',ylim=(-110,0));ax[1,1].legend(fontsize=8)
fig.suptitle('DDSS equation and angular mapping • 220 Hz, D=1, dt=0.001, seed 1729\nCommon Gaussian increments; zero x modulation; no universal spectral ordering',fontsize=12)
fig.tight_layout(rect=(0,0,1,.92));fig.savefig(a.output,dpi=135);plt.close(fig)
print('GENDY_DIFFUSION_FIGURE_OK')
