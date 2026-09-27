#!/usr/bin/env python3
"""Render text-free stochastic-process, state-model, and distribution figures."""

from __future__ import annotations

import argparse
import math
import random
from pathlib import Path
from typing import Callable, Iterable

from plot_chua_state_space import BORDER, GRID, SUPERSAMPLE, TRACE_COLORS, Raster, fitter, write_png
from plot_deterministic_dynamics import dot, draw_frame, finish


Point = tuple[float, float]
Curve = list[Point]


def polyline(
    raster: Raster,
    curve: Curve,
    transform: Callable[[Point], Point],
    color: tuple[int, int, int],
    width: float = 2.5,
    alpha: float = 0.82,
) -> None:
    points = [transform(point) for point in curve]
    for start, end in zip(points, points[1:]):
        raster.line(start, end, color, width=width, alpha=alpha)


def render_curves(path: Path, curves: list[Curve], size: tuple[int, int] = (1200, 820)) -> None:
    output_width, output_height = size
    width, height = output_width * SUPERSAMPLE, output_height * SUPERSAMPLE
    raster = Raster(width, height)
    rect = (0, 0, width - 1, height - 1)
    draw_frame(raster, rect)
    transform = fitter([point for curve in curves for point in curve], rect)
    for curve, color in zip(curves, TRACE_COLORS):
        polyline(raster, curve, transform, color)
    finish(path, raster)


def render_two_curves(path: Path, left: list[Curve], right: list[Curve]) -> None:
    output_width, output_height = 1800, 600
    width, height = output_width * SUPERSAMPLE, output_height * SUPERSAMPLE
    raster = Raster(width, height)
    gap = 18 * SUPERSAMPLE
    panel_width = (width - gap) // 2
    panels = (
        (left, (0, 0, panel_width - 1, height - 1)),
        (right, (panel_width + gap, 0, width - 1, height - 1)),
    )
    for curves, rect in panels:
        draw_frame(raster, rect)
        transform = fitter([point for curve in curves for point in curve], rect)
        for curve, color in zip(curves, TRACE_COLORS):
            polyline(raster, curve, transform, color)
    finish(path, raster)


def density_curve(function: Callable[[float], float], low: float, high: float, count: int = 1400) -> Curve:
    result = []
    for index in range(count):
        value = low + (high - low) * index / (count - 1)
        density = function(value)
        if math.isfinite(density):
            result.append((value, density))
    return result


def mass_curve(function: Callable[[int], float], maximum: int) -> Curve:
    points: Curve = []
    for value in range(maximum + 1):
        probability = function(value)
        points.extend(((value - 0.001, 0.0), (value, probability), (value + 0.001, 0.0)))
    return points


def histogram_curve(samples: Iterable[float], low: float, high: float, bins: int = 180) -> Curve:
    counts = [0] * bins
    accepted = 0
    for value in samples:
        if low <= value <= high:
            index = min(bins - 1, int((value - low) * bins / (high - low)))
            counts[index] += 1
            accepted += 1
    width = (high - low) / bins
    scale = max(1, accepted) * width
    return [(low + (index + 0.5) * width, count / scale) for index, count in enumerate(counts)]


def smooth_curve(curve: Curve, radius: int = 4) -> Curve:
    smoothed = []
    for index, point in enumerate(curve):
        start = max(0, index - radius)
        stop = min(len(curve), index + radius + 1)
        smoothed.append((point[0], sum(value[1] for value in curve[start:stop]) / (stop - start)))
    return smoothed


def ou_figure(path: Path) -> None:
    generator = random.Random(0x0A11CE)
    state, dt, theta, sigma = 0.0, 0.02, 1.0, 1.0
    decay = math.exp(-theta * dt)
    deviation = sigma * math.sqrt((1.0 - decay * decay) / (2.0 * theta))
    trace: Curve = []
    for index in range(5000):
        state = decay * state + deviation * generator.gauss(0.0, 1.0)
        if index >= 3000:
            trace.append(((index - 3000) * dt, state))
    stationary_deviation = sigma / math.sqrt(2.0 * theta)
    density = density_curve(
        lambda value: math.exp(-0.5 * (value / stationary_deviation) ** 2)
        / (stationary_deviation * math.sqrt(2.0 * math.pi)),
        -3.0,
        3.0,
    )
    render_two_curves(path, [trace], [density])


def renewal_figure(path: Path) -> None:
    generator = random.Random(0x0E11A1)
    rate, refractory = 4.0, 0.05
    intervals = [refractory + generator.expovariate(rate) for _ in range(30_000)]
    events: Curve = []
    time = 0.0
    for interval in intervals:
        time += interval
        if time > 20.0:
            break
        events.extend(((time - 0.002, 0.0), (time, 1.0), (time + 0.002, 0.0)))
    histogram = histogram_curve(intervals, 0.0, 1.6, 110)
    render_two_curves(path, [events], [histogram])


def hawkes_figure(path: Path) -> None:
    generator = random.Random(0xA11CE5)
    base, excitation, decay, dt = 0.6, 8.0, 10.0, 0.002
    extra = 0.0
    intensity: Curve = []
    events: Curve = []
    for index in range(round(35.0 / dt)):
        time = index * dt
        extra *= math.exp(-decay * dt)
        current = base + extra
        if index % 5 == 0:
            intensity.append((time, current))
        if generator.random() < current * dt:
            events.extend(((time - 0.003, 0.0), (time, 7.0), (time + 0.003, 0.0)))
            extra += excitation
    render_curves(path, [intensity, events], (1800, 600))


def fractional_spectrum_figure(path: Path) -> None:
    curves = []
    for alpha in (0.0, 1.0, 2.0):
        curves.append([(index / 1000.0, 1.0 - alpha * index / 2000.0) for index in range(1001)])
    render_curves(path, curves)


def fractional_path(hurst: float, seed: int, length: int = 2200, memory: int = 192) -> Curve:
    generator = random.Random(seed)
    d = hurst - 0.5
    coefficients = [1.0]
    for index in range(1, memory):
        coefficients.append(coefficients[-1] * ((index - 1) + d) / index)
    normalization = math.sqrt(sum(value * value for value in coefficients))
    coefficients = [value / normalization for value in coefficients]
    innovations = [0.0] * memory
    write_index = 0
    state = 0.0
    values = []
    for index in range(length):
        innovations[write_index] = generator.gauss(0.0, 1.0)
        read_index = write_index
        increment = 0.0
        for coefficient in coefficients:
            increment += coefficient * innovations[read_index]
            read_index = (read_index - 1) % memory
        write_index = (write_index + 1) % memory
        state += increment
        values.append(state)
    minimum, maximum = min(values), max(values)
    span = max(maximum - minimum, 1e-12)
    return [(index, 2.0 * (value - minimum) / span - 1.0) for index, value in enumerate(values)]


def fbrownian_figure(path: Path) -> None:
    render_curves(path, [
        fractional_path(0.2, 101),
        fractional_path(0.5, 202),
        fractional_path(0.8, 303),
    ])


def apply_boundary(value: float, mode: int) -> tuple[float, bool]:
    if mode == 0:
        while value < -1.0 or value > 1.0:
            value = -2.0 - value if value < -1.0 else 2.0 - value
        return value, False
    if mode == 1:
        return ((value + 1.0) % 2.0) - 1.0, False
    if value < -1.0:
        return -1.0, True
    if value > 1.0:
        return 1.0, True
    return value, False


def bounded_walk(mode: int, seed: int) -> Curve:
    generator = random.Random(seed)
    value, absorbed = 0.0, False
    points: Curve = []
    for index in range(700):
        if not absorbed:
            value, hit = apply_boundary(value + generator.uniform(-0.24, 0.24), mode)
            absorbed = mode == 4 and hit
        points.append((index, value))
    return points


def bounded_walk_figure(path: Path) -> None:
    output_width, output_height = 1800, 560
    width, height = output_width * SUPERSAMPLE, output_height * SUPERSAMPLE
    raster = Raster(width, height)
    gap = 18 * SUPERSAMPLE
    panel_width = (width - 2 * gap) // 3
    for index, mode in enumerate((0, 1, 4)):
        rect = (index * (panel_width + gap), 0, index * (panel_width + gap) + panel_width - 1, height - 1)
        draw_frame(raster, rect)
        transform = fitter([(0, -1), (699, 1)], rect)
        polyline(raster, bounded_walk(mode, 707), transform, TRACE_COLORS[index])
    finish(path, raster)


def choose(weights: list[float], generator: random.Random) -> int:
    threshold = generator.random() * sum(weights)
    total = 0.0
    for index, weight in enumerate(weights):
        total += weight
        if threshold <= total:
            return index
    return len(weights) - 1


def fill_rect(raster: Raster, rect: tuple[int, int, int, int], color: tuple[int, int, int]) -> None:
    left, top, right, bottom = rect
    for y in range(top, bottom + 1):
        offset = (y * raster.width + left) * 3
        row = bytes(color) * (right - left + 1)
        raster.pixels[offset : offset + len(row)] = row


def heat_color(value: float) -> tuple[int, int, int]:
    low, high = (247, 249, 250), (69, 108, 145)
    value = max(0.0, min(1.0, value))
    return tuple(round(low[channel] + (high[channel] - low[channel]) * value) for channel in range(3))


def markov_figure(path: Path) -> None:
    matrix = [[0.72, 0.20, 0.08], [0.15, 0.65, 0.20], [0.08, 0.27, 0.65]]
    generator = random.Random(0x5A7E)
    state = 0
    timeline: Curve = [(0, state)]
    for index in range(1, 180):
        state = choose(matrix[state], generator)
        timeline.extend(((index, timeline[-1][1]), (index, state)))

    output_width, output_height = 1800, 600
    width, height = output_width * SUPERSAMPLE, output_height * SUPERSAMPLE
    raster = Raster(width, height)
    gap = 18 * SUPERSAMPLE
    left_width = 600 * SUPERSAMPLE
    matrix_rect = (0, 0, left_width - 1, height - 1)
    cell_width = left_width // 3
    cell_height = height // 3
    for row in range(3):
        for column in range(3):
            rect = (column * cell_width, row * cell_height, (column + 1) * cell_width - 1, (row + 1) * cell_height - 1)
            fill_rect(raster, rect, heat_color(matrix[row][column]))
            raster.line((rect[0], rect[1]), (rect[2], rect[1]), BORDER)
            raster.line((rect[2], rect[1]), (rect[2], rect[3]), BORDER)
    plot_rect = (left_width + gap, 0, width - 1, height - 1)
    draw_frame(raster, plot_rect)
    transform = fitter([(0, -0.2), (179, 2.2)], plot_rect)
    polyline(raster, timeline, transform, TRACE_COLORS[2], width=2.8)
    finish(path, raster)


def semi_markov_figure(path: Path) -> None:
    matrix = [[0.7, 0.2, 0.1], [0.2, 0.6, 0.2], [0.1, 0.3, 0.6]]
    means = [0.6, 0.2, 1.4]
    generator = random.Random(0x5E611)
    state, remaining, dt = 0, means[0], 0.02
    states: Curve = []
    countdown: Curve = []
    for index in range(2500):
        time = index * dt
        remaining -= dt
        if remaining <= 0.0:
            state = choose(matrix[state], generator)
            if state == 0:
                remaining += means[state]
            elif state == 1:
                remaining += generator.gammavariate(2.0, means[state] / 2.0)
            else:
                remaining += generator.weibullvariate(means[state] / math.gamma(3.0), 0.5)
        states.append((time, state))
        countdown.append((time, max(remaining, 0.0)))
    render_two_curves(path, [states], [countdown])


def multigauss_figure(path: Path) -> None:
    generator = random.Random(0xC0A4)
    correlation = 0.8
    samples = []
    for _ in range(30_000):
        first = generator.gauss(0.0, 1.0)
        second = correlation * first + math.sqrt(1.0 - correlation * correlation) * generator.gauss(0.0, 1.0)
        samples.append((first, second))
    output_width, output_height = 1200, 820
    width, height = output_width * SUPERSAMPLE, output_height * SUPERSAMPLE
    raster = Raster(width, height)
    rect = (0, 0, width - 1, height - 1)
    draw_frame(raster, rect)
    transform = fitter([(-4.0, -4.0), (4.0, 4.0)], rect)
    for index, sample in enumerate(samples):
        color = TRACE_COLORS[min(2, index * 3 // len(samples))]
        dot(raster, transform(sample), color, 0.18)
    origin = transform((0.0, 0.0))
    raster.arrow(origin, transform((2.6, 2.6)), (67, 137, 101))
    raster.arrow(origin, transform((-0.75, 0.75)), (190, 83, 76))
    finish(path, raster)


def stable_sample(generator: random.Random, alpha: float) -> float:
    if alpha == 2.0:
        return math.sqrt(2.0) * generator.gauss(0.0, 1.0)
    if alpha == 1.0:
        return math.tan(math.pi * (generator.random() - 0.5))
    angle = math.pi * (generator.random() - 0.5)
    exponential = generator.expovariate(1.0)
    return (
        math.sin(alpha * angle)
        / math.cos(angle) ** (1.0 / alpha)
        * (math.cos((1.0 - alpha) * angle) / exponential) ** ((1.0 - alpha) / alpha)
    )


def distribution_figures(directory: Path) -> None:
    gamma_curves = [
        density_curve(lambda x, shape=shape: x ** (shape - 1.0) * math.exp(-x) / math.gamma(shape), 0.02, 10.0)
        for shape in (0.7, 2.0, 5.0)
    ]
    render_curves(directory / "gamma-density.png", gamma_curves)

    weibull_curves = [
        density_curve(lambda x, shape=shape: shape * x ** (shape - 1.0) * math.exp(-(x**shape)), 0.02, 5.0)
        for shape in (0.6, 1.0, 2.0)
    ]
    render_curves(directory / "weibull-density.png", weibull_curves)

    lognormal_curves = [
        density_curve(
            lambda x, sigma=sigma: math.exp(-(math.log(x) ** 2) / (2.0 * sigma * sigma))
            / (x * sigma * math.sqrt(2.0 * math.pi)),
            0.03,
            8.0,
        )
        for sigma in (0.3, 0.7, 1.0)
    ]
    render_curves(directory / "lognormal-density.png", lognormal_curves)

    cauchy_curves = [
        density_curve(lambda x, scale=scale: 1.0 / (math.pi * scale * (1.0 + (x / scale) ** 2)), -8.0, 8.0)
        for scale in (0.5, 1.0, 2.0)
    ]
    render_curves(directory / "cauchy-density.png", cauchy_curves)

    poisson_curves = [
        mass_curve(lambda value, mean=mean: math.exp(-mean + value * math.log(mean) - math.lgamma(value + 1.0)), 24)
        for mean in (1.0, 4.0, 10.0)
    ]
    render_curves(directory / "poisson-mass.png", poisson_curves)

    geometric_curves = [
        mass_curve(lambda value, probability=probability: probability * (1.0 - probability) ** value, 20)
        for probability in (0.2, 0.5, 0.8)
    ]
    render_curves(directory / "geometric-mass.png", geometric_curves)

    negbin_curves = [
        mass_curve(
            lambda value, probability=probability: math.exp(
                math.lgamma(value + 2.0) - math.lgamma(2.0) - math.lgamma(value + 1.0)
                + 2.0 * math.log(probability) + value * math.log(1.0 - probability)
            ),
            24,
        )
        for probability in (0.25, 0.5, 0.75)
    ]
    render_curves(directory / "negative-binomial-mass.png", negbin_curves)

    normalizer = math.erf(1.0 / math.sqrt(2.0))
    truncated = density_curve(
        lambda x: math.exp(-0.5 * x * x) / math.sqrt(2.0 * math.pi) / normalizer if -1.0 <= x <= 1.0 else 0.0,
        -3.0,
        3.0,
    )
    ordinary = density_curve(lambda x: math.exp(-0.5 * x * x) / math.sqrt(2.0 * math.pi), -3.0, 3.0)
    render_curves(directory / "truncated-normal-density.png", [ordinary, truncated])

    stable_curves = []
    for index, alpha in enumerate((2.0, 1.5, 1.0)):
        generator = random.Random(0x57AB1E + index)
        stable_curves.append(smooth_curve(
            histogram_curve((stable_sample(generator, alpha) for _ in range(120_000)), -8.0, 8.0)
        ))
    render_curves(directory / "stable-density.png", stable_curves)

    levy_curves = [
        density_curve(
            lambda x, scale=scale: math.sqrt(scale / (2.0 * math.pi)) * math.exp(-scale / (2.0 * x)) / x ** 1.5,
            0.03,
            12.0,
        )
        for scale in (0.5, 1.0, 2.0)
    ]
    render_curves(directory / "levy-density.png", levy_curves)


def write_figures(output_directory: Path) -> None:
    output_directory.mkdir(parents=True, exist_ok=True)
    ou_figure(output_directory / "ou-process-dynamics.png")
    renewal_figure(output_directory / "renewal-process-events.png")
    hawkes_figure(output_directory / "hawkes-events-intensity.png")
    fractional_spectrum_figure(output_directory / "fractional-noise-spectra.png")
    fbrownian_figure(output_directory / "fbrownian-paths.png")
    bounded_walk_figure(output_directory / "bounded-walk-boundaries.png")
    markov_figure(output_directory / "markov-chain-dynamics.png")
    semi_markov_figure(output_directory / "semi-markov-dynamics.png")
    multigauss_figure(output_directory / "multigauss-correlation.png")
    distribution_figures(output_directory)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("output_directory", type=Path)
    arguments = parser.parse_args()
    write_figures(arguments.output_directory)


if __name__ == "__main__":
    main()
