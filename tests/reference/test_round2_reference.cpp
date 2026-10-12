#include "SDEventProcesses.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>

void require(bool ok, const char* message)
{
    if (!ok) throw std::runtime_error(message);
}
void near(double a, double b, double tolerance, const char* message)
{
    require(std::isfinite(a) && std::abs(a-b) <= tolerance, message);
}

int main()
{
    try {
        near(sd::eventProbability(1e-15, 1e-5), 1e-20, 1e-34, "small hazard precision");
        near(sd::eventProbability(1e30, 1.0), 1.0, 0.0, "large hazard");
        sd::PCG32 rng; rng.setSeed(123456);
        double z = 0;
        auto e = sd::selfCorrectingStep(rng, z, 0, 1, 100, 0.1);
        require(!e.event && e.intensity == 0, "zero self-correcting rate");
        e = sd::selfCorrectingStep(rng, z, 2, 0, 100, 0.1);
        near(e.intensity, 2, 0, "Poisson limit intensity");
        e = sd::selfCorrectingStep(rng, z, 1e30, 1e30, 20, 1);
        require(e.intensity <= 20 && std::isfinite(z), "explicit log ceiling");

        sd::TelegraphState telegraph;
        telegraph.reset(rng, false);
        telegraph.hazard = 0.25;
        require(telegraph.step(rng, 1, 0, 0.5) == 1 && telegraph.high, "fractional telegraph switch");
        require(telegraph.step(rng, 1, 0, 100) == 0 && telegraph.high, "absorbing high state");
        telegraph.reset(rng, false);
        require(telegraph.step(rng, 0, 1, 100) == 0 && !telegraph.high, "absorbing low state");
        require(telegraph.step(rng, 1e30, 1e30, 1) == 8 && telegraph.debt > 0,
                "bounded transitions carry time");

        sd::ShotState shot;
        shot.reset(rng);
        shot.hazard = 0.25;
        require(shot.step(rng, 1, 1, 2, 0, 0.25) == 1, "shot at interval boundary");
        near(shot.value, 2, 0, "fixed mark");
        shot.step(rng, 0, 1, 2, 0, 0.5);
        near(shot.value, 2*std::exp(-0.5), 1e-14, "analytical shot decay");
        shot.hazard = 0.01;
        require(shot.step(rng, 1, 0, 3, 0, 0.01) == 1, "instantaneous shot");
        near(shot.value, 3, 0, "instantaneous mark value");
        shot.step(rng, 0, 0, 3, 0, 0.1);
        near(shot.value, 0, 0, "instantaneous shot clears next calculation");
        require(shot.step(rng, 1e30, 1, 1, 0, 1) == 8 && shot.debt > 0,
                "bounded shots carry time");

        sd::RisingEdge edge; edge.initialize(0);
        require(edge.update(1) && !edge.update(1) && !edge.update(0) && edge.update(2), "edge rule");
        std::cout << "Round 2 reference tests passed\n";
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
