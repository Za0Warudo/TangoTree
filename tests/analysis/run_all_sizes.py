#!/usr/bin/env python3
"""Runs run_size.py for every size and workload in the settings below, several at a time, and logs everything to files.

Every (size, workload) pair runs as its own job, with its own result files, so jobs never write to the same file.
Pairs that already have all their averages saved are skipped. After an interruption (a power cut, a crash, a reboot),
just start this script again. An unfinished pair continues from its last saved run.

Start it in the background, from anywhere:
    nohup python3 tests/analysis/run_all_sizes.py > /dev/null 2>&1 &
See what is done, what is running and what is missing, at any time:
    python3 tests/analysis/run_all_sizes.py --status
Follow the main log, or the output of one job:
    tail -f tests/analysis/results/by_size/run_all_sizes.log
    tail -f tests/analysis/results/by_size/logs/random_n1073741823.log

The main log ends with "ALL DONE" when everything finished, "FINISHED WITH ... FAILED" when some pairs failed,
or "STOPPED" when the script was killed. If it ends with none of these and --status says nothing is running,
the machine went down in the middle. In every case, start the script again to continue.
"""

import csv
import fcntl
import os
import signal
import subprocess
import sys
import time
from datetime import datetime
from pathlib import Path

ANALYSIS_DIR = Path(__file__).resolve().parent
sys.path.insert(0, str(ANALYSIS_DIR))
from compare_trees import REPO_ROOT, TREES, build_benchmarks  # noqa: E402

# ----------------------------------------------------------------------------------------------------------------------
# Settings
# ----------------------------------------------------------------------------------------------------------------------
# The code supports at most 2^30 - 1 keys (see RESOURCE_ESTIMATES.md).
SIZES = [
    1023, 4095, 16383, 65535, 262143, 1048575, 4194303, 16777215,     # 2^10 - 1 to 2^24 - 1
    33554431, 67108863, 134217727, 268435455, 536870911, 1073741823,  # 2^25 - 1 to 2^30 - 1
]
WORKLOADS = ["random", "sequential", "bitreversal", "zipf"]
AGAINST = ["rbt", "splay"]
QUERIES = 100_000
RUNS = 10
# Fewer runs for the largest sizes, where one run of one workload takes hours. Sizes not listed use RUNS.
RUNS_BY_SIZE = {
    268435455: 5,
    536870911: 3,
    1073741823: 3,
}

# How many jobs run at the same time. Each job uses one core. Parallel jobs share the memory and the caches, so their
# times are a little slower than alone, but the node access counts are exact either way. Keep it at or below the
# number of physical cores (16), so that no two jobs share a core.
JOBS = 12
# The most memory, in GB, that the running jobs may use together. Leave room for other users of the machine.
MEMORY_BUDGET_GB = 300
# Gives each job its own physical core, so the operating system does not move it around or pair it with another job.
PIN_TO_CORES = True

OUTPUT_DIR = REPO_ROOT / "tests" / "analysis" / "results" / "by_size"
BUILD_DIR = REPO_ROOT / "build"
# ----------------------------------------------------------------------------------------------------------------------

LOG_FILE = OUTPUT_DIR / "run_all_sizes.log"
JOB_LOG_DIR = OUTPUT_DIR / "logs"
LOCK_FILE = OUTPUT_DIR / "run_all_sizes.lock"
SELECTED_TREES = [tree for tree in TREES if tree == "tango" or tree in AGAINST]
POLL_SECONDS = 5


def runs_for(n):
    return RUNS_BY_SIZE.get(n, RUNS)


def estimated_memory_bytes(n):
    """Peak memory of one benchmark process (RESOURCE_ESTIMATES.md), with 30% headroom."""
    return int(1.3 * (53 * n + 4 * QUERIES))


def gigabytes(value):
    return f"{value / 1e9:,.1f} GB"


def duration(seconds):
    seconds = int(seconds)
    return f"{seconds // 3600}h {seconds % 3600 // 60:02d}m {seconds % 60:02d}s"


def log(message):
    print(f"[{datetime.now():%Y-%m-%d %H:%M:%S}] {message}", flush=True)


def is_done(n, workload):
    """A pair is done when its averages file covers every selected tree with enough runs and the same queries."""
    try:
        with open(OUTPUT_DIR / "summaries" / f"{workload}_n{n}.csv", newline="") as file:
            rows = list(csv.DictReader(file))
        complete = {row["tree"] for row in rows if int(row["runs"]) >= runs_for(n) and int(row["queries"]) == QUERIES}
    except (OSError, KeyError, ValueError):
        return False
    return set(SELECTED_TREES) <= complete


def saved_runs(n, workload):
    """Counts the runs already saved for a pair, for the status report."""
    count = 0
    try:
        with open(OUTPUT_DIR / "runs" / f"{workload}_n{n}.csv", newline="") as file:
            for row in csv.DictReader(file):
                try:
                    if row["tree"] in SELECTED_TREES and int(row["run"]) <= runs_for(n) and int(row["queries"]) == QUERIES:
                        count += 1
                except (KeyError, TypeError, ValueError):
                    pass
    except OSError:
        pass
    return count


def available_memory_bytes():
    """The memory the system can still give out, from /proc/meminfo, or None if unknown."""
    try:
        for line in Path("/proc/meminfo").read_text().splitlines():
            if line.startswith("MemAvailable:"):
                return int(line.split()[1]) * 1024
    except OSError:
        pass
    return None


def physical_cores():
    """Returns one logical CPU per physical core, among the CPUs this process may use."""
    chosen, seen = [], set()
    for cpu in sorted(os.sched_getaffinity(0)):
        try:
            siblings = Path(f"/sys/devices/system/cpu/cpu{cpu}/topology/thread_siblings_list").read_text().strip()
        except OSError:
            siblings = str(cpu)
        if siblings not in seen:
            seen.add(siblings)
            chosen.append(cpu)
    return chosen


def is_running():
    """Tells whether a copy of this script holds the lock, which the system frees as soon as that copy dies."""
    try:
        with open(LOCK_FILE, "a") as file:
            fcntl.flock(file, fcntl.LOCK_EX | fcntl.LOCK_NB)
            fcntl.flock(file, fcntl.LOCK_UN)
            return False
    except BlockingIOError:
        return True
    except OSError:
        return False


def print_status():
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    print(f"Runner: {'RUNNING' if is_running() else 'not running'}")
    print(f"Results: {OUTPUT_DIR}\n")
    print(f"{'n':>15}  {'workload':<12} {'status':<9} {'saved runs':>12}  {'memory per job':>14}")
    totals = {"done": 0, "partial": 0, "missing": 0}
    for n in SIZES:
        for workload in WORKLOADS:
            expected = runs_for(n) * len(SELECTED_TREES)
            saved = min(saved_runs(n, workload), expected)
            state = "done" if is_done(n, workload) else "partial" if saved else "missing"
            totals[state] += 1
            print(f"{n:>15,}  {workload:<12} {state:<9} {saved:>5} / {expected:<4}  {gigabytes(estimated_memory_bytes(n)):>14}")
    print(f"\n{totals['done']} done, {totals['partial']} partial, {totals['missing']} missing.")
    print("A partial pair is one that was interrupted or is running now. It continues from its last saved run.")


class Runner:
    def __init__(self):
        self.running = {}  # (n, workload) -> dict with the process, its core, memory and start time.
        self.cores = []
        self.failed = []

    def start_job(self, n, workload):
        cpu = self.cores.pop(0) if self.cores else None
        JOB_LOG_DIR.mkdir(parents=True, exist_ok=True)
        job_log = open(JOB_LOG_DIR / f"{workload}_n{n}.log", "a")
        job_log.write(f"\n===== {datetime.now():%Y-%m-%d %H:%M:%S} start{f' on cpu {cpu}' if cpu is not None else ''} =====\n")
        job_log.flush()
        command = [
            sys.executable, "-u", str(ANALYSIS_DIR / "run_size.py"),
            "--size", str(n), "--workloads", workload, "--against", *AGAINST,
            "--queries", str(QUERIES), "--runs", str(runs_for(n)),
            "--output-dir", str(OUTPUT_DIR), "--build-dir", str(BUILD_DIR), "--no-build",
        ]
        process = subprocess.Popen(
            command, stdout=job_log, stderr=subprocess.STDOUT, stdin=subprocess.DEVNULL,
            preexec_fn=(lambda: os.sched_setaffinity(0, {cpu})) if cpu is not None else None,
        )
        job_log.close()
        self.running[(n, workload)] = {"process": process, "cpu": cpu, "memory": estimated_memory_bytes(n), "start": time.monotonic()}
        where = f" on cpu {cpu}" if cpu is not None else ""
        log(f"BEGIN n={n:,} {workload}{where} ({runs_for(n)} runs, about {gigabytes(estimated_memory_bytes(n))})")

    def finish_jobs(self):
        for (n, workload), job in list(self.running.items()):
            code = job["process"].poll()
            if code is None:
                continue
            del self.running[(n, workload)]
            if job["cpu"] is not None:
                self.cores.append(job["cpu"])
            elapsed = duration(time.monotonic() - job["start"])
            if code == 0 and is_done(n, workload):
                log(f"DONE n={n:,} {workload} in {elapsed}")
            else:
                self.failed.append((n, workload))
                log(f"FAILED n={n:,} {workload} after {elapsed} (exit code {code}). See logs/{workload}_n{n}.log")

    def stop(self, signum, _frame):
        log(f"STOPPED by signal {signal.Signals(signum).name}. Stopping {len(self.running)} jobs; their saved runs are kept.")
        for job in self.running.values():
            job["process"].terminate()
        for job in self.running.values():
            job["process"].wait()
        sys.exit(130)

    def run(self):
        budget = MEMORY_BUDGET_GB * 1e9
        pending = [(n, workload) for n in sorted(SIZES, reverse=True) for workload in WORKLOADS]
        for n, workload in list(pending):
            if is_done(n, workload):
                log(f"SKIP n={n:,} {workload}: already done")
                pending.remove((n, workload))
            elif estimated_memory_bytes(n) > budget:
                log(f"SKIP n={n:,} {workload}: needs about {gigabytes(estimated_memory_bytes(n))}, more than MEMORY_BUDGET_GB")
                self.failed.append((n, workload))
                pending.remove((n, workload))

        if PIN_TO_CORES:
            self.cores = physical_cores()
            if len(self.cores) < JOBS:
                log(f"Only {len(self.cores)} physical cores for {JOBS} jobs, so the jobs are not pinned.")
                self.cores = []

        # The largest pairs start first, since they take the longest. Smaller ones fill the remaining cores and memory.
        waiting_for_memory = False
        while pending or self.running:
            self.finish_jobs()
            used = sum(job["memory"] for job in self.running.values())
            available = available_memory_bytes()
            for pair in list(pending):
                if len(self.running) >= JOBS:
                    break
                need = estimated_memory_bytes(pair[0])
                if used + need > budget or (available is not None and need > available):
                    continue
                pending.remove(pair)
                self.start_job(*pair)
                used += need
                if available is not None:
                    available -= need
            stuck = pending and not self.running
            if stuck and not waiting_for_memory:
                log(f"WAITING: the machine has only {gigabytes(available or 0)} free, not enough for the next job. Retrying until it frees up.")
            waiting_for_memory = bool(stuck)
            time.sleep(POLL_SECONDS)


def main():
    if "--status" in sys.argv[1:]:
        print_status()
        return
    if sys.argv[1:]:
        sys.exit(f"Usage: {sys.argv[0]} [--status]. The settings are at the top of the script.")

    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    # Everything this script prints, including errors, goes to the main log.
    log_fd = os.open(LOG_FILE, os.O_WRONLY | os.O_CREAT | os.O_APPEND, 0o644)
    os.dup2(log_fd, 1)
    os.dup2(log_fd, 2)

    lock = open(LOCK_FILE, "a")
    try:
        fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
    except BlockingIOError:
        log("NOT STARTED: another copy is already running.")
        sys.exit(1)

    runner = Runner()
    for signum in (signal.SIGINT, signal.SIGTERM, signal.SIGHUP):
        signal.signal(signum, runner.stop)

    log(f"========== START (pid {os.getpid()}, host {os.uname().nodename}) ==========")
    log(f"sizes: {', '.join(f'{n:,}' for n in SIZES)}")
    log(f"workloads: {' '.join(WORKLOADS)} | trees: {' '.join(SELECTED_TREES)} | queries: {QUERIES:,} | runs: {RUNS}, by size: {RUNS_BY_SIZE}")
    log(f"jobs: {JOBS} | memory budget: {MEMORY_BUDGET_GB} GB | pinned: {PIN_TO_CORES}")
    build_benchmarks(BUILD_DIR)
    runner.run()

    if runner.failed:
        log(f"========== FINISHED WITH {len(runner.failed)} FAILED: {', '.join(f'n={n:,} {w}' for n, w in runner.failed)} ==========")
        log("Check their logs in logs/, then start this script again to retry them.")
        sys.exit(1)
    log("========== ALL DONE ==========")
    log(f"Join the results with: python3 {ANALYSIS_DIR / 'merge_results.py'} --input-dir {OUTPUT_DIR}")


if __name__ == "__main__":
    main()
