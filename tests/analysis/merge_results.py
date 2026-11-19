#!/usr/bin/env python3
"""Joins the averages saved by run_size.py for each number of keys, and draws the charts and a Markdown table.

It reads every file in <input-dir>/summaries, so the sizes can come from separate runs, days or machines
(copy their summaries/ folders together). Sizes that are missing for a tree show as "-" in the table.

Files written inside --output-dir (by default the same as --input-dir):
    summary.csv                     the averages of every size, in one file
    results.md                      one table per workload, with the charts below it
    <workload>_time.png             search time per number of keys
    <workload>_accesses.png         node accesses per search per number of keys

Examples, from the repository root:
    python3 tests/analysis/merge_results.py
    python3 tests/analysis/merge_results.py --input-dir tests/analysis/results/by_size --workloads random zipf
"""

import argparse
import csv
import sys
from pathlib import Path

from compare_trees import METRICS, TREES, WORKLOADS, draw_chart, positive_int, write_csv
from run_size import DEFAULT_OUTPUT_DIR

INTEGER_COLUMNS = {"n", "queries", "runs"}
TEXT_COLUMNS = {"tree", "workload"}


def parse_arguments():
    parser = argparse.ArgumentParser(
        description="Joins the per-size results of run_size.py into charts and a Markdown table.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument("--input-dir", type=Path, default=DEFAULT_OUTPUT_DIR, help="the --output-dir given to run_size.py")
    parser.add_argument("--output-dir", type=Path, help="where to save the charts and tables (default: --input-dir)")
    parser.add_argument("--workloads", nargs="+", choices=list(WORKLOADS), help="workloads to include (default: all found)")
    parser.add_argument("--trees", nargs="+", choices=list(TREES), help="trees to include (default: all found)")
    parser.add_argument("--sizes", nargs="+", type=positive_int, metavar="N", help="numbers of keys to include (default: all found)")
    return parser.parse_args()


def load_summaries(directory):
    paths = sorted(directory.glob("*.csv"))
    if not paths:
        sys.exit(f"No results found in {directory}. Run run_size.py first.")

    rows = {}
    for path in paths:
        with open(path, newline="") as file:
            for raw in csv.DictReader(file):
                row = {key: value if key in TEXT_COLUMNS else int(value) if key in INTEGER_COLUMNS else float(value) for key, value in raw.items()}
                key = (row["tree"], row["workload"], row["n"])
                if key in rows:
                    sys.exit(f"{path} repeats the results of {key}, already read from another file.")
                rows[key] = row
    return list(rows.values())


def check_queries(summary, workload):
    """The time of different numbers of queries cannot share an axis, so a workload must use a single one."""
    queries = {row["queries"] for row in summary if row["workload"] == workload}
    if len(queries) > 1:
        sizes = ", ".join(f"n={row['n']:,}: {row['queries']:,}" for row in sorted(summary, key=lambda row: row["n"]) if row["workload"] == workload and row["tree"] == "tango")
        sys.exit(f"{WORKLOADS[workload]} mixes different numbers of queries ({sizes}). Rerun those sizes with the same --queries, or leave them out with --sizes.")
    return queries.pop()


def format_cell(row, average, stdev):
    if row is None:
        return "-"
    return f"{row[average]:,.2f} ± {row[stdev]:,.2f}"


def markdown_section(summary, workload, trees, queries, runs, charts):
    lookup = {(row["tree"], row["n"]): row for row in summary if row["workload"] == workload}
    sizes = sorted({n for _, n in lookup})
    runs_text = f"{min(runs)} runs" if len(runs) == 1 else f"{min(runs)} to {max(runs)} runs"

    lines = [f"## {WORKLOADS[workload]}", "", f"Average ± standard deviation over {runs_text} of {queries:,} searches each.", ""]
    for metric, title in (("time", f"Time for {queries:,} searches (ms)"), ("accesses", "Node accesses per search")):
        average, stdev = METRICS[metric]["avg"], METRICS[metric]["stdev"]
        lines.append(f"### {title}")
        lines.append("")
        lines.append("| n | " + " | ".join(TREES[tree]["label"] for tree in trees) + " |")
        lines.append("|--:|" + "--:|" * len(trees))
        for n in sizes:
            lines.append(f"| {n:,} | " + " | ".join(format_cell(lookup.get((tree, n)), average, stdev) for tree in trees) + " |")
        lines.append("")
    for chart in charts:
        lines.append(f"![{chart.stem}]({chart.name})")
    lines.append("")
    return lines


def main():
    args = parse_arguments()
    args.output_dir = (args.output_dir or args.input_dir).resolve()
    summary = load_summaries(args.input_dir.resolve() / "summaries")
    if args.workloads:
        summary = [row for row in summary if row["workload"] in args.workloads]
    if args.trees:
        summary = [row for row in summary if row["tree"] in args.trees]
    if args.sizes:
        summary = [row for row in summary if row["n"] in args.sizes]
    if not summary:
        sys.exit("No results match the chosen workloads, trees and sizes.")
    # Keeps the usual order of workloads, trees and sizes.
    summary.sort(key=lambda row: (list(WORKLOADS).index(row["workload"]), list(TREES).index(row["tree"]), row["n"]))

    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
        from matplotlib import ticker
    except ImportError:
        plt = None
        print("Charts skipped: matplotlib is not installed. Install it with 'sudo apt install python3-matplotlib', or with 'pip install matplotlib' inside a virtual environment, then run this script again.")

    args.output_dir.mkdir(parents=True, exist_ok=True)
    workloads = [workload for workload in WORKLOADS if any(row["workload"] == workload for row in summary)]
    markdown = ["# Tango tree comparison", ""]
    written = ["summary.csv", "results.md"]
    for workload in workloads:
        rows = [row for row in summary if row["workload"] == workload]
        queries = check_queries(summary, workload)
        trees = [tree for tree in TREES if any(row["tree"] == tree for row in rows)]
        runs = sorted({row["runs"] for row in rows})
        if len(runs) > 1:
            print(f"Note: {WORKLOADS[workload]} has sizes averaged over different numbers of runs ({', '.join(map(str, runs))}). The charts state the smallest.")

        charts = []
        if plt is not None:
            # draw_chart reads these settings from the command line arguments of compare_trees.py.
            chart_args = argparse.Namespace(runs=min(runs), queries=queries, sizes=sorted({row["n"] for row in rows}), output_dir=args.output_dir)
            charts = [draw_chart(plt, ticker, rows, trees, workload, metric, chart_args) for metric in METRICS]
            written += [chart.name for chart in charts]
        markdown += markdown_section(summary, workload, trees, queries, runs, charts)

    write_csv(args.output_dir / "summary.csv", summary)
    (args.output_dir / "results.md").write_text("\n".join(markdown))

    print(f"Joined {len({row['n'] for row in summary})} sizes. Results saved in {args.output_dir}:")
    for name in written:
        print(f"  {name}")


if __name__ == "__main__":
    main()
