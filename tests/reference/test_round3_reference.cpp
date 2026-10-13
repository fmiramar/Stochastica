#include "SDAllocation.hpp"
#include "SDLatent.hpp"
#include "SDMatern.hpp"
#include "SDPopulation.hpp"
#include "SDTransport.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>

void require(bool value,const char* message) { if (!value) throw std::runtime_error(message); }
void near(double actual,double expected,double tolerance,const char* message)
{ require(std::isfinite(actual) && std::abs(actual-expected)<=tolerance,message); }
double q(const sd::MaternCoefficients& c,int i,int j)
{ double value=0; for (int k=0;k<c.n;++k) value+=c.l[i*3+k]*c.l[j*3+k]; return value; }
int main()
{
    try {
        sd::PCG32 rng; rng.setSeed(123);
        double alpha[]={.2,2,8},allocation[3];
        sd::concentrationMean(alpha,allocation,3);
        near(allocation[2],8/10.2,1e-15,"Dirichlet reset mean");
        for (int j=0;j<200;++j) {
            sd::dirichletSample(rng,alpha,allocation,3);
            near(allocation[0]+allocation[1]+allocation[2],1,1e-14,"simplex sum");
            for (double x:allocation) require(x>=0 && x<=1,"simplex support");
        }
        double tiny[]={1e-35,1e-35,1e-35};
        sd::dirichletSample(rng,tiny,allocation,3);
        near(*std::max_element(allocation,allocation+3),1,0,"underflow-safe corner allocation");
        double weights[]={1,1,1},unitScale=1;
        const auto draw=sd::urnDraw(rng,weights,3,2,unitScale);
        near(draw.probability,1.0/3,1e-15,"pre-reinforcement probability");
        near(weights[draw.state]/sd::weightTotal(weights,3),.6,1e-15,"reinforcement increases probability");
        near(draw.entropy,sd::normalizedEntropy(weights,3),0,"post-reinforcement entropy");
        double huge[]={1e15,2e15,3e15},plain[]={1e15,2e15,3e15};
        unitScale=1; sd::PCG32 reference; rng.setSeed(4); reference.setSeed(4);
        for (int j=0;j<200;++j) {
            const int chosen=sd::categoricalSample(reference,plain,3,0,true);
            const auto actual=sd::urnDraw(rng,huge,3,1e14,unitScale);
            require(actual.state==chosen,"urn rescale changes future choices");
            near(actual.probability,plain[chosen]/sd::weightTotal(plain,3),1e-14,"rescale probability");
            plain[chosen]+=1e14;
        }
        double zero[]={0,0,0};
        const auto before=rng.state;
        require(sd::categoricalSample(rng,zero,3,1)==1 && rng.state==before,"zero row holds without draw");

        double transitions[]={0,1,1,0},eventRates[]={0,1e30},stateRates[]={1e30,1e30};
        auto event=sd::modulatedEventStep(rng,0,transitions,eventRates,stateRates,2,1e30,1e30,.01);
        require(!event.event && event.state==1 && event.intensity==0 && event.changed,"old-state event semantics");
        event=sd::modulatedEventStep(rng,1,transitions,eventRates,stateRates,2,9,0,.01);
        require(event.state==1 && !event.changed && event.intensity==9,"modulation ceilings");

        // Independent stationary-covariance identity and transition semigroup.
        for (int mode=0;mode<3;++mode) for (double dt:{1e-12,1e-6,.01,.3,1.0,100.0}) {
            sd::MaternCoefficients a,b;
            require(sd::maternCoefficients(a,mode,2,.7,dt),"Matérn coefficients");
            require(sd::maternCoefficients(b,mode,2,.7,2*dt),"Matérn double step");
            double p[9]={2,0,0,0,2,0,0,0,0};
            if (mode==2) { p[2]=p[6]=-2.0/3; p[4]=2.0/3; p[8]=2; }
            for (int i=0;i<a.n;++i) for (int j=0;j<a.n;++j) {
                double apa=0,aa=0,aqa=0;
                for (int k=0;k<a.n;++k) {
                    aa+=a.a[i*3+k]*a.a[k*3+j];
                    for (int m=0;m<a.n;++m) {
                        apa+=a.a[i*3+k]*p[k*3+m]*a.a[j*3+m];
                        aqa+=a.a[i*3+k]*q(a,k,m)*a.a[j*3+m];
                    }
                }
                near(apa+q(a,i,j),p[i*3+j],2e-12,"Matérn stationary Lyapunov identity");
                near(aa,b.a[i*3+j],2e-12,"Matérn exact transition semigroup");
                near(aqa+q(a,i,j),q(b,i,j),3e-12,"Matérn noise semigroup");
            }
            const double u=a.lambda*dt;
            double autocov=0;
            for (int i=0;i<a.n;++i) autocov+=a.a[i]*p[i*3];
            const double expected=2*std::exp(-u)*(1+(mode>=1?u:0)+(mode==2?u*u/3:0));
            near(autocov,expected,1e-13,"Matérn covariance family");
        }
        sd::MaternCoefficients c;
        require(sd::maternCoefficients(c,2,1,1e38,1e-5),"Matérn tiny elapsed relative to scale");
        require(sd::maternCoefficients(c,2,1,1e-6,1e30),"Matérn huge elapsed relative to scale");
        require(sd::maternCoefficients(c,2,0,.5,.01),"zero variance coefficients");
        double state[3]={5,0,0}; const auto initialRng=rng.state;
        require(sd::maternStep(rng,state,1,c) && rng.state==initialRng,"deterministic Matérn consumes no RNG");
        require(state[0]<5 && state[0]>1,"zero variance relaxes");
        require(sd::maternCoefficients(c,1,3,.5,0),"zero time scale");
        const double held=state[0];sd::maternStep(rng,state,1,c);near(state[0],held,0,"zero scale holds");

        double transport=0,increment=0; rng.setSeed(16); reference.setSeed(16);
        require(sd::levyFlightStep(rng,transport,increment,2,3,2,.25),"Gaussian stable limit");
        near(increment,.5+3*std::sqrt(.5)*sd::normalSample(reference),1e-14,"stable alpha=2 normalization");
        transport=1e50;
        require(sd::levyFlightStep(rng,transport,increment,2,0,1e50,1),"large finite transport");
        near(transport,2e50,0,"no clipping of finite double state");
        sd::JumpResult jump{};
        transport=0;
        rng.setSeed(41);reference.setSeed(41);
        require(sd::jumpDiffusionStep(rng,transport,jump,.3,.7,0,9,8,.2),"zero-rate diffusion limit");
        near(transport,.3*.2+.7*std::sqrt(.2)*sd::normalSample(reference),1e-14,"zero-rate Brownian increment");
        require(jump.count==0 && jump.size==0,"zero-rate jump diagnostics");
        transport=0;
        for(int j=0;j<500;++j) {
            const double old=transport;
            require(sd::jumpDiffusionStep(rng,transport,jump,0,0,5,3,0,.2),"fixed-size jumps");
            near(jump.size,3*double(jump.count),0,"aggregated fixed marks");
            near(transport-old,jump.size,0,"jump displacement");
        }
        require(!sd::jumpDiffusionStep(rng,transport,jump,0,1,1e30,1,1,1),"unsupported Poisson mean rejected");
        require(jump.count==0 && jump.size==0,"recovery has no spurious jump");

        near(sd::logistic(1000),1,0,"stable positive logistic");
        near(sd::logistic(-1000),0,0,"stable negative logistic");
        near(sd::logistic(0),.5,0,"logistic midpoint");
        sd::PopulationParameters parameters={0,0,0,0,0,0,.02,.01,1,0,1,0};
        std::array<double,2> population={.1,.8};
        const auto derivative=sd::wilsonCowanDerivative(population,parameters);
        near(derivative[0],20,1e-14,"uncoupled E derivative");near(derivative[1],-30,1e-14,"uncoupled I derivative");
        sd::wilsonCowanStep(population,parameters,16,.05);
        near(population[0],.5-.4*std::exp(-2.5),1e-6,"RK4 uncoupled analytic E");
        near(population[1],.5+.3*std::exp(-5),2e-6,"RK4 uncoupled analytic I");
        double error[3];
        for (int j=0;j<3;++j) {
            std::array<double,2> x={.1,.8};sd::wilsonCowanStep(x,parameters,1<<j,.005);
            error[j]=std::abs(x[1]-(.5+.3*std::exp(-.5)));
        }
        require(error[2]<error[1]/10 && error[1]<error[0]/10,"RK4 fourth-order convergence");
        parameters={0,0,10,12,10,2,.02,.01,1,0,1,0};
        std::array<double,2> ar={.1,.1},kr=ar;
        for(int i=0;i<4800;++i)sd::wilsonCowanStep(ar,parameters,1,1.0/48000);
        for(int i=0;i<75;++i)sd::wilsonCowanStep(kr,parameters,16,1.0/750);
        near(ar[0],kr[0],1e-9,"Wilson ar/kr elapsed time");near(ar[1],kr[1],1e-9,"Wilson ar/kr inhibition");

        for(int n:{4,8,64}) {
            std::vector<double>x(n),out(n),rotated(n),rotOut(n),scratch(5*n),otherScratch(5*n);
            for(int i=0;i<n;++i)x[i]=std::sin(i*.3)+.1*i;
            for(int i=0;i<n;++i)rotated[(i+1)%n]=x[i];
            sd::lorenz96Derivative(x.data(),out.data(),n,8);
            for(int i=0;i<n;++i) {
                const auto cyclic=[&](int k){return x[(k%n+n)%n];};
                near(out[i],(cyclic(i+1)-cyclic(i-2))*cyclic(i-1)-cyclic(i)+8,0,"Lorenz cyclic stencil");
            }
            sd::lorenz96Derivative(rotated.data(),rotOut.data(),n,8);
            for(int i=0;i<n;++i)near(out[i],rotOut[(i+1)%n],0,"Lorenz derivative rotation");
            for(int t=0;t<100;++t) {
                require(sd::lorenz96Step(x.data(),scratch.data(),n,8,4,.01),"Lorenz finite trajectory");
                sd::lorenz96Step(rotated.data(),otherScratch.data(),n,8,4,.01);
            }
            for(int i=0;i<n;++i)near(x[i],rotated[(i+1)%n],1e-12,"Lorenz trajectory rotation");
            std::fill(x.begin(),x.end(),1);
            sd::lorenz96Step(x.data(),scratch.data(),n,1,16,.1);
            for(double value:x)near(value,1,0,"Lorenz uniform equilibrium");
        }
        // Convergence against a much finer integration, before chaotic separation.
        std::array<double,8> start={1,2,3,4,5,6,7,8}, fine=start;
        double work[40];
        for(int j=0;j<256;++j)sd::lorenz96Step(fine.data(),work,8,8,1,.02/256);
        for(int j=0;j<3;++j) {
            auto x=start;sd::lorenz96Step(x.data(),work,8,8,1<<j,.02);
            error[j]=0;
            for(int k=0;k<8;++k)error[j]+=std::abs(x[k]-fine[k]);
        }
        require(error[1]<error[0]/10 && error[2]<error[1]/10,"Lorenz RK4 convergence");
        auto audio=start,control=start;
        for(int j=0;j<4800;++j)sd::lorenz96Step(audio.data(),work,8,8,1,1.0/48000);
        for(int j=0;j<75;++j)sd::lorenz96Step(control.data(),work,8,8,4,1.0/750);
        for(int k=0;k<8;++k)near(audio[k],control[k],2e-9,"Lorenz ar/kr elapsed time");
        std::cout<<"Round 3 reference tests passed\n";
    } catch(const std::exception& e) { std::cerr<<e.what()<<'\n';return 1; }
}
