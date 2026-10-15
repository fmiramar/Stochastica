#!/usr/bin/env python3
"""Independent scalar DDSS reconstruction fixture; no production code imported."""
import csv
import math
from pathlib import Path
from gendy_reference_oracle import PCG, mirror

def rows():
    for dimension in (1, 2, 3):
        rng=PCG(123);radii=[1.]*4
        for cycle in range(17):
            for j in range(4):
                if cycle:
                    u=(rng.next()+.5)/2**32;v=(rng.next()+.5)/2**32
                    z=math.sqrt(-2*math.log(u))*math.cos(2*math.pi*v)
                    r=radii[j]
                    radii[j]=abs(r+.001*(dimension-1)/max(r,1e-6)+math.sqrt(.002)*z)
                q=mirror(radii[j],-1,1);angle=2*math.pi*j/4
                amp=q*(math.sin(angle)+.1*math.cos(angle)*math.sin(2*math.pi*.2*cycle/220))/math.sqrt(1.01)
                yield dimension,cycle,j,radii[j],q,amp

if __name__=='__main__':
    path=Path(__file__).resolve().parents[1]/'tests/reference/gendy-diffusion-trace.csv'
    with path.open('w',newline='') as f:
        w=csv.writer(f,lineterminator='\n');w.writerow(['dimensions','cycle','index','radius','reflected','amplitude']);w.writerows(rows())
    print(path.name)
