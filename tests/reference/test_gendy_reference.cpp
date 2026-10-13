#include "SDGendy.hpp"
#include <array>
#include <iostream>
#include <stdexcept>
using namespace sd::gendy;
void check(bool ok,const char* what) { if(!ok) throw std::runtime_error(what); }
void near(double a,double b,double tol=1e-12) { if (!(std::abs(a-b)<tol)) {std::cerr << "actual=" << a << " expected=" << b << " tolerance=" << tol << "\n"; throw std::runtime_error("reference mismatch");} }
int main()
{
    try {
        // Independent piecewise reflection expectations, including repeated overshoot.
        for(auto pair: {std::pair<double,double>{1.25,.75},{-1.25,-.75},{5.25,.75},{-5.25,-.75},{101.5,.5}})
            near(reflect(pair.first,-1,1),pair.second);
        near(reflect(1e300,2,2),2);
        const double huge=std::numeric_limits<double>::max();
        for(double x:{-huge,huge,0.0}) for(auto b:{Bounds{-1,1},Bounds{-huge,huge},Bounds{huge/2,huge}}) {
            const double y=reflect(x,b.lo,b.hi);check(std::isfinite(y)&&y>=b.lo&&y<=b.hi,"large reflection");
        }
        // Golden first/second-order traces derived by hand, no RNG involved.
        double a=0, b=0, primary=0;
        const std::array<double,5> deltas={1,1,-1,-1,1};
        const std::array<double,5> first={.4,.8,.4,0,.4};
        const std::array<double,5> second={.2,.6,.8,.8,1};
        for(size_t i=0;i<deltas.size();++i) {
            a=firstOrder(a,deltas[i],.2,{-1,1});
            b=secondOrder(b,primary,deltas[i],.25,.2,{-1,1});
            near(a,first[i]);near(b,second[i]);
        }
        const auto bounds=durationBounds(48000,12,100,1000);
        near(bounds.lo,4);near(bounds.hi,40);
        near(durationBounds(48000,64,1e30,-10).lo,1);
        for(int mode=1;mode<4;++mode) {
            near(interpolate(mode,-.2,.8,0),-.2);near(interpolate(mode,-.2,.8,1),.8);
            for(int j=0;j<100;++j){const auto x=interpolate(mode,-.2,.8,j/100.);check(x>=-.2&&x<=.8,"interpolation bound");}
        }
        near(interpolate(0,-.2,.8,1),-.2);near(interpolate(2,0,1,.5),.5);
        near(interpolate(3,0,1,.25),.15625);
        near(mobility(1,.01,.5,1),.1);near(mobility(1,0,.5,1),.5);
        near(mobility(.4,.4,10,1),.4);near(mobility(1,.1,0,0),.1);
        for(int dist=0;dist<6;++dist) for(double shape:{.0001,.2,1.0}) {
            double old=-2;
            for(int j=0;j<=10000;++j) {
                double x=innovation(dist,shape,j/10000.);
                check(std::isfinite(x)&&x>=-1&&x<=1&&x>=old,"quantile range/monotonicity"); old=x;
            }
            if(dist!=5) { near(innovation(dist,shape,.5),0);near(innovation(dist,shape,.25),-innovation(dist,shape,.75)); }
        }
        near(innovation(0,1,.25),-.5);near(innovation(1,.1,.75),std::sqrt(2.)-1);
        near(innovation(4,1,.75),std::sqrt(.5));
        // A cycle's new first point connects to the actual old last endpoint.
        std::array<Point,4> memory{};Engine e;Parameters p;e.points=memory.data();e.n=4;e.fs=16;
        p.minFreq=p.maxFreq=2;p.ampStep=.25;e.reset(p);
        for(int seg=0;seg<20;++seg){e.beginSegment(p); const double end=e.right;
            near(interpolate(1,e.left,e.right,1),end);
            e.beginSegment(p);near(e.left,end);
        }
        // Synchronous ring operation independently computed from signed innovations.
        Engine c;c.points=memory.data();c.n=4;c.fs=48000;c.seed=73;p=Parameters{};p.ampCoupling=.25;c.reset(p);
        sd::PCG32 r;r.setSeed(73);double v[4];for(auto& x:v){x=2*p.ampStep*(2*sd::openUniformDouble(r)-1);sd::openUniformDouble(r);}
        c.updateCycle(p);for(int i=0;i<4;++i)near(memory[i].amp,.5*v[i]+.25*v[(i+3)%4]+.25*v[(i+1)%4]);
        // Reset reproducibility; switching interpolation never changes point evolution.
        std::array<Point,12> m1{},m2{};Engine e1,e2;e1.points=m1.data();e2.points=m2.data();p=Parameters{};
        e1.reset(p);e2.reset(p);
        for(int i=0;i<10000;++i){p.interp=1;e1.sample(p);p.interp=i%4;e2.sample(p);
            for(int k=0;k<12;++k){near(m1[k].amp,m2[k].amp);near(m1[k].dur,m2[k].dur);}}
        e1.reset(p);e2.reset(p);for(int i=0;i<10000;++i)near(e1.sample(p),e2.sample(p));
        // Changing modes mid-segment applies only at the next segment.
        p.interp=1;e1.reset(p);e1.sample(p);p.interp=0;e1.sample(p);check(e1.mode==1,"midsegment mode switch");
        while(e1.index==0)e1.sample(p);check(e1.mode==0,"boundary mode switch");
        std::cout<<"GENDY reference checks passed\n";
    } catch(const std::exception& ex) {std::cerr<<ex.what()<<'\n';return 1;}
}
