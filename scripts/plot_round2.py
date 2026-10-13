#!/usr/bin/env python3
"""Render Round 2 figures from export_round2_analysis; no runtime dependencies.

c++ -std=c++17 -O2 -Iinclude scripts/export_round2_analysis.cpp -o build-round2/export
build-round2/export > build-round2/analysis.csv
python3 scripts/plot_round2.py build-round2/analysis.csv
"""
import argparse
import csv
import math
from collections import defaultdict
from pathlib import Path
from plot_chua_state_space import Raster, TRACE_COLORS, GRID, fitter
from plot_deterministic_dynamics import draw_frame, finish, dot
from plot_stochastic_dynamics import polyline

COLORS=TRACE_COLORS+((190,100,70),)

def panels(path, groups, bounds=None):
    raster=Raster(2400,800);width=2400//len(groups)
    for index,curves in enumerate(groups):
        rect=(index*width+12,12,(index+1)*width-12,788)
        draw_frame(raster,rect)
        points=[p for curve in curves for p in curve]
        transform=fitter(bounds[index] if bounds else points,rect)
        for curve,color in zip(curves,COLORS*100):
            polyline(raster,curve,transform,color,2.2,.85)
    finish(path,raster)

def circle_panels(path,groups,hist=False):
    width=800; raster=Raster(width*len(groups),800)
    for index,angles in enumerate(groups):
        cx=index*width+width/2;cy=400;radius=330
        def point(angle,r=1):return (cx+radius*r*math.cos(angle),cy-radius*r*math.sin(angle))
        circle=[point(i*math.tau/400) for i in range(401)]
        for a,b in zip(circle,circle[1:]):raster.line(a,b,GRID,2)
        for angle in (0,math.pi/2,math.pi,math.pi*1.5):raster.line((cx,cy),point(angle),GRID,1)
        if hist:
            counts=[0]*48
            for a in angles:counts[int((a+math.pi)/math.tau*48)%48]+=1
            for i,count in enumerate(counts):
                angle=(i+.5)*math.tau/48-math.pi
                raster.line((cx,cy),point(angle,count/max(counts)),COLORS[index],width=7,alpha=.85)
        else:
            for a in angles:
                p=point(a)
                for dx in range(-4,5):
                    for dy in range(-4,5):dot(raster,(p[0]+dx,p[1]+dy),COLORS[index],.8)
            c=sum(map(math.cos,angles))/len(angles);s=sum(map(math.sin,angles))/len(angles)
            raster.arrow((cx,cy),(cx+radius*c,cy-radius*s),COLORS[2])
    finish(path,raster)

def main():
    parser=argparse.ArgumentParser();parser.add_argument('csv');parser.add_argument('--output',type=Path,default=Path('HelpSource/Classes'));args=parser.parse_args()
    data=defaultdict(list)
    with open(args.csv) as f:
        for row in csv.reader(f):data[row[0]].append(list(map(float,row[1:])))
    out=args.output;out.mkdir(parents=True,exist_ok=True)
    groups=[]
    for mode in range(2):
        rows=[r for r in data['correct'] if r[0]==mode]
        trace=[(r[1],r[3]) for r in rows]
        ticks=[p for r in rows if r[2] for p in ((r[1],-1),(r[1],-3),(r[1],-1))]
        # Separate tick strokes to avoid connecting events into a baseline.
        groups.append([trace]+[ticks[i:i+3] for i in range(0,len(ticks),3)])
    peak=max(r[3] for r in data['correct'])
    panels(out/'self-correcting-dynamics.png',groups,[[(0,-4),(12,peak*1.05)]]*2)
    panels(out/'random-telegraph-dynamics.png',[[[(r[0],r[1]) for r in data['tele']]]])
    rows=data['shot'];dt=rows[1][0]-rows[0][0];individual=[]
    for i,r in enumerate(rows):
        if r[2]:
            amplitude=r[1]-(rows[i-1][1] if i else 0)*math.exp(-dt/.25)
            individual.append([(r[0]+j*dt,amplitude*math.exp(-j*dt/.25)) for j in range(min(600,len(rows)-i))])
    ticks=[[(r[0],0),(r[0],1)] for r in rows if r[2]]
    panels(out/'shot-noise-dynamics.png',[ticks,individual,[[(r[0],r[1]) for r in rows]]], [[(0,0),(5,1)],[(0,0),(5,1)],[(0,0),(5,max(r[1] for r in rows))]])
    rows=data['ou'];panels(out/'multi-ou-dynamics.png',[[[(r[1],r[2]) for r in rows]],[[(r[0],r[k]) for r in rows] for k in (1,2)]])
    potential=[(i/100,((i/100)**4)/4-((i/100)**2)/2) for i in range(-180,181)]
    panels(out/'double-well-dynamics.png',[[potential],[[(r[0],r[1]) for r in data['well']]]])
    circle_panels(out/'von-mises-circular.png',[[r[1] for r in data['von'] if r[0]==k] for k in (0,2,8)],True)
    circle_panels(out/'kuramoto-coherence.png',[[r[1] for r in data['kur'] if r[0]==mode] for mode in (0,1)])
    rows=data['hawkes'];rasters=[]
    for j in range(4):
        rasters.extend([[(r[0],3-j-.18),(r[0],3-j+.18)] for r in rows if r[j+1]])
    panels(out/'multi-hawkes-dynamics.png',[rasters,[[(r[0],r[k]) for r in rows] for k in (6,7)]],[[(0,-.5),(20,3.5)],[(0,0),(20,max(r[k] for r in rows for k in (6,7)))]] )
    print('ROUND2_FIGURES_OK: 8 numerical illustrations')
if __name__=='__main__':main()
