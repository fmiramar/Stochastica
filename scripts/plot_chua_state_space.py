#!/usr/bin/env python3
"""Render two text-free Chua state-space PNGs for SCDoc.

The renderer uses only the Python standard library. It draws at twice the
requested resolution and downsamples the result so the checked-in images have
smooth lines without requiring matplotlib, Pillow, or a platform GUI tool.
"""

from __future__ import annotations

import argparse
import binascii
import math
import struct
import zlib
from pathlib import Path


SUPERSAMPLE = 2

BACKGROUND = (255, 255, 255)
GRID = (232, 232, 232)
BORDER = (196, 196, 196)
TRACE_COLORS = (
    (43, 100, 126),
    (72, 98, 145),
    (111, 82, 123),
)
AXIS_COLORS = (
    (190, 83, 76),
    (67, 137, 101),
    (65, 105, 166),
)


def derivative(state: tuple[float, float, float]) -> tuple[float, float, float]:
    x, y, z = state
    alpha, beta, m0, m1 = 15.6, 28.0, -1.143, -0.714
    diode = m1 * x + 0.5 * (m0 - m1) * (abs(x + 1.0) - abs(x - 1.0))
    return alpha * (y - x - diode), x - y + z, -beta * y


def add_scaled(
    state: tuple[float, float, float],
    slope: tuple[float, float, float],
    scale: float,
) -> tuple[float, float, float]:
    return tuple(state[index] + slope[index] * scale for index in range(3))  # type: ignore[return-value]


def rk4(
    state: tuple[float, float, float], step: float
) -> tuple[float, float, float]:
    k1 = derivative(state)
    k2 = derivative(add_scaled(state, k1, 0.5 * step))
    k3 = derivative(add_scaled(state, k2, 0.5 * step))
    k4 = derivative(add_scaled(state, k3, step))
    return tuple(
        state[index]
        + step * (k1[index] + 2.0 * k2[index] + 2.0 * k3[index] + k4[index]) / 6.0
        for index in range(3)
    )  # type: ignore[return-value]


def trajectory() -> list[tuple[float, float, float]]:
    state = (0.7, 0.0, 0.0)
    points: list[tuple[float, float, float]] = []
    for index in range(82_000):
        state = rk4(state, 0.0025)
        if index >= 4_000 and index % 3 == 0:
            points.append(state)
    return points


def oblique(state: tuple[float, float, float]) -> tuple[float, float]:
    # Normalize the differently sized model coordinates before projection.
    x, y, z = state[0] / 2.4, state[1] / 0.45, state[2] / 4.0
    return x * 0.72 - z * 0.62, x * 0.24 + z * 0.29 - y * 0.90


def project(
    state: tuple[float, float, float], first: int, second: int
) -> tuple[float, float]:
    return state[first], state[second]


def fitter(
    points: list[tuple[float, float]],
    rect: tuple[float, float, float, float],
):
    minimum_x = min(point[0] for point in points)
    maximum_x = max(point[0] for point in points)
    minimum_y = min(point[1] for point in points)
    maximum_y = max(point[1] for point in points)
    span_x = max(maximum_x - minimum_x, 1e-12)
    span_y = max(maximum_y - minimum_y, 1e-12)
    left, top, right, bottom = rect
    width = right - left
    height = bottom - top
    margin_x = width * 0.035
    margin_y = height * 0.05

    def transform(point: tuple[float, float]) -> tuple[float, float]:
        return (
            left + margin_x + (point[0] - minimum_x) * (width - 2.0 * margin_x) / span_x,
            bottom - margin_y - (point[1] - minimum_y) * (height - 2.0 * margin_y) / span_y,
        )

    return transform


class Raster:
    def __init__(self, width: int, height: int) -> None:
        self.width = width
        self.height = height
        self.pixels = bytearray(BACKGROUND * (width * height))

    def set_pixel(self, x: int, y: int, color: tuple[int, int, int], alpha: float = 1.0) -> None:
        if x < 0 or x >= self.width or y < 0 or y >= self.height:
            return
        offset = (y * self.width + x) * 3
        inverse = 1.0 - alpha
        self.pixels[offset] = round(self.pixels[offset] * inverse + color[0] * alpha)
        self.pixels[offset + 1] = round(self.pixels[offset + 1] * inverse + color[1] * alpha)
        self.pixels[offset + 2] = round(self.pixels[offset + 2] * inverse + color[2] * alpha)

    def line(
        self,
        start: tuple[float, float],
        end: tuple[float, float],
        color: tuple[int, int, int],
        width: float = 1.0,
        alpha: float = 1.0,
    ) -> None:
        x0, y0 = start
        x1, y1 = end
        delta_x = x1 - x0
        delta_y = y1 - y0
        steps = max(1, math.ceil(max(abs(delta_x), abs(delta_y))))
        radius = max(0, math.ceil(width * 0.5) - 1)
        for step in range(steps + 1):
            fraction = step / steps
            x = round(x0 + delta_x * fraction)
            y = round(y0 + delta_y * fraction)
            for offset_y in range(-radius, radius + 1):
                for offset_x in range(-radius, radius + 1):
                    if offset_x * offset_x + offset_y * offset_y <= radius * radius + 0.25:
                        self.set_pixel(x + offset_x, y + offset_y, color, alpha)

    def arrow(
        self,
        start: tuple[float, float],
        end: tuple[float, float],
        color: tuple[int, int, int],
    ) -> None:
        self.line(start, end, color, width=3.2, alpha=0.9)
        angle = math.atan2(end[1] - start[1], end[0] - start[0])
        head = 13.0 * SUPERSAMPLE
        spread = 0.48
        first = (
            end[0] - head * math.cos(angle - spread),
            end[1] - head * math.sin(angle - spread),
        )
        second = (
            end[0] - head * math.cos(angle + spread),
            end[1] - head * math.sin(angle + spread),
        )
        self.line(end, first, color, width=3.2, alpha=0.9)
        self.line(end, second, color, width=3.2, alpha=0.9)

    def downsample(self, factor: int) -> tuple[int, int, bytearray]:
        output_width = self.width // factor
        output_height = self.height // factor
        output = bytearray(output_width * output_height * 3)
        divisor = factor * factor
        for output_y in range(output_height):
            for output_x in range(output_width):
                sums = [0, 0, 0]
                for offset_y in range(factor):
                    source_y = output_y * factor + offset_y
                    for offset_x in range(factor):
                        source_x = output_x * factor + offset_x
                        source = (source_y * self.width + source_x) * 3
                        sums[0] += self.pixels[source]
                        sums[1] += self.pixels[source + 1]
                        sums[2] += self.pixels[source + 2]
                destination = (output_y * output_width + output_x) * 3
                output[destination] = round(sums[0] / divisor)
                output[destination + 1] = round(sums[1] / divisor)
                output[destination + 2] = round(sums[2] / divisor)
        return output_width, output_height, output


def png_chunk(chunk_type: bytes, payload: bytes) -> bytes:
    return (
        struct.pack(">I", len(payload))
        + chunk_type
        + payload
        + struct.pack(">I", binascii.crc32(chunk_type + payload) & 0xFFFFFFFF)
    )


def write_png(path: Path, width: int, height: int, pixels: bytearray) -> None:
    scanlines = b"".join(
        b"\x00" + bytes(pixels[row * width * 3 : (row + 1) * width * 3])
        for row in range(height)
    )
    encoded = (
        b"\x89PNG\r\n\x1a\n"
        + png_chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0))
        + png_chunk(b"IDAT", zlib.compress(scanlines, 9))
        + png_chunk(b"IEND", b"")
    )
    path.write_bytes(encoded)


def draw_panel(
    raster: Raster,
    points: list[tuple[float, float]],
    rect: tuple[int, int, int, int],
) -> object:
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

    transform = fitter(points, rect)
    fitted = [transform(point) for point in points]
    denominator = max(1, len(fitted) - 1)
    for index, (start, end) in enumerate(zip(fitted, fitted[1:])):
        color_index = min(len(TRACE_COLORS) - 1, index * len(TRACE_COLORS) // denominator)
        raster.line(start, end, TRACE_COLORS[color_index], width=2.6, alpha=0.70)
    return transform


def render_oblique(path: Path, states: list[tuple[float, float, float]]) -> None:
    output_width, output_height = 1200, 820
    width = output_width * SUPERSAMPLE
    height = output_height * SUPERSAMPLE
    raster = Raster(width, height)
    points = [oblique(state) for state in states]
    transform = draw_panel(raster, points, (0, 0, width - 1, height - 1))

    origin = transform(oblique((0.0, 0.0, 0.0)))
    endpoints = (
        transform(oblique((2.0, 0.0, 0.0))),
        transform(oblique((0.0, 0.34, 0.0))),
        transform(oblique((0.0, 0.0, 3.2))),
    )
    for endpoint, color in zip(endpoints, AXIS_COLORS):
        raster.arrow(origin, endpoint, color)

    final_width, final_height, output = raster.downsample(SUPERSAMPLE)
    write_png(path, final_width, final_height, output)


def render_projections(path: Path, states: list[tuple[float, float, float]]) -> None:
    output_width, output_height = 1800, 560
    width = output_width * SUPERSAMPLE
    height = output_height * SUPERSAMPLE
    raster = Raster(width, height)
    gap = 18 * SUPERSAMPLE
    panel_width = (width - 2 * gap) // 3
    panels = (
        [project(state, 0, 1) for state in states],
        [project(state, 0, 2) for state in states],
        [project(state, 1, 2) for state in states],
    )
    for index, points in enumerate(panels):
        left = index * (panel_width + gap)
        right = left + panel_width - 1
        draw_panel(raster, points, (left, 0, right, height - 1))

    final_width, final_height, output = raster.downsample(SUPERSAMPLE)
    write_png(path, final_width, final_height, output)


def write_figures(output_directory: Path) -> None:
    output_directory.mkdir(parents=True, exist_ok=True)
    states = trajectory()
    render_oblique(output_directory / "chua-state-space-3d.png", states)
    render_projections(output_directory / "chua-state-space-projections.png", states)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("output_directory", type=Path, help="directory that receives two PNG files")
    arguments = parser.parse_args()
    write_figures(arguments.output_directory)


if __name__ == "__main__":
    main()
