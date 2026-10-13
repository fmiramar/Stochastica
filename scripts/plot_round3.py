#!/usr/bin/env python3
"""Render all eight Round 3 figures from export_round3_analysis.cpp output.

Requires NumPy and Matplotlib; no network, audio server, or manual editing.
"""
import argparse
from collections import defaultdict
from pathlib import Path
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

COLORS = ['#176b87', '#c15326', '#7756a0']

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('data', type=Path)
    parser.add_argument('--output', type=Path, default=Path(__file__).resolve().parents[1]/'HelpSource/Classes')
    args = parser.parse_args()
    rows = defaultdict(list)
    with args.data.open() as source:
        for line in source:
            name, *values = line.strip().split(',')
            rows[name].append([float(x) for x in values])
    data = {key: np.array(values) for key, values in rows.items()}
    args.output.mkdir(parents=True, exist_ok=True)
    plt.rcParams.update({'font.size':10, 'axes.spines.top':False, 'axes.spines.right':False,
                         'axes.prop_cycle':plt.cycler(color=COLORS), 'savefig.facecolor':'white'})

    def save(fig, name, caption):
        fig.text(.02, .015, caption, fontsize=9, color='#46535c')
        fig.tight_layout(rect=(0,.045,1,.95))
        fig.savefig(args.output/f'round3-{name}.png', dpi=150)
        plt.close(fig)

    fig, axes = plt.subplots(2, 1, figsize=(10,6.5))
    fig.suptitle('Matérn: equal variance and correlationTime, different local smoothness')
    a = data['matern']
    for mode in range(3):
        b = a[a[:,0]==mode]; t, x = b[:,1], b[:,2]
        axes[0].plot(t[t<5], x[t<5], lw=1, label=f'smoothness={mode} (ν={mode+.5})')
        centered = x-x.mean()
        transform = np.fft.rfft(centered, n=2*len(x))
        cov = np.fft.irfft(transform*transform.conj())[:501]/np.arange(len(x),len(x)-501,-1)
        lag = np.arange(501)*.002; u = np.sqrt(2*mode+1)*lag/.25
        theory = np.exp(-u)*(1+(u if mode>=1 else 0)+(u*u/3 if mode==2 else 0))
        axes[1].plot(lag, cov/cov[0], color=COLORS[mode], label=f'mode {mode}: empirical')
        axes[1].plot(lag, theory, '--', color=COLORS[mode], alpha=.75)
    axes[0].set(xlabel='time (s)', ylabel='raw state', xlim=(0,5)); axes[0].legend(ncol=3, fontsize=9)
    axes[1].set(xlabel='lag (s)', ylabel='autocorrelation', xlim=(0,1)); axes[1].legend(ncol=3, fontsize=9)
    save(fig,'matern','mean=0, variance=1, correlationTime=0.25 s; dt=0.002 s; seed=1729; 20 s burn-in. Dashed: analytic covariance.')

    fig, axes = plt.subplots(2,2,figsize=(9,7))
    fig.suptitle('Dirichlet allocations: one total, four concentration patterns')
    labels = ['alpha=[1, 1, 1]','alpha=[0.2, 0.2, 0.2]','alpha=[8, 8, 8]','alpha=[6, 1, 1]']
    for mode, ax in enumerate(axes.flat):
        b=data['dirichlet']; w=b[b[:,0]==mode,1:]
        ax.scatter(w[:,1]+w[:,2]/2, w[:,2]*np.sqrt(3)/2,s=3,alpha=.23,color=COLORS[0],rasterized=True)
        ax.plot([0,1,.5,0],[0,0,np.sqrt(3)/2,0],color='#33444f',lw=1)
        for x,y,label in [(0,-.035,'w₀=1'),(1,-.035,'w₁=1'),(.5,.91,'w₂=1')]:
            ax.text(x,y,label,ha='center',va='center',fontsize=9)
        ax.set_title(labels[mode],pad=12); ax.set_aspect('equal'); ax.set_axis_off()
    save(fig,'dirichlet','4,000 draws per panel; seed=401; w₀+w₁+w₂=1. Low concentration favors corners; high concentration favors balance.')

    fig, axes=plt.subplots(2,3,figsize=(11,6),sharex=True)
    fig.suptitle('Pólya reinforcement: equal starts, different histories')
    for run in range(3):
        a=data['urn']; b=a[a[:,0]==run]
        for k in range(3):axes[0,run].plot(b[:,1],b[:,2+k],lw=1.2,label=f'state {k}')
        axes[0,run].set(title=f'seed={17+run*31}',ylim=(0,1));axes[0,run].legend(fontsize=8)
        axes[1,run].plot(b[:,1],b[:,5],color='#384c59');axes[1,run].set(xlabel='trigger number',ylim=(0,1.03))
    axes[0,0].set_ylabel('selection probability');axes[1,0].set_ylabel('normalized entropy')
    save(fig,'urn','Initial weights=[1,1,1]; reinforcement=1. Entropy measures current diversity and need not fall monotonically.')

    fig,axes=plt.subplots(3,1,figsize=(10,6),sharex=True,gridspec_kw={'height_ratios':[1,1,0.7]})
    fig.suptitle('Markov modulation: a hidden regime organizes observed events')
    b=data['mmpp'];t=b[:,0]
    axes[0].step(t,b[:,1],where='post');axes[0].set(ylabel='pre-event state',yticks=[0,1,2])
    axes[1].step(t,b[:,2],where='post',color=COLORS[1]);axes[1].set(ylabel='intensity (s⁻¹)')
    axes[2].eventplot(t[b[:,3]>0],colors=COLORS[0],linewidths=.8);axes[2].set(xlabel='time (s)',ylabel='events',yticks=[],xlim=(0,20))
    save(fig,'mmpp','eventRateBuf=[1,8,28], stateRateBuf=[0.7,0.8,1.2]; dt=0.001 s; seed=81. Intensity uses the state before transition.')

    fig,axes=plt.subplots(3,1,figsize=(10,8))
    fig.suptitle('Lévy transport: increments accumulate, rare relocations remain visible')
    for mode in range(2):
        a=data['levy'];b=a[a[:,0]==mode];label=f'alpha={2 if mode==0 else 1.2}'
        axes[0].plot(b[:,1],b[:,2],lw=.8,label=label)
        axes[1].plot(b[:,1],b[:,3],lw=.6,label=label,alpha=.75)
        magnitudes=np.sort(np.abs(b[:,3]));ccdf=(len(magnitudes)-np.arange(len(magnitudes)))/len(magnitudes)
        axes[2].loglog(magnitudes,ccdf,lw=1.5,label=label)
    axes[0].set(xlabel='time (s)',ylabel='raw position');axes[0].legend()
    axes[1].set(xlabel='time (s)',ylabel='raw increment');axes[1].legend()
    axes[2].set(xlabel='|increment| (log scale)',ylabel='P(|increment| ≥ x)');axes[2].legend()
    save(fig,'levy','scale=1, drift=0; dt=0.001 s; seed=57; all 50,000 increments shown without clipping. alpha=2 is Gaussian with variance 2dt.')

    fig,axes=plt.subplots(2,1,figsize=(10,6),sharex=True,gridspec_kw={'height_ratios':[2,1]})
    fig.suptitle('Jump diffusion: continuous wandering plus distinct compound-Poisson shocks')
    b=data['jump'];mask=b[:,3]>0
    axes[0].plot(b[:,0],b[:,1],lw=1);axes[0].scatter(b[mask,0],b[mask,1],s=25,color=COLORS[1],label='jumpCount > 0',zorder=3)
    axes[0].set_ylabel('raw position');axes[0].legend()
    axes[1].vlines(b[mask,0],0,b[mask,2],color=COLORS[1]);axes[1].axhline(0,lw=.6,color='#555555')
    axes[1].set(xlabel='time (s)',ylabel='aggregate jumpSize',xlim=(0,24))
    save(fig,'jump','drift=0.03, diffusion=0.25, jumpRate=0.8 s⁻¹, jumpMean=0, jumpStd=1.5; dt=0.002 s; seed=72.')

    fig,axes=plt.subplots(2,2,figsize=(10,7))
    fig.suptitle('Wilson–Cowan: recurrent excitation changes the population regime')
    for mode in range(2):
        a=data['wilson'];b=a[a[:,0]==mode];shown=b[b[:,1]<=1]
        axes[0,mode].plot(shown[:,1],shown[:,2],label='exc E');axes[0,mode].plot(shown[:,1],shown[:,3],label='inh I')
        axes[0,mode].set(title=f'wEE={16 if mode else 0}',xlabel='time (s)',ylabel='population activity',ylim=(0,1));axes[0,mode].legend()
        axes[1,mode].plot(b[:,2],b[:,3],lw=.9,alpha=.8)
        axes[1,mode].scatter([b[0,2]],[b[0,3]],s=25,color=COLORS[1],label='start');axes[1,mode].legend()
        axes[1,mode].set(xlabel='exc E',ylabel='inh I',xlim=(0,1),ylim=(0,1))
    save(fig,'wilson','inputExc=3; wEI=12, wIE=15, wII=3; tauExc=0.02, tauInh=0.04; thresholds=4,3; gains=1; dt=0.0001 s, substeps=4.')

    fig,ax=plt.subplots(figsize=(10,5))
    fig.suptitle('Lorenz–96: spatially extended deterministic dynamics on a cyclic lattice')
    b=data['lorenz'];im=ax.imshow(b[:,1:].T,origin='lower',aspect='auto',extent=(0,20,-.5,31.5),cmap='RdBu_r',interpolation='nearest')
    ax.set(xlabel='time (model seconds)',ylabel='cell index',yticks=[0,8,16,24,31])
    fig.colorbar(im,ax=ax,label='raw state xᵢ')
    save(fig,'lorenz','numCells=32, forcing=8; dt=0.005, substeps=4. Initial xᵢ=8, x₀=8.01; 10 model seconds discarded; no stochastic innovations.')

if __name__=='__main__':
    main()
