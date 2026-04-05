#!/usr/bin/env python3
"""
Perf test suite for conv benchmark.
Runs each parameter set N times, appends results to CSV after every run,
then prints a summary table with averages.
 
Usage:
    python3 run_perf.py [--binary ./bin/conv] [--runs 3] [--csv results.csv]
"""
 
import subprocess
import re
import csv
import os
import sys
import argparse
from datetime import datetime
from collections import defaultdict
 
# ── parameter sets ──────────────────────────────────────────────────────────
PARAM_SETS = [
    "128 128 3 256 256",
    "128 128 3 256 256",
    "128 128 3 256 256",
    "256 256 3 128 256",
    "256 256 3 128 256",
    "256 256 3 128 256",
    "128 128 7 256 256",
    "128 128 7 256 256",
    "128 128 7 256 256",
]
 
CSV_HEADER = ["timestamp", "params", "single_us", "pthreads_us", "openmp_us"]
 
# ── regex patterns ───────────────────────────────────────────────────────────
RE_SINGLE   = re.compile(r"Single threaded conv time:\s*([\d]+)\s*microseconds")
RE_PTHREADS = re.compile(r"Student pthreads conv time:\s*([\d]+)\s*microseconds")
RE_OPENMP   = re.compile(r"Student openmp conv time:\s*([\d]+)\s*microseconds")
 
 
def parse_output(text: str) -> dict:
    s = RE_SINGLE.search(text)
    p = RE_PTHREADS.search(text)
    o = RE_OPENMP.search(text)
    return {
        "single_us":   int(s.group(1)) if s else None,
        "pthreads_us": int(p.group(1)) if p else None,
        "openmp_us":   int(o.group(1)) if o else None,
    }
 
 
def run_once(binary: str, params: str) -> dict:
    cmd = [binary] + params.split()
    print(f"  $ {' '.join(cmd)}")
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=600)
        output = result.stdout + result.stderr
        print(output.rstrip())
        parsed = parse_output(output)
        if any(v is None for v in parsed.values()):
            print("  [WARN] Could not parse one or more timing values from output above.")
        return parsed
    except subprocess.TimeoutExpired:
        print("  [ERROR] Run timed out after 600 s")
        return {"single_us": None, "pthreads_us": None, "openmp_us": None}
    except FileNotFoundError:
        print(f"  [ERROR] Binary not found: {binary}")
        sys.exit(1)
 
 
def append_csv(csv_path: str, row: dict, params: str):
    write_header = not os.path.exists(csv_path)
    with open(csv_path, "a", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=CSV_HEADER)
        if write_header:
            writer.writeheader()
        writer.writerow({
            "timestamp":   datetime.now().isoformat(timespec="seconds"),
            "params":      params,
            "single_us":   row["single_us"]   if row["single_us"]   is not None else "",
            "pthreads_us": row["pthreads_us"] if row["pthreads_us"] is not None else "",
            "openmp_us":   row["openmp_us"]   if row["openmp_us"]   is not None else "",
        })
 
 
def avg(values):
    clean = [v for v in values if v is not None]
    return sum(clean) / len(clean) if clean else None
 
 
def pct(part, whole):
    return f"{part / whole * 100:.1f}%" if part and whole else "N/A"
 
 
def print_table(summary: dict):
    col_w = [22, 22, 22, 22]
    headers = ["Parameters", "Single (μs)", "Pthreads (μs)", "OpenMP (μs)"]
 
    sep = "+" + "+".join("-" * w for w in col_w) + "+"
 
    def row(*cells):
        parts = []
        for cell, w in zip(cells, col_w):
            parts.append(f" {str(cell):<{w-2}} ")
        print("|" + "|".join(parts) + "|")
 
    print("\n" + sep)
    row(*headers)
    print(sep)
 
    # group by unique param string
    seen = {}
    for params, data in summary.items():
        key = params
        if key in seen:
            continue
        seen[key] = True
 
        s = data["single_us"]
        p = data["pthreads_us"]
        o = data["openmp_us"]
 
        s_str = f"{s:,.0f}" if s else "N/A"
        p_str = f"{p:,.0f} ({pct(p, s)})" if p else "N/A"
        o_str = f"{o:,.0f} ({pct(o, s)})" if o else "N/A"
 
        row(params, s_str, p_str, o_str)
 
    print(sep + "\n")
 
 
def main():
    parser = argparse.ArgumentParser(description="Conv perf test suite")
    parser.add_argument("--binary", default="./bin/conv", help="Path to the conv binary")
    parser.add_argument("--runs",   type=int, default=3,  help="Trials per unique param set (default 3)")
    parser.add_argument("--csv",    default="perf_results.csv", help="Output CSV path")
    args = parser.parse_args()
 
    # Collect unique param sets, preserving order
    unique_params = list(dict.fromkeys(PARAM_SETS))
 
    # Build the full trial list: unique_params × runs
    trials = []
    for p in unique_params:
        for _ in range(args.runs):
            trials.append(p)
 
    print(f"Binary : {args.binary}")
    print(f"CSV    : {args.csv}")
    print(f"Trials : {len(trials)}  ({len(unique_params)} param sets × {args.runs} runs)\n")
 
    # Storage: params → list of results
    raw: dict = defaultdict(lambda: {"single_us": [], "pthreads_us": [], "openmp_us": []})
 
    for i, params in enumerate(trials, 1):
        print(f"[{i}/{len(trials)}] params: {params}")
        result = run_once(args.binary, params)
        for k in ("single_us", "pthreads_us", "openmp_us"):
            raw[params][k].append(result[k])
        append_csv(args.csv, result, params)
        print()
 
    # Build averages
    summary = {}
    for params in unique_params:
        data = raw[params]
        summary[params] = {
            "single_us":   avg(data["single_us"]),
            "pthreads_us": avg(data["pthreads_us"]),
            "openmp_us":   avg(data["openmp_us"]),
        }
 
    print("=" * 90)
    print("AVERAGE RESULTS")
    print("=" * 90)
    print_table(summary)
 
    print(f"Raw results saved to: {args.csv}")
 
 
if __name__ == "__main__":
    main()