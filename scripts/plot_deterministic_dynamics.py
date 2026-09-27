#!/usr/bin/env python3
"""Render text-free deterministic-dynamics figures for SCDoc."""

from __future__ import annotations

import argparse
import math
import random
from pathlib import Path
from typing import Callable

from plot_chua_state_space import (
    BACKGROUND,
    BORDER,
    GRID,
    SUPERSAMPLE,
    TRACE_COLORS,
    Raster,
    fitter,
    write_png,
)


PhaseStep = Callable[[float, float], tuple[float, float]]


def phase_trajectory(
    step: PhaseStep,
    initial: tuple[float, float],
    iterations: int = 62_000,
    discard: int = 2_000,
) -> list[tuple[float, float]]:
    state = initial
    points: list[tuple[float, float]] = []
    for index in range(iterations):
        state = step(*state)
        if not math.isfinite(state[0]) or not math.isfinite(state[1]):
            break
        if index >= discard and index % 2 == 0:
            points.append(state)
    return points


def draw_frame(raster: Raster, rect: tuple[int, int, int, int]) -> None:
    left, top, right, bottom = rect
    for division in range(1, 8):
        x = round(left + (right - left) * division / 8)
        y = round(top + (bottom - top) * division / 8)
        raster.line((x, top), (x, bottom), GRID)
        raster.line((left, y), (right, y), GRID)
    raster.line((left, top), (right, top), BORDER)
    raster.line((right, top), (right, bottom), BORDER)
    raster.line((right, bottom), (left, bottom), BORDER)
    raster.line((left, bottom), (left, top), BORDER)


def dot(
    raster: Raster,
    point: tuple[float, float],
    color: tuple[int, int, int],
    alpha: float,
) -> None:
    x, y = round(point[0]), round(point[1])
    for offset_y in (-1, 0, 1):
        for offset_x in (-1, 0, 1):
            distance_scale = 1.0 if offset_x == 0 and offset_y == 0 else 0.72
            raster.set_pixel(x + offset_x, y + offset_y, color, alpha * distance_scale)


def finish(path: Path, raster: Raster) -> None:
    width, height, pixels = raster.downsample(SUPERSAMPLE)
    write_png(path, width, height, pixels)


def render_phase(path: Path, points: list[tuple[float, float]]) -> None:
    output_width, output_height = 1200, 820
    width, height = output_width * SUPERSAMPLE, output_height * SUPERSAMPLE
    raster = Raster(width, height)
    rect = (0, 0, width - 1, height - 1)
    draw_frame(raster, rect)
    transform = fitter(points, rect)
    denominator = max(1, len(points) - 1)
    for index, point in enumerate(points):
        color_index = min(len(TRACE_COLORS) - 1, index * len(TRACE_COLORS) // denominator)
        dot(raster, transform(point), TRACE_COLORS[color_index], 0.58)
    finish(path, raster)


def circle_step(value: float, omega: float = 0.2, coupling: float = 0.9) -> float:
    return (value + omega - (coupling / (2.0 * math.pi)) * math.sin(2.0 * math.pi * value)) % 1.0


def render_circle(path: Path) -> None:
    output_width, output_height = 1200, 820
    width, height = output_width * SUPERSAMPLE, output_height * SUPERSAMPLE
    raster = Raster(width, height)
    rect = (0, 0, width - 1, height - 1)
    draw_frame(raster, rect)
    transform = fitter([(0.0, 0.0), (1.0, 1.0)], rect)

    identity = [transform((index / 1200, index / 1200)) for index in range(1201)]
    curve_values = [(index / 2400, circle_step(index / 2400)) for index in range(2401)]
    curve = [transform(point) for point in curve_values]
    for start, end in zip(identity, identity[1:]):
        raster.line(start, end, (160, 160, 160), width=2.0, alpha=0.8)
    for index, (start, end) in enumerate(zip(curve, curve[1:])):
        if abs(curve_values[index + 1][1] - curve_values[index][1]) < 0.5:
            raster.line(start, end, TRACE_COLORS[0], width=2.8, alpha=0.85)

    value = 0.0
    current = transform((value, value))
    for _ in range(65):
        following = circle_step(value)
        vertical = transform((value, following))
        diagonal = transform((following, following))
        raster.line(current, vertical, TRACE_COLORS[2], width=2.2, alpha=0.78)
        raster.line(vertical, diagonal, TRACE_COLORS[2], width=2.2, alpha=0.78)
        value = following
        current = diagonal
    finish(path, raster)


def render_lattice(path: Path) -> None:
    output_width, output_height = 1200, 700
    raster = Raster(output_width, output_height)
    cells = 48
    iterations = 400
    generator = random.Random(0x5DCA77)
    state = [0.3 + 0.2 * generator.random() for _ in range(cells)]
    rows: list[list[float]] = []
    for _ in range(iterations):
        rows.append(state[:])
        images = [3.88 * value * (1.0 - value) for value in state]
        state = [
            0.86 * images[index]
            + 0.07 * (images[(index - 1) % cells] + images[(index + 1) % cells])
            for index in range(cells)
        ]

    low = (246, 248, 250)
    middle = (72, 121, 151)
    high = (116, 72, 118)
    for y in range(output_height):
        iteration = min(iterations - 1, y * iterations // output_height)
        for x in range(output_width):
            cell = min(cells - 1, x * cells // output_width)
            value = max(0.0, min(1.0, rows[iteration][cell]))
            if value < 0.5:
                fraction = value * 2.0
                color = tuple(round(low[c] + (middle[c] - low[c]) * fraction) for c in range(3))
            else:
                fraction = (value - 0.5) * 2.0
                color = tuple(round(middle[c] + (high[c] - middle[c]) * fraction) for c in range(3))
            raster.set_pixel(x, y, color)
    raster.line((0, 0), (output_width - 1, 0), BORDER)
    raster.line((output_width - 1, 0), (output_width - 1, output_height - 1), BORDER)
    raster.line((output_width - 1, output_height - 1), (0, output_height - 1), BORDER)
    raster.line((0, output_height - 1), (0, 0), BORDER)
    write_png(path, output_width, output_height, raster.pixels)


def mackey_glass() -> tuple[list[tuple[float, float]], list[tuple[float, float]]]:
    step = 0.02
    delay_steps = round(17.0 / step)
    history = [1.2] * (delay_steps * 2 + 8)
    write_index = 0
    state = 1.2
    time_series: list[tuple[float, float]] = []
    embedding: list[tuple[float, float]] = []
    for index in range(90_000):
        delayed = history[(write_index - delay_steps) % len(history)]
        first = 0.2 * delayed / (1.0 + delayed**10) - 0.1 * state
        predicted = state + step * first
        second = 0.2 * delayed / (1.0 + delayed**10) - 0.1 * predicted
        state += 0.5 * step * (first + second)
        history[write_index] = state
        write_index = (write_index + 1) % len(history)
        if index >= 30_000 and index % 8 == 0:
            delayed_now = history[(write_index - delay_steps) % len(history)]
            time_series.append((len(time_series), state))
            embedding.append((state, delayed_now))
    return time_series[-2400:], embedding


def izhikevich() -> tuple[list[tuple[float, float]], list[tuple[float, float]]]:
    voltage, recovery = -65.0, -13.0
    timeline: list[tuple[float, float]] = []
    phase: list[tuple[float, float]] = []
    step = 0.1
    for index in range(3500):
        dv = lambda v, u: 0.04 * v * v + 5.0 * v + 140.0 - u + 12.0
        voltage += 0.5 * step * dv(voltage, recovery)
        voltage += 0.5 * step * dv(voltage, recovery)
        recovery += step * 0.02 * (0.2 * voltage - recovery)
        if voltage >= 30.0:
            timeline.append((index * step, 30.0))
            phase.append((30.0, recovery))
            voltage = -65.0
            recovery += 8.0
        timeline.append((index * step, voltage))
        phase.append((voltage, recovery))
    return timeline, phase


def render_two_panel_lines(
    path: Path,
    left_points: list[tuple[float, float]],
    right_points: list[tuple[float, float]],
) -> None:
    output_width, output_height = 1800, 600
    width, height = output_width * SUPERSAMPLE, output_height * SUPERSAMPLE
    raster = Raster(width, height)
    gap = 18 * SUPERSAMPLE
    panel_width = (width - gap) // 2
    panels = (
        (left_points, (0, 0, panel_width - 1, height - 1)),
        (right_points, (panel_width + gap, 0, width - 1, height - 1)),
    )
    for points, rect in panels:
        draw_frame(raster, rect)
        transform = fitter(points, rect)
        fitted = [transform(point) for point in points]
        denominator = max(1, len(fitted) - 1)
        for index, (start, end) in enumerate(zip(fitted, fitted[1:])):
            color_index = min(len(TRACE_COLORS) - 1, index * len(TRACE_COLORS) // denominator)
            raster.line(start, end, TRACE_COLORS[color_index], width=2.4, alpha=0.75)
    finish(path, raster)


def write_figures(output_directory: Path) -> None:
    output_directory.mkdir(parents=True, exist_ok=True)
    phase_maps: dict[str, tuple[PhaseStep, tuple[float, float]]] = {
        "henon-map-phase.png": (lambda x, y: (1.0 - 1.4 * x * x + y, 0.3 * x), (0.0, 0.0)),
        "gbman-map-phase.png": (lambda x, y: (1.0 - y + abs(x), x), (1.2, 2.1)),
        "latoocarfian-map-phase.png": (
            lambda x, y: (math.sin(3.0 * y) + 0.5 * math.sin(3.0 * x), math.sin(x) + 0.5 * math.sin(y)),
            (0.1, 0.1),
        ),
        "ikeda-map-phase.png": (
            lambda x, y: (
                1.0 + 0.9 * (x * math.cos(0.4 - 6.0 / (1.0 + x * x + y * y)) - y * math.sin(0.4 - 6.0 / (1.0 + x * x + y * y))),
                0.9 * (x * math.sin(0.4 - 6.0 / (1.0 + x * x + y * y)) + y * math.cos(0.4 - 6.0 / (1.0 + x * x + y * y))),
            ),
            (0.1, 0.1),
        ),
        "lozi-map-phase.png": (lambda x, y: (1.0 - 1.7 * abs(x) + 0.5 * y, x), (0.1, 0.0)),
        "tinkerbell-map-phase.png": (
            lambda x, y: (x * x - y * y + 0.9 * x - 0.6013 * y, 2.0 * x * y + 2.0 * x + 0.5 * y),
            (-0.72, -0.64),
        ),
        "clifford-map-phase.png": (
            lambda x, y: (math.sin(-1.4 * y) + math.cos(-1.4 * x), math.sin(1.6 * x) + 0.7 * math.cos(1.6 * y)),
            (0.1, 0.1),
        ),
        "dejong-map-phase.png": (
            lambda x, y: (math.sin(1.4 * y) - math.cos(-2.3 * x), math.sin(2.4 * x) - math.cos(-2.1 * y)),
            (0.0, 0.0),
        ),
        "coupled-logistic-map-phase.png": (
            lambda x, y: (
                0.95 * 3.8 * x * (1.0 - x) + 0.05 * 3.8 * y * (1.0 - y),
                0.95 * 3.8 * y * (1.0 - y) + 0.05 * 3.8 * x * (1.0 - x),
            ),
            (0.2, 0.21),
        ),
        "rulkov-map-phase.png": (lambda x, y: (4.1 / (1.0 + x * x) + y, y - 0.001 * (x + 1.6)), (-1.0, -2.9)),
    }
    for filename, (step, initial) in phase_maps.items():
        render_phase(output_directory / filename, phase_trajectory(step, initial))

    render_circle(output_directory / "circle-map-cobweb.png")
    render_lattice(output_directory / "coupled-map-lattice-space-time.png")
    time_series, embedding = mackey_glass()
    render_two_panel_lines(output_directory / "mackey-glass-dynamics.png", time_series, embedding)
    voltage, phase = izhikevich()
    render_two_panel_lines(output_directory / "izhikevich-dynamics.png", voltage, phase)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("output_directory", type=Path)
    arguments = parser.parse_args()
    write_figures(arguments.output_directory)


if __name__ == "__main__":
    main()
