#!/usr/bin/env python3
"""Runs the benchmark for one number of keys and saves the results, so a long study can be split into pieces.

Each run is saved to disk as soon as it finishes. If the script stops early (a crash, a power cut, a lost
connection), running the same command again skips the runs already saved and continues from there.
When every run of a workload is done, the script also saves its averages. Afterwards, merge_results.py
joins the averages of all the sizes into charts and a Markdown table.

Files written inside --output-dir:
    runs/<workload>_n<N>.csv        every run, added as it finishes
    summaries/<workload>_n<N>.csv   the averages, written once all the runs are done

Running again with a higher --runs adds the missing runs. Running again with other trees adds those trees.
The number of queries must stay the same for a given workload and size.

Examples, from the repository root:
    python3 tests/analysis/run_size.py --size 1048575
    python3 tests/analysis/run_size.py --size 16777215 --against rbt --workloads random zipf --runs 10
    for n in 1023 4095 16383 65535; do python3 tests/analysis/run_size.py --size $n || break; done
"""

import argparse
import csv
import os
import sys
from pathlib import Path

from compare_trees import REPO_ROOT, TREES, WORKLOADS, build_benchmarks, positive_int, raise_stack_limit, run_once, summarize, write_csv

DEFAULT_OUTPUT_DIR = REPO_ROOT / "tests" / "analysis" / "results" / "by_size"
RUN_FIELDS = ["tree", "workload", "n", "queries", "run", "build_ms", "search_ms", "accesses"]


def parse_arguments():
    parser = argparse.ArgumentParser(
        description="Runs the benchmark for one number of keys, saving each run as it finishes so that an interrupted study can be resumed.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument("--size", type=positive_int, required=True, metavar="N", help="number of keys to test")
    parser.add_argument("--against", nargs="+", choices=["rbt", "splay"], default=["rbt", "splay"], help="trees to compare the tango tree against")
    parser.add_argument("--workloads", nargs="+", choices=list(WORKLOADS), default=list(WORKLOADS), help="search patterns to run")
    parser.add_argument("--queries", type=positive_int, default=100_000, help="searches per run")
    parser.add_argument("--runs", type=positive_int, default=10, help="times to repeat each input, to take the average")
    parser.add_argument("--output-dir", type=Path, default=DEFAULT_OUTPUT_DIR, help="where to save the CSV files")
    parser.add_argument("--build-dir", type=Path, default=REPO_ROOT / "build", help="the CMake build directory")
    parser.add_argument("--no-build", action="store_true", help="use the benchmark already built in --build-dir, for example when several copies of this script run at once")
    return parser.parse_args()


def parse_run(raw):
    """Converts one CSV row to typed values. Raises ValueError or TypeError if the row is damaged."""
    if None in raw or any(raw[field] in (None, "") for field in RUN_FIELDS):
        raise ValueError("incomplete row")
    row = {
        "tree": raw["tree"],
        "workload": raw["workload"],
        "n": int(raw["n"]),
        "queries": int(raw["queries"]),
        "run": int(raw["run"]),
        "build_ms": float(raw["build_ms"]),
        "search_ms": float(raw["search_ms"]),
        "accesses": int(raw["accesses"]),
    }
    if row["tree"] not in TREES or row["workload"] not in WORKLOADS:
        raise ValueError("unknown tree or workload")
    return row


def write_runs_atomically(path, rows):
    """Replaces the file in one step, so a failure during the write cannot leave it half written."""
    temporary = path.with_suffix(".tmp")
    with open(temporary, "w", newline="") as file:
        writer = csv.DictWriter(file, fieldnames=RUN_FIELDS)
        writer.writeheader()
        for row in rows:
            writer.writerow({key: f"{value:.3f}" if isinstance(value, float) else value for key, value in row.items()})
        file.flush()
        os.fsync(file.fileno())
    os.replace(temporary, path)


def load_saved_runs(path, workload, n, queries):
    """Reads the runs saved by earlier attempts. Drops any line damaged by an interruption and rewrites the file without it."""
    if not path.exists():
        return []

    text = path.read_text(errors="replace")
    # A line without its final newline was cut off while being written.
    lines = text.split("\n")[:-1]
    rows = []
    damaged = len(text.split("\n")[-1].strip("\0 \r")) > 0
    if lines and lines[0].strip() == ",".join(RUN_FIELDS):
        seen = set()
        for raw in csv.DictReader(lines):
            try:
                row = parse_run(raw)
            except (ValueError, TypeError):
                damaged = True
                continue
            if row["workload"] != workload or row["n"] != n:
                sys.exit(f"{path} holds results of another input ({row['workload']}, n={row['n']}). Move it away and run again.")
            if row["queries"] != queries:
                sys.exit(f"{path} holds runs with {row['queries']:,} queries, but this command asks for {queries:,}. "
                         f"Use --queries {row['queries']}, or a different --output-dir.")
            if (row["tree"], row["run"]) in seen:
                continue
            seen.add((row["tree"], row["run"]))
            rows.append(row)
    else:
        damaged = damaged or bool(text.strip("\0 \r\n"))

    if damaged:
        print(f"  {path.name}: dropped damaged lines left by an interruption, kept {len(rows)} complete runs", flush=True)
        write_runs_atomically(path, rows)
    return rows


def append_run(path, row):
    """Adds one run to the file and forces it to disk, so it survives a power cut."""
    is_new = not path.exists() or path.stat().st_size == 0
    with open(path, "a", newline="") as file:
        writer = csv.DictWriter(file, fieldnames=RUN_FIELDS)
        if is_new:
            writer.writeheader()
        writer.writerow({key: f"{value:.3f}" if isinstance(value, float) else value for key, value in row.items()})
        file.flush()
        os.fsync(file.fileno())


def write_summary(path, rows, runs):
    """Averages runs 1 to `runs` of every tree that has all of them, and saves the averages."""
    complete = []
    for tree in TREES:
        tree_rows = sorted((row for row in rows if row["tree"] == tree and row["run"] <= runs), key=lambda row: row["run"])
        if len(tree_rows) == runs:
            complete += tree_rows
    temporary = path.with_suffix(".tmp")
    write_csv(temporary, summarize(complete))
    os.replace(temporary, path)
    return sorted({row["tree"] for row in complete}, key=list(TREES).index)


def run_workload(timing_binary, counting_binary, trees, workload, args):
    n = args.size
    name = f"{workload}_n{n}.csv"
    runs_path = args.output_dir / "runs" / name
    summary_path = args.output_dir / "summaries" / name

    rows = load_saved_runs(runs_path, workload, n, args.queries)
    done = {(row["tree"], row["run"]) for row in rows}
    pending = [(run, tree) for run in range(1, args.runs + 1) for tree in trees if (tree, run) not in done]
    total = args.runs * len(trees)
    print(f"{WORKLOADS[workload]}, n={n:,}: {total - len(pending)} of {total} runs already saved", flush=True)

    for run, tree in pending:
        print(f"  run {run}/{args.runs}, {TREES[tree]['label']}", flush=True)
        # The seed depends only on the run, so all the trees get the same searches.
        build_ms, search_ms, _ = run_once(timing_binary, tree, workload, n, args.queries, seed=run)
        _, _, accesses = run_once(counting_binary, tree, workload, n, args.queries, seed=run)
        row = {"tree": tree, "workload": workload, "n": n, "queries": args.queries, "run": run, "build_ms": build_ms, "search_ms": search_ms, "accesses": accesses}
        append_run(runs_path, row)
        rows.append(row)

    summarized = write_summary(summary_path, rows, args.runs)
    print(f"  averages saved in {summary_path} for: {', '.join(TREES[tree]['label'] for tree in summarized)}", flush=True)


def main():
    args = parse_arguments()
    args.output_dir = args.output_dir.resolve()
    # Follows the order of TREES, so the files always list the trees the same way.
    trees = [tree for tree in TREES if tree == "tango" or tree in args.against]

    build_dir = args.build_dir.resolve()
    if args.no_build:
        directory = build_dir / "tests" / "analysis"
        timing_binary, counting_binary = directory / "TreeBenchmark", directory / "TreeBenchmarkCounted"
        for binary in (timing_binary, counting_binary):
            if not binary.exists():
                sys.exit(f"{binary} does not exist. Run once without --no-build to compile it.")
    else:
        timing_binary, counting_binary = build_benchmarks(build_dir)
    raise_stack_limit()

    (args.output_dir / "runs").mkdir(parents=True, exist_ok=True)
    (args.output_dir / "summaries").mkdir(parents=True, exist_ok=True)
    for workload in args.workloads:
        run_workload(timing_binary, counting_binary, trees, workload, args)
    print(f"\nDone. Join the sizes with: python3 tests/analysis/merge_results.py --input-dir {args.output_dir}")


if __name__ == "__main__":
    main()
