#!/usr/bin/env python3
"""Independent scalar Python oracle for the small stored GENDY trace.

Only the specified PCG integer recurrence is shared mathematically. Reflection
uses iterative mirrors here (bounded offline inputs), not the C++ fold helper.
Run from the repository root to regenerate tests/reference/gendy-trace.csv.
"""
import csv
from pathlib import Path
class PCG:
    def __init__(self,seed):
        self.state=0;self.next();self.state=(self.state+seed)&((1<<64)-1);self.next()
    def next(self):
        old=self.state;self.state=(old*6364136223846793005+0xda3e39cb94b95bdb)&((1<<64)-1)
        x=(((old>>18)^old)>>27)&0xffffffff;rot=old>>59
        return ((x>>rot)|(x<<((-rot)&31)))&0xffffffff
    def signed(self):return 2*(self.next()+.5)/4294967296-1

def mirror(x,lo,hi):
    if lo==hi:return lo
    while x<lo or x>hi:
        x=2*hi-x if x>hi else 2*lo-x
    return x

def rows():
    for model in range(3):
        rng=PCG(123);amp=[0.]*3;dur=[88.]*3;ap=[0.]*3;dp=[0.]*3
        for cycle in range(1,17):
            qa=[];qd=[]
            for j in range(3):
                da,dd=rng.signed(),rng.signed()
                if model==1:
                    ap[j]=mirror(ap[j]+da*.04,-.2,.2)
                    dp[j]=mirror(dp[j]+dd*14.4,-72,72)
                    qa.append(mirror(amp[j]+ap[j],-1,1));qd.append(mirror(dur[j]+dp[j],16,160))
                else:qa.append(amp[j]+da*.2);qd.append(dur[j]+dd*14.4)
            for j in range(3):
                a=qa[j];d=qd[j]
                if model==2:
                    a+=.25*(qa[(j-1)%3]-2*qa[j]+qa[(j+1)%3])
                    d+=.125*(qd[(j-1)%3]-2*qd[j]+qd[(j+1)%3])
                amp[j]=mirror(a,-1,1);dur[j]=mirror(d,16,160)
                yield [model,cycle,j,amp[j],dur[j],ap[j],dp[j]]
if __name__=='__main__':
    p=Path(__file__).resolve().parents[1]/'tests/reference/gendy-trace.csv'
    with p.open('w',newline='') as f:
        w=csv.writer(f);w.writerow(['model','cycle','index','amplitude','duration','amplitude_primary','duration_primary']);w.writerows(rows())
    print(p.name)
