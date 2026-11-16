#!/usr/bin/env python3
"""Compares the speed of the tango tree against other binary search trees.

For each workload and number of keys, every selected tree runs the same searches
several times. The script measures the time and counts the node accesses, saves
the raw and the averaged results as CSV files, and draws two charts per workload.

Examples, from the repository root:
    python3 tests/analysis/compare_trees.py --against splay
    python3 tests/analysis/compare_trees.py --against rbt splay --runs 10
    python3 tests/analysis/compare_trees.py --against rbt --workloads random zipf --sizes 1023 65535 --queries 500000

"""

import argparse
import csv
import itertools
import statistics
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]

# Each tree keeps its color and marker whichever trees are selected, so charts stay comparable.
TREES = {
    "tango": {"label": "Tango tree", "color": "#2a78d6", "marker": "o"},
    "rbt": {"label": "Red-black tree", "color": "#eb6834", "marker": "s"},
    "splay": {"label": "Splay tree", "color": "#1baf7a", "marker": "D"},
}
WORKLOADS = {
    "random": "Uniformly random searches",
    "sequential": "Sequential scans",
    "bitreversal": "Bit-reversal searches",
    "zipf": "Zipf-distributed searches",
}
DEFAULT_SIZES = [1023, 4095, 16383, 65535, 262143, 1048575, 4194303, 16777215]

# What each chart shows. "avg" and "stdev" are columns of the summary.
METRICS = {
    "time": {"avg": "avg_search_ms", "stdev": "stdev_search_ms", "title": "search time", "end_label": "{:,.1f} ms"},
    "accesses": {"avg": "avg_accesses_per_search", "stdev": "stdev_accesses_per_search", "title": "node accesses", "end_label": "{:,.1f} nodes"},
}

# A recursive splay can go as deep as the number of keys, which does not fit in the default stack.
STACK_LIMIT_BYTES = 1 << 30

SURFACE = "#fcfcfb"
TEXT_PRIMARY = "#0b0b0b"
TEXT_SECONDARY = "#52514e"
GRIDLINE = "#e1e0d9"
AXIS_LINE = "#c3c2b7"


def positive_int(text):
    value = int(text)
    if value < 1:
        raise argparse.ArgumentTypeError("must be at least 1")
    return value


def parse_arguments():
    parser = argparse.ArgumentParser(
        description="Compares the speed of the tango tree against other trees and charts the results.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument("--against", nargs="+", choices=["rbt", "splay"], default=["rbt", "splay"], help="trees to compare the tango tree against")
    parser.add_argument("--workloads", nargs="+", choices=list(WORKLOADS), default=list(WORKLOADS), help="search patterns to run")
    parser.add_argument("--sizes", nargs="+", type=positive_int, default=DEFAULT_SIZES, metavar="N", help="numbers of keys to test")
    parser.add_argument("--queries", type=positive_int, default=100_000, help="searches per run")
    parser.add_argument("--runs", type=positive_int, default=5, help="times to repeat each input, to take the average")
    parser.add_argument("--output-dir", type=Path, default=REPO_ROOT / "tests" / "analysis" / "results", help="where to save the CSV files and charts")
    parser.add_argument("--build-dir", type=Path, default=REPO_ROOT / "build", help="the CMake build directory")
    return parser.parse_args()


def run_cmake(command):
    result = subprocess.run(command, capture_output=True, text=True)
    if result.returncode != 0:
        sys.exit(f"Command failed: {' '.join(command)}\n{result.stdout}{result.stderr}")


def build_benchmarks(build_dir):
    """Compiles the benchmark and returns the paths of its two versions: the one that measures time and the one that counts node accesses."""
    print("Building the benchmark...", flush=True)
    # Configuring is needed so that a build directory created before the benchmark existed learns about it.
    run_cmake(["cmake", "-S", str(REPO_ROOT), "-B", str(build_dir)])
    for target in ("TreeBenchmark", "TreeBenchmarkCounted"):
        run_cmake(["cmake", "--build", str(build_dir), "--target", target])
    directory = build_dir / "tests" / "analysis"
    return directory / "TreeBenchmark", directory / "TreeBenchmarkCounted"


def raise_stack_limit():
    """Raises the stack limit, which the benchmark processes started by this script inherit."""
    try:
        import resource
    except ImportError:
        return  # The module only exists on Unix-like systems.

    soft, hard = resource.getrlimit(resource.RLIMIT_STACK)
    target = STACK_LIMIT_BYTES if hard == resource.RLIM_INFINITY else min(hard, STACK_LIMIT_BYTES)
    if soft != resource.RLIM_INFINITY and soft < target:
        resource.setrlimit(resource.RLIMIT_STACK, (target, hard))


def run_once(binary, tree, workload, n, queries, seed):
    """Runs one measurement in its own process and returns (build_ms, search_ms, accesses)."""
    command = [str(binary), tree, workload, str(n), str(queries), str(seed)]
    result = subprocess.run(command, capture_output=True, text=True)
    if result.returncode != 0:
        sys.exit(f"Benchmark failed with exit code {result.returncode}: {' '.join(command)}\n{result.stderr}")

    build_ms, search_ms, found, accesses = result.stdout.split()
    if int(found) != queries:
        sys.exit(f"{tree} found only {found} of {queries} keys: {' '.join(command)}")
    return float(build_ms), float(search_ms), int(accesses)


def collect_measurements(timing_binary, counting_binary, trees, args):
    """Runs every input. The trees alternate within each run, so slow drifts in the machine affect them equally."""
    rows = []
    inputs = list(itertools.product(args.workloads, args.sizes))
    for step, (workload, n) in enumerate(inputs, start=1):
        print(f"[{step}/{len(inputs)}] {workload}, n={n:,}", flush=True)
        for run in range(1, args.runs + 1):
            for tree in trees:
                # The seed depends only on the run, so all the trees get the same searches.
                build_ms, search_ms, _ = run_once(timing_binary, tree, workload, n, args.queries, seed=run)
                _, _, accesses = run_once(counting_binary, tree, workload, n, args.queries, seed=run)
                rows.append({"tree": tree, "workload": workload, "n": n, "queries": args.queries, "run": run, "build_ms": build_ms, "search_ms": search_ms, "accesses": accesses})
    return rows


def summarize(rows):
    """Averages the runs of each (tree, workload, n) input."""
    groups = {}
    for row in rows:
        groups.setdefault((row["tree"], row["workload"], row["n"]), []).append(row)

    summary = []
    for (tree, workload, n), runs in groups.items():
        search_times = [run["search_ms"] for run in runs]
        average = statistics.mean(search_times)
        accesses_per_search = [run["accesses"] / run["queries"] for run in runs]
        summary.append({
            "tree": tree,
            "workload": workload,
            "n": n,
            "queries": runs[0]["queries"],
            "runs": len(runs),
            "avg_search_ms": average,
            "stdev_search_ms": statistics.stdev(search_times) if len(runs) > 1 else 0.0,
            "avg_us_per_search": average * 1000 / runs[0]["queries"],
            "avg_build_ms": statistics.mean(run["build_ms"] for run in runs),
            "avg_accesses_per_search": statistics.mean(accesses_per_search),
            "stdev_accesses_per_search": statistics.stdev(accesses_per_search) if len(runs) > 1 else 0.0,
        })
    return summary


def write_csv(path, rows):
    with open(path, "w", newline="") as file:
        writer = csv.DictWriter(file, fieldnames=list(rows[0]))
        writer.writeheader()
        for row in rows:
            writer.writerow({key: f"{value:.3f}" if isinstance(value, float) else value for key, value in row.items()})


def print_summary(summary, trees, args):
    lookup = {(row["tree"], row["workload"], row["n"]): row for row in summary}
    for workload in args.workloads:
        runs = f"{args.runs} {'run' if args.runs == 1 else 'runs'}"
        print(f"\n{WORKLOADS[workload]}: average ms for {args.queries:,} searches, and average node accesses per search, over {runs}")
        print(f"{'n':>12}" + "".join(f"{TREES[tree]['label'] + ' ms':>20}" for tree in trees) + "".join(f"{TREES[tree]['label'] + ' nodes':>23}" for tree in trees))
        for n in args.sizes:
            rows = [lookup[(tree, workload, n)] for tree in trees]
            print(f"{n:>12,}" + "".join(f"{row['avg_search_ms']:>20,.2f}" for row in rows) + "".join(f"{row['avg_accesses_per_search']:>23,.2f}" for row in rows))


def style_axes(ax):
    ax.set_facecolor(SURFACE)
    for side in ("top", "right"):
        ax.spines[side].set_visible(False)
    for side in ("left", "bottom"):
        ax.spines[side].set_color(AXIS_LINE)
        ax.spines[side].set_linewidth(0.75)
    ax.tick_params(which="both", color=AXIS_LINE, labelcolor=TEXT_SECONDARY, labelsize=9)
    ax.grid(axis="y", which="major", color=GRIDLINE, linewidth=0.75, linestyle="solid")
    ax.set_axisbelow(True)


def add_end_labels(fig, ax, ends, value_format):
    """Writes the name and time beside each line's last point, skipping labels that would overlap. The legend still identifies those lines."""
    fig.canvas.draw()
    heights = [ax.transData.transform((x, y))[1] for _, x, y in ends]
    minimum_gap = 15 * fig.dpi / 72
    for index, (label, x, y) in enumerate(ends):
        if all(abs(heights[index] - other) >= minimum_gap for position, other in enumerate(heights) if position != index):
            ax.annotate(f"{label}\n{value_format.format(y)}", (x, y), xytext=(9, 0), textcoords="offset points", va="center", fontsize=8.5, color=TEXT_PRIMARY)


def axis_label(metric, use_log_scale, queries):
    if metric == "time":
        return f"Time for {queries:,} searches (ms{', log scale' if use_log_scale else ''})"
    return f"Node accesses per search{' (log scale)' if use_log_scale else ''}"


def draw_chart(plt, ticker, summary, trees, workload, metric, args):
    """Draws one measure of one workload, for each tree and number of keys."""
    average_column = METRICS[metric]["avg"]
    stdev_column = METRICS[metric]["stdev"]
    fig, ax = plt.subplots(figsize=(8.5, 5), dpi=150, layout="constrained")
    fig.get_layout_engine().set(w_pad=0.12, h_pad=0.14)
    fig.patch.set_facecolor(SURFACE)
    style_axes(ax)

    values = []
    tops = []
    ends = []
    for tree in trees:
        style = TREES[tree]
        points = sorted((row for row in summary if row["tree"] == tree and row["workload"] == workload), key=lambda row: row["n"])
        sizes = [row["n"] for row in points]
        averages = [row[average_column] for row in points]
        # The lower edge stays positive so that it can be drawn on a log scale.
        lower = [max(row[average_column] - row[stdev_column], row[average_column] * 0.1) for row in points]
        upper = [row[average_column] + row[stdev_column] for row in points]
        ax.fill_between(sizes, lower, upper, color=style["color"], alpha=0.16, linewidth=0)
        ax.plot(sizes, averages, color=style["color"], marker=style["marker"], label=style["label"], linewidth=1.6, markersize=7, markeredgecolor=SURFACE, markeredgewidth=1.2, solid_capstyle="round", solid_joinstyle="round")
        values += averages
        tops += upper
        ends.append((style["label"], sizes[-1], averages[-1]))

    ax.set_xscale("log", base=2)
    sizes = sorted(args.sizes)
    ax.set_xticks(sizes, [f"{n:,}" for n in sizes])
    ax.xaxis.set_minor_locator(ticker.NullLocator())
    # Leaves room on the right of the last points for their labels.
    if len(sizes) > 1:
        span = sizes[-1] / sizes[0]
        ax.set_xlim(sizes[0] / span ** 0.04, sizes[-1] * span ** 0.2)
    else:
        ax.set_xlim(sizes[0] / 2, sizes[0] * 2)

    # The trees can differ by orders of magnitude. A log scale keeps the fastest one readable.
    use_log_scale = max(values) / min(values) >= 10
    if use_log_scale:
        ax.set_yscale("log")
        ax.yaxis.set_major_locator(ticker.LogLocator(base=10, subs=(1, 2, 5)))
        ax.yaxis.set_major_formatter(ticker.FuncFormatter(lambda value, _: f"{value:,.10g}"))
        ax.yaxis.set_minor_formatter(ticker.NullFormatter())
    else:
        ax.set_ylim(0, max(tops) * 1.12)

    ax.set_xlabel("Number of keys (n)", color=TEXT_SECONDARY, fontsize=9.5)
    ax.set_ylabel(axis_label(metric, use_log_scale, args.queries), color=TEXT_SECONDARY, fontsize=9.5)
    ax.annotate(f"{WORKLOADS[workload]}: {METRICS[metric]['title']}", (0, 1), xycoords="axes fraction", xytext=(0, 48), textcoords="offset points", fontsize=13, fontweight="bold", color=TEXT_PRIMARY)
    note = f"Average of {args.runs} runs. The shaded area shows one standard deviation." if args.runs > 1 else "A single run, with no average."
    ax.annotate(note, (0, 1), xycoords="axes fraction", xytext=(0, 31), textcoords="offset points", fontsize=9, color=TEXT_SECONDARY)
    legend = ax.legend(loc="lower left", bbox_to_anchor=(0, 1.0), ncols=len(trees), frameon=False, fontsize=9, borderaxespad=0, handlelength=2.2, columnspacing=1.8, borderpad=0.2)
    for text in legend.get_texts():
        text.set_color(TEXT_PRIMARY)

    add_end_labels(fig, ax, ends, METRICS[metric]["end_label"])

    path = args.output_dir / f"{workload}_{metric}.png"
    fig.savefig(path, facecolor=SURFACE)
    plt.close(fig)
    return path


def draw_charts(summary, trees, args):
    """Draws the charts of every workload and returns their paths, or nothing if matplotlib is not installed."""
    try:
        import matplotlib
    except ImportError:
        print("\nCharts skipped: matplotlib is not installed. Install it with 'sudo apt install python3-matplotlib', or with 'pip install matplotlib' inside a virtual environment, then run this script again.")
        return []

    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    from matplotlib import ticker

    return [draw_chart(plt, ticker, summary, trees, workload, metric, args) for workload in args.workloads for metric in METRICS]


def main():
    args = parse_arguments()
    args.sizes = sorted(set(args.sizes))
    # Follows the order of TREES, so tables and charts always list the trees the same way.
    trees = [tree for tree in TREES if tree == "tango" or tree in args.against]

    timing_binary, counting_binary = build_benchmarks(args.build_dir.resolve())
    raise_stack_limit()
    rows = collect_measurements(timing_binary, counting_binary, trees, args)
    summary = summarize(rows)

    args.output_dir.mkdir(parents=True, exist_ok=True)
    write_csv(args.output_dir / "runs.csv", rows)
    write_csv(args.output_dir / "summary.csv", summary)
    print_summary(summary, trees, args)
    charts = draw_charts(summary, trees, args)

    print(f"\nResults saved in {args.output_dir}:")
    for name in ["runs.csv", "summary.csv"] + [chart.name for chart in charts]:
        print(f"  {name}")


if __name__ == "__main__":
    main()
