#!/usr/bin/env python3
"""Draws the first searches of each workload, to show what the access patterns look like.

The sequential and bit-reversal sequences are the same as in TreeBenchmark.cpp. The
random and Zipf sequences follow the same rules, but use Python's random generator,
so the exact keys differ from the benchmark's. The picture is only an illustration.

Usage, from the repository root (needs matplotlib):
    python3 tests/analysis/plot_workloads.py
"""

import random
from pathlib import Path

N = 255
SEARCHES = 2 * N
OUTPUT = Path(__file__).resolve().parent / "images" / "workloads.png"

SURFACE = "#fcfcfb"
SERIES = "#2a78d6"
TEXT_PRIMARY = "#0b0b0b"
TEXT_SECONDARY = "#52514e"
GRIDLINE = "#e1e0d9"
AXIS_LINE = "#c3c2b7"


def random_workload(n, count, generator):
    return [generator.randint(1, n) for _ in range(count)]


def sequential_workload(n, count):
    return [i % n + 1 for i in range(count)]


def bit_reversal_workload(n, count):
    bits = 0
    while (1 << bits) <= n:
        bits += 1
    cycle = []
    for i in range(1, 1 << bits):
        reversed_value = int(format(i, f"0{bits}b")[::-1], 2)
        if reversed_value <= n:
            cycle.append(reversed_value)
    return [cycle[i % len(cycle)] for i in range(count)]


def zipf_workload(n, count, generator):
    key_of_rank = list(range(1, n + 1))
    generator.shuffle(key_of_rank)
    return [key_of_rank[min(n, int(n ** generator.random())) - 1] for _ in range(count)]


def main():
    import matplotlib

    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    generator = random.Random(1)
    panels = [
        ("Random", "Every key is equally likely.", random_workload(N, SEARCHES, generator)),
        ("Sequential", "Keys in ascending order, starting over at the end.", sequential_workload(N, SEARCHES)),
        ("Bit-reversal", "Each search jumps to the opposite side of the tree.", bit_reversal_workload(N, SEARCHES)),
        ("Zipf", "A few keys repeat often, spread over the whole range.", zipf_workload(N, SEARCHES, generator)),
    ]

    fig, axes = plt.subplots(2, 2, figsize=(9, 6.4), dpi=150, sharex=True, sharey=True, layout="constrained")
    fig.get_layout_engine().set(w_pad=0.12, h_pad=0.12, hspace=0.08, wspace=0.05)
    fig.patch.set_facecolor(SURFACE)
    for ax, (title, subtitle, keys) in zip(axes.flat, panels):
        ax.set_facecolor(SURFACE)
        ax.scatter(range(1, len(keys) + 1), keys, s=7, color=SERIES, linewidths=0)
        ax.set_title(f"{title}\n", loc="left", fontsize=11.5, fontweight="bold", color=TEXT_PRIMARY)
        ax.text(0, 1.02, subtitle, transform=ax.transAxes, fontsize=8.5, color=TEXT_SECONDARY)
        ax.set_xlim(0, len(keys) + 1)
        ax.set_ylim(0, N + 1)
        for side in ("top", "right"):
            ax.spines[side].set_visible(False)
        for side in ("left", "bottom"):
            ax.spines[side].set_color(AXIS_LINE)
            ax.spines[side].set_linewidth(0.75)
        ax.tick_params(color=AXIS_LINE, labelcolor=TEXT_SECONDARY, labelsize=8.5)
        ax.grid(color=GRIDLINE, linewidth=0.75)
        ax.set_axisbelow(True)
    for ax in axes[1]:
        ax.set_xlabel("Search number", color=TEXT_SECONDARY, fontsize=9)
    for ax in axes[:, 0]:
        ax.set_ylabel("Key searched", color=TEXT_SECONDARY, fontsize=9)
    fig.suptitle(f"The first {SEARCHES} searches of each workload, with n = {N} keys", x=0.01, ha="left", fontsize=12.5, fontweight="bold", color=TEXT_PRIMARY)

    OUTPUT.parent.mkdir(exist_ok=True)
    fig.savefig(OUTPUT, facecolor=SURFACE)
    print(f"Saved {OUTPUT}")


if __name__ == "__main__":
    main()
