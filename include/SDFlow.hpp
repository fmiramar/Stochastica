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

} // namespace sd

