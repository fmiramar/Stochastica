#!/usr/bin/env python3
"""Plot only project-generated GENDY data; requires optional numpy/matplotlib."""
import argparse,csv
from collections import defaultdict
from pathlib import Path
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
p=argparse.ArgumentParser();p.add_argument('csv',type=Path);p.add_argument('--output',type=Path,default=Path('HelpSource/Classes'));a=p.parse_args()
audio=defaultdict(list);points=defaultdict(list);fields=[]
with a.csv.open() as f:
    for r in csv.DictReader(f):
        if r['kind']=='audio':audio[int(r['variant'])].append(float(r['amp']))
        elif r['kind']=='point':points[int(r['variant'])].append([float(r[k]) for k in ['time','index','amp','dur','ampPrimary','durPrimary']])
        else:fields.append([float(r['time']),int(r['index']),float(r['amp'])])
audio={k:np.asarray(v) for k,v in audio.items()};points={k:np.asarray(v).reshape(-1,12,6) for k,v in points.items()}
plt.rcParams.update({'font.size':10,'axes.spines.top':False,'axes.spines.right':False,'figure.facecolor':'white','axes.grid':True,'grid.alpha':.18})
colors=['#196d8a','#c55b35','#587a39','#8d538a'];a.output.mkdir(parents=True,exist_ok=True)
def save(fig,name,title):
    fig.suptitle(title,fontsize=13);fig.tight_layout(rect=(0,0,1,.94));fig.savefig(a.output/('gendy-'+name+'.png'),dpi=135);plt.close(fig)
fig,ax=plt.subplots(2,1,figsize=(9,5))
x=points[0]
for j in range(4):ax[0].plot(x[:,j,0],x[:,j,2],lw=.8,label=f'point {j}')
ax[0].set(xlabel='seconds',ylabel='amplitude');ax[0].legend(ncol=4)
for cyc in [5,30,80]:ax[1].plot(range(12),x[cyc,:,2],marker='.',label=f'cycle {cyc+1}')
ax[1].set(xlabel='remembered point index',ylabel='amplitude');ax[1].legend(ncol=3)
save(fig,'gendy1977','First-order breakpoint memory • seed 1729, 12 points')
fig,ax=plt.subplots(2,1,figsize=(9,5))
for variant,label,col in [(0,'first order',colors[0]),(1,'second order',colors[1])]:
    x=points[variant];ax[0].plot(x[:,0,0],x[:,0,2],lw=.8,label=label,color=col)
x=points[1];ax[1].plot(x[:,0,0],x[:,0,4],color=colors[1],label='primary amplitude position')
ax[0].set(ylabel='secondary amplitude');ax[0].legend();ax[1].set(xlabel='seconds',ylabel='retained increment');ax[1].legend()
save(fig,'gendy1991','First-order position versus second-order tendency • same seed')
fig,ax=plt.subplots(2,1,figsize=(9,5));t=np.arange(96000)/48000
ax[0].plot(t,.035*2**(2*np.sin(2*np.pi*2*t)),label='amplitude step');ax[0].plot(t,.015*2**(2*np.sin(2*np.pi*2*t)),label='duration step');ax[0].legend();ax[0].set(ylabel='effective step fraction')
ax[1].plot(t[::8],audio[2][::8],lw=.5);ax[1].set(xlabel='seconds',ylabel='raw waveform')
save(fig,'gendys709','Internal rule modulation • 2 Hz sine, depth 2 octaves')
fig,ax=plt.subplots(2,1,figsize=(9,5));ax[0].plot(t[::8],audio[3][::8],lw=.5);ax[0].set(ylabel='raw waveform')
x=points[3];motion=np.abs(np.diff(x[:,:,2],axis=0)).mean(axis=1);ax[1].plot(x[1:,0,0],motion,label='mean amplitude change per cycle');ax[1].axvline(.6,color=colors[1],ls='--',label='mobility reaches zero');ax[1].legend();ax[1].set(xlabel='seconds',ylabel='breakpoint motion')
save(fig,'gendyperc','Timbre stabilization without a volume envelope • 0.6-second collapse')
fig,ax=plt.subplots(2,1,figsize=(9,5));start=48000;window=600
for v,label,col in zip(range(4,8),['hold','linear','cosine','Hermite'],colors):
    ax[0].plot(np.arange(window)/48,audio[v][start:start+window],lw=1,label=label,color=col)
    y=audio[v][48000:]*np.hanning(48000);spec=np.abs(np.fft.rfft(y));freq=np.fft.rfftfreq(len(y),1/48000)
    # Broad logarithmic averages reduce line-bin clutter without normalizing each curve separately.
    edges=np.geomspace(50,24000,100);freqs=[];level=[]
    for lo,hi in zip(edges[:-1],edges[1:]):
        pick=(freq>=lo)&(freq<hi);freqs.append(np.sqrt(lo*hi));level.append(20*np.log10(np.sqrt(np.mean(spec[pick]**2))/24000+1e-12))
    ax[1].semilogx(freqs,level,label=label,color=col)
ax[0].set(xlabel='milliseconds after t=1 s',ylabel='waveform');ax[0].legend(ncol=4)
ax[1].set(xlabel='Hz',ylabel='spectral level (dB)',ylim=(-110,0));ax[1].legend(ncol=4)
save(fig,'gendyinterp','One breakpoint history, four renderers • waveform and spectrum')
fig,ax=plt.subplots(1,2,figsize=(9,4));correlations=[]
for v,k,col in zip([8,9,10],[0,.125,.25],colors):
    x=points[v];aa=x[20:,:,2];rough=np.mean((aa-np.roll(aa,1,axis=1))**2,axis=1)
    ax[0].plot(x[20:,0,0],rough,label=f'k={k}',color=col,lw=.8)
    correlations.append(np.corrcoef(aa.ravel(),np.roll(aa,1,axis=1).ravel())[0,1])
ax[0].set(xlabel='seconds',ylabel='mean squared neighbor difference',yscale='log');ax[0].legend()
ax[1].bar(['0','0.125','0.25'],correlations,color=colors[:3]);ax[1].set(xlabel='amplitude coupling',ylabel='neighbor correlation',ylim=(-.3,1))
save(fig,'gendycoupled','Synchronous ring interaction • equal innovations, seed 1729')
fig,ax=plt.subplots(figsize=(9,3.5));r=np.asarray(fields).reshape(8,5000,3)[:,:,2]
ax.imshow(r,origin='lower',aspect='auto',interpolation='nearest',extent=(0,10,-.5,7.5),cmap='Blues',vmin=0,vmax=1);ax.set(xlabel='seconds',ylabel='voice',yticks=range(8));ax.grid(False)
save(fig,'gendyfield','Independent activity fields • mean 0.3 s, sound probability 0.55')
print('GENDY_FIGURES_OK: 7 figures')
