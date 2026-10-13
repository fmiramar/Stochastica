#pragma once

#include "SDCommon.hpp"

#include <array>
#include <cstddef>

namespace sd {

template <std::size_t Size, typename Derivative>
inline std::array<double, Size> rk4(
    const std::array<double, Size>& state, double step, Derivative derivative)
{
    const auto k1 = derivative(state);
    std::array<double, Size> temporary {};
    for (std::size_t index = 0; index < Size; ++index)
        temporary[index] = state[index] + 0.5 * step * k1[index];
    const auto k2 = derivative(temporary);
    for (std::size_t index = 0; index < Size; ++index)
        temporary[index] = state[index] + 0.5 * step * k2[index];
    const auto k3 = derivative(temporary);
    for (std::size_t index = 0; index < Size; ++index)
        temporary[index] = state[index] + step * k3[index];
    const auto k4 = derivative(temporary);

    std::array<double, Size> result {};
    for (std::size_t index = 0; index < Size; ++index)
        result[index] = state[index] + (step / 6.0)
            * (k1[index] + 2.0 * k2[index] + 2.0 * k3[index] + k4[index]);
    return result;
}

// Runtime-sized counterpart for bounded spatial flows. The caller owns five
// n-element scratch arrays; existing fixed-size users retain their exact code.
template <typename Derivative>
inline void rk4(double* state, int n, double step, double* scratch, Derivative derivative)
{
    double* k1=scratch;
    double* k2=k1+n;
    double* k3=k2+n;
    double* k4=k3+n;
    double* temporary=k4+n;
    derivative(state,k1);
    for (int i=0; i<n; ++i) temporary[i]=state[i]+0.5*step*k1[i];
    derivative(temporary,k2);
    for (int i=0; i<n; ++i) temporary[i]=state[i]+0.5*step*k2[i];
    derivative(temporary,k3);
    for (int i=0; i<n; ++i) temporary[i]=state[i]+step*k3[i];
    derivative(temporary,k4);
    for (int i=0; i<n; ++i) state[i]+=(step/6.0)*(k1[i]+2*k2[i]+2*k3[i]+k4[i]);
}

} // namespace sd

