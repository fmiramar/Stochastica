#pragma once

#include "SDCommon.hpp"
#include "SDRange.hpp"

#include <cmath>

namespace sd {

struct State2 {
    double x;
    double y;
};

inline State2 henonStep(double x, double y, double a, double b)
{
    return { 1.0 - a * x * x + y, b * x };
}

inline State2 gbmanStep(double x, double y)
{
    return { 1.0 - y + std::abs(x), x };
}

inline State2 latoocarfianStep(double x, double y, double a, double b, double c, double d)
{
    return { std::sin(b * y) + c * std::sin(b * x),
             std::sin(a * x) + d * std::sin(a * y) };
}

inline State2 ikedaStep(double x, double y, double u)
{
    const double t = 0.4 - 6.0 / (1.0 + x * x + y * y);
    const double sine = std::sin(t);
    const double cosine = std::cos(t);
    return { 1.0 + u * (x * cosine - y * sine),
             u * (x * sine + y * cosine) };
}

inline State2 loziStep(double x, double y, double a, double b)
{
    return { 1.0 - a * std::abs(x) + b * y, x };
}

inline State2 tinkerbellStep(double x, double y, double a, double b, double c, double d)
{
    return { x * x - y * y + a * x + b * y,
             2.0 * x * y + c * x + d * y };
}

inline State2 cliffordStep(double x, double y, double a, double b, double c, double d)
{
    return { std::sin(a * y) + c * std::cos(a * x),
             std::sin(b * x) + d * std::cos(b * y) };
}

inline State2 deJongStep(double x, double y, double a, double b, double c, double d)
{
    return { std::sin(a * y) - std::cos(b * x),
             std::sin(c * x) - std::cos(d * y) };
}

inline State2 coupledLogisticStep(double x, double y, double rX, double rY, double coupling)
{
    const double fx = rX * x * (1.0 - x);
    const double fy = rY * y * (1.0 - y);
    return { (1.0 - coupling) * fx + coupling * fy,
             (1.0 - coupling) * fy + coupling * fx };
}

inline State2 rulkovStep(double x, double y, double alpha, double mu, double sigma)
{
    return { alpha / (1.0 + x * x) + y,
             y - mu * (x - sigma) };
}

inline double circleStep(double x, double omega, double k)
{
    return wrapValue(x + omega - (k / kTwoPi) * std::sin(kTwoPi * x), 0.0, 1.0);
}

} // namespace sd

