#include "SDEventProcesses.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>

void check(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
struct Moments {
    double sum = 0, square = 0; int count = 0;
    void add(double x) { sum += x; square += x*x; ++count; }
    double mean() const { return sum/count; }
    double variance() const { return square/count - mean()*mean(); }
};

int main()
{
    try {
        double fano[2], shortFraction[2];
        for (int mode = 0; mode < 2; ++mode) {
            sd::PCG32 rng; rng.setSeed(117);
            double z = 0; int total = 0, window = 0, previous = -100000, shortCount = 0;
            Moments windows;
            for (int i = 0; i < 1000000; ++i) {
                auto result = sd::selfCorrectingStep(rng, z, 8, mode ? 1.5 : 0, 10000, 0.001);
                if (result.event) { ++total; ++window; if (i-previous < 20) ++shortCount; previous = i; }
                if ((i+1)%1000 == 0) { windows.add(window); window = 0; }
            }
            check(std::abs(total/1000.0-8) < 0.3, "self-correcting / Poisson rate");
            fano[mode] = windows.variance()/windows.mean();
            shortFraction[mode] = double(shortCount)/total;
        }
        check(fano[0] > 0.8 && fano[0] < 1.2, "Poisson count dispersion");
        std::cout << "Fano " << fano[0] << "," << fano[1] << " short " << shortFraction[0] << "," << shortFraction[1] << std::endl;
        check(fano[1] < 0.3 && shortFraction[1] < shortFraction[0]*0.7, "inhibition suppresses clustering");
        std::cout << "Fano Poisson=" << fano[0] << " correcting=" << fano[1] << '\n';

        sd::PCG32 rng; rng.setSeed(4321);
        sd::TelegraphState telegraph; telegraph.reset(rng, false);
        int high = 0, last = 0; Moments lowDwell, highDwell;
        for (int i = 0; i < 1000000; ++i) {
            bool before = telegraph.high;
            telegraph.step(rng, 2, 1, 0.001);
            high += telegraph.high;
            if (before != telegraph.high) {
                (before ? highDwell : lowDwell).add((i-last)*0.001); last = i;
            }
        }
        check(std::abs(high/1000000.0 - 2.0/3.0) < 0.04, "telegraph asymmetric occupancy");
        check(std::abs(lowDwell.mean()-0.5)<0.06 && std::abs(highDwell.mean()-1)<0.1, "telegraph dwell means");
        std::cout << "Telegraph occupancy=" << high/1000000.0 << " dwells=" << lowDwell.mean() << ',' << highDwell.mean() << '\n';

        sd::ShotState shot; rng.setSeed(9876); shot.reset(rng); Moments shots;
        for (int i = 0; i < 1500000; ++i) {
            shot.step(rng, 20, 0.15, 0.7, 0.6, 0.002);
            if (i > 5000) shots.add(shot.value);
        }
        check(std::abs(shots.mean()-2.1)<0.09, "shot stationary mean");
        check(std::abs(shots.variance()-1.275)<0.12, "shot stationary variance");
        std::cout << "Shot mean=" << shots.mean() << " variance=" << shots.variance() << '\n';
        std::cout << "Round 2 statistical tests passed\n";
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
