#pragma once

#include "SDCommon.hpp"

#include <algorithm>
#include <cmath>

namespace sd {

inline bool choleskyFactor(
    const double* matrix, double* lower, int size, double diagonalJitter = 0.0)
{
    if (!matrix || !lower || size < 1)
        return false;
    std::fill(lower, lower + size * size, 0.0);
    for (int row = 0; row < size; ++row) {
        for (int column = 0; column <= row; ++column) {
            double sum = matrix[row * size + column];
            if (row == column)
                sum += diagonalJitter;
            for (int inner = 0; inner < column; ++inner)
                sum -= lower[row * size + inner] * lower[column * size + inner];
            if (row == column) {
                if (!(sum > 0.0) || !finite(sum))
                    return false;
                lower[row * size + column] = std::sqrt(sum);
            } else {
                const double divisor = lower[column * size + column];
                if (!(divisor > 0.0) || !finite(divisor))
                    return false;
                lower[row * size + column] = sum / divisor;
            }
        }
    }
    return true;
}

} // namespace sd

