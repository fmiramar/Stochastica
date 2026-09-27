#include "SDBuffer.hpp"
#include "SDDistributions.hpp"
#include "SDMap.hpp"
#include "SDRange.hpp"
#include "SDRandom.hpp"

#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <stdexcept>

namespace {

void require(bool condition, const char* message)
{
    if (!condition)
        throw std::runtime_error(message);
}

void requireNear(double actual, double expected, double tolerance, const char* message)
{
    if (std::abs(actual - expected) > tolerance)
        throw std::runtime_error(message);
}

} // namespace

int main()
{
    try {
        {
            sd::PCG32 first {};
            sd::PCG32 second {};
            first.setSeed(123456U);
            second.setSeed(123456U);
            constexpr uint32_t expected[8] {
                21440019U, 3517618283U, 3755164769U, 3884490675U,
                1178054185U, 2295289928U, 819783694U, 113434924U
            };
            for (uint32_t value : expected) {
                require(first.nextUInt() == value, "PCG32 stored reference mismatch");
                require(second.nextUInt() == value, "PCG32 reset sequence mismatch");
            }
        }

        {
            auto state = sd::henonStep(0.0, 0.0, 1.4, 0.3);
            requireNear(state.x, 1.0, 1.0e-14, "Henon state 1 x mismatch");
            requireNear(state.y, 0.0, 1.0e-14, "Henon state 1 y mismatch");
            state = sd::henonStep(state.x, state.y, 1.4, 0.3);
            requireNear(state.x, -0.4, 1.0e-14, "Henon state 2 x mismatch");
            requireNear(state.y, 0.3, 1.0e-14, "Henon state 2 y mismatch");
            state = sd::henonStep(state.x, state.y, 1.4, 0.3);
            requireNear(state.x, 1.076, 1.0e-14, "Henon state 3 x mismatch");
            requireNear(state.y, -0.12, 1.0e-14, "Henon state 3 y mismatch");
        }

        {
            const auto positive = sd::gbmanStep(1.2, 2.1);
            requireNear(positive.x, 0.1, 1.0e-14, "Gbman positive x mismatch");
            requireNear(positive.y, 1.2, 1.0e-14, "Gbman positive y mismatch");
            const auto negative = sd::gbmanStep(-1.2, 2.1);
            requireNear(negative.x, 0.1, 1.0e-14, "Gbman negative x mismatch");
            requireNear(negative.y, -1.2, 1.0e-14, "Gbman negative y mismatch");
        }

        {
            double x = 0.1;
            double y = 0.1;
            constexpr double expectedX[4] {
                0.4432803099920094, 0.9198456298447106,
                1.1845972288340156, -0.16899125293221545
            };
            constexpr double expectedY[4] {
                0.14975012497024223, 0.5035005633241293,
                1.0367554041236362, 1.356726039560002
            };
            for (int index = 0; index < 4; ++index) {
                const auto state = sd::latoocarfianStep(x, y, 1.0, 3.0, 0.5, 0.5);
                requireNear(state.x, expectedX[index], 1.0e-13,
                            "Latoocarfian x reference mismatch");
                requireNear(state.y, expectedY[index], 1.0e-13,
                            "Latoocarfian y reference mismatch");
                x = state.x;
                y = state.y;
            }
        }

        {
            const auto ikeda = sd::ikedaStep(0.1, -0.2, 0.9);
            requireNear(ikeda.x, 1.199325988688821, 1.0e-13, "Ikeda x mismatch");
            requireNear(ikeda.y, -0.027733557889748767, 1.0e-13, "Ikeda y mismatch");
            const auto lozi = sd::loziStep(0.1, -0.2, 1.7, 0.5);
            requireNear(lozi.x, 0.73, 1.0e-14, "Lozi x mismatch");
            requireNear(lozi.y, 0.1, 1.0e-14, "Lozi y mismatch");
            const auto tinkerbell = sd::tinkerbellStep(
                0.1, -0.2, 0.9, -0.6013, 2.0, 0.5);
            requireNear(tinkerbell.x, 0.18026, 1.0e-14, "Tinkerbell x mismatch");
            requireNear(tinkerbell.y, 0.06, 1.0e-14, "Tinkerbell y mismatch");
            const auto clifford = sd::cliffordStep(
                0.1, -0.2, -1.4, 1.6, 1.0, 0.7);
            requireNear(clifford.x, 1.266571644776751, 1.0e-13, "Clifford x mismatch");
            requireNear(clifford.y, 0.8237829992719545, 1.0e-13, "Clifford y mismatch");
            const auto deJong = sd::deJongStep(
                0.1, -0.2, 1.4, -2.3, 2.4, -2.1);
            requireNear(deJong.x, -1.2500220435694887, 1.0e-13, "DeJong x mismatch");
            requireNear(deJong.y, -0.6753863138851737, 1.0e-13, "DeJong y mismatch");
            requireNear(sd::circleStep(0.1, 0.2, 0.9), 0.21580596445902253,
                        1.0e-13, "Circle map mismatch");
            const auto coupled = sd::coupledLogisticStep(0.1, -0.2, 3.8, 3.8, 0.05);
            requireNear(coupled.x, 0.2793, 1.0e-14, "Coupled logistic x mismatch");
            requireNear(coupled.y, -0.8493, 1.0e-14, "Coupled logistic y mismatch");
            const auto rulkov = sd::rulkovStep(0.1, -0.2, 4.1, 0.001, -1.6);
            requireNear(rulkov.x, 3.8594059405940593, 1.0e-13, "Rulkov x mismatch");
            requireNear(rulkov.y, -0.2017, 1.0e-14, "Rulkov y mismatch");
        }

        {
            requireNear(sd::wrapValue(7.25, -1.0, 1.0), -0.75, 1.0e-14,
                        "wide wrap mismatch");
            requireNear(sd::foldValue(7.25, -1.0, 1.0), -0.75, 1.0e-14,
                        "wide fold mismatch");
        }

        {
            const double matrix[4] { 1.0, 0.8, 0.8, 1.0 };
            double lower[4] {};
            require(sd::choleskyFactor(matrix, lower, 2), "Cholesky factorization failed");
            requireNear(lower[0] * lower[0], 1.0, 1.0e-12, "Cholesky C00 mismatch");
            requireNear(lower[2] * lower[0], 0.8, 1.0e-12, "Cholesky C10 mismatch");
            requireNear(lower[2] * lower[2] + lower[3] * lower[3], 1.0, 1.0e-12,
                        "Cholesky C11 mismatch");
        }

        {
            requireNear(sd::inverseNormalCDF(0.5), 0.0, 1.0e-12,
                        "inverse normal median mismatch");
            requireNear(sd::normalCDF(sd::inverseNormalCDF(0.975)), 0.975, 1.0e-7,
                        "inverse normal roundtrip mismatch");
        }

        {
            sd::PCG32 rng {};
            rng.setSeed(0x53445547U);
            constexpr int count = 200000;
            double gammaMean = 0.0;
            double poissonMean = 0.0;
            for (int index = 0; index < count; ++index) {
                gammaMean += sd::gammaSample(rng, 2.0, 3.0);
                poissonMean += static_cast<double>(sd::poissonSample(rng, 7.0));
            }
            gammaMean /= count;
            poissonMean /= count;
            requireNear(gammaMean, 6.0, 0.06, "gamma empirical mean outside tolerance");
            requireNear(poissonMean, 7.0, 0.06, "Poisson empirical mean outside tolerance");
        }

        {
            sd::PCG32 rng {};
            rng.setSeed(99U);
            for (int index = 0; index < 10000; ++index) {
                const double value = sd::truncatedNormalSample(rng, 0.0, 1.0, -0.25, 0.75);
                require(value >= -0.25 && value <= 0.75,
                        "truncated normal escaped interval");
            }
            require(sd::finite(sd::stableS0Sample(rng, 1.5, 0.2, 1.0, 0.0)),
                    "stable sample was non-finite");
            require(sd::levySample(rng, 2.0, 1.0) >= 2.0,
                    "Levy sample escaped one-sided support");
        }

        std::cout << "StochasticDynamics reference tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "StochasticDynamics reference test failure: " << error.what() << '\n';
        return 1;
    }
}
