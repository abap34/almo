#!/usr/bin/env python3

import argparse
import json
import re
from pathlib import Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--test-status", required=True)
    parser.add_argument("--bench-status", required=True)
    parser.add_argument("--coverage-status", required=True)
    parser.add_argument("--test-log", required=True)
    parser.add_argument("--benchmark-json", required=True)
    parser.add_argument("--coverage-log", required=True)
    parser.add_argument("--pages-url", required=True)
    parser.add_argument("--event-name")
    parser.add_argument("--branch")
    parser.add_argument("--commit")
    parser.add_argument("--run-id", type=int)
    parser.add_argument("--run-url")
    parser.add_argument("--pr-number", type=int)
    parser.add_argument("--preview-url")
    parser.add_argument("--dashboard-json")
    parser.add_argument("--base-benchmark-json")
    parser.add_argument("--base-branch")
    return parser.parse_args()


def read_text(path: str) -> str:
    file_path = Path(path)
    if not file_path.exists():
        return ""
    return file_path.read_text()


def status_label(status: str) -> str:
    return {
        "success": "Success",
        "failure": "Failure",
        "cancelled": "Cancelled",
        "skipped": "Skipped",
    }.get(status, status)


def parse_test_summary(log: str) -> tuple[int, int]:
    passed = len(re.findall(r"^\[PASS\]", log, re.MULTILINE))
    failed = len(re.findall(r"^\[FAIL\]", log, re.MULTILINE))
    return passed, failed


def parse_coverage_summary(log: str) -> str:
    matches = re.findall(r"lines\.+: ([0-9.]+%) \(([^)]+)\)", log)
    if not matches:
        return "Unavailable"
    percent, detail = matches[-1]
    return f"{percent} ({detail})"


def parse_benchmarks(path: str) -> list[dict]:
    file_path = Path(path)
    if not file_path.exists():
        return []
    with file_path.open() as infile:
        payload = json.load(infile)
    return payload.get("benchmarks", [])


def benchmark_deltas(
    benchmarks: list[dict], base_benchmarks: list[dict]
) -> list[dict]:
    base_by_name = {benchmark["name"]: benchmark for benchmark in base_benchmarks}
    deltas = []

    for benchmark in benchmarks:
        base = base_by_name.get(benchmark["name"])
        if not base:
            continue

        base_mean = base.get("mean_ms")
        current_mean = benchmark.get("mean_ms")
        if not isinstance(base_mean, (int, float)) or not isinstance(
            current_mean, (int, float)
        ):
            continue
        if base_mean <= 0:
            continue

        delta_ms = current_mean - base_mean
        delta_pct = (delta_ms / base_mean) * 100.0

        if delta_pct <= -1.0:
            change_label = f"✅ {abs(delta_pct):.1f}% speed up"
        elif delta_pct >= 1.0:
            change_label = f"⚠️ {delta_pct:.1f}% slow down"
        else:
            change_label = f"➖ {abs(delta_pct):.1f}% change"

        deltas.append(
            {
                "name": benchmark["name"],
                "base_mean_ms": base_mean,
                "current_mean_ms": current_mean,
                "delta_ms": delta_ms,
                "delta_pct": delta_pct,
                "change_label": change_label,
            }
        )

    return deltas


def build_dashboard_entry(
    args: argparse.Namespace,
    passed: int,
    failed: int,
    coverage_summary: str,
    benchmarks: list[dict],
) -> dict:
    entry = {
        "test_status": args.test_status,
        "bench_status": args.bench_status,
        "coverage_status": args.coverage_status,
        "tests": {
            "passed": passed,
            "failed": failed,
        },
        "coverage_summary": coverage_summary,
        "benchmarks": benchmarks,
        "context": {
            "event_name": args.event_name,
            "branch": args.branch,
            "commit": args.commit,
            "run_id": args.run_id,
            "run_url": args.run_url,
            "pr_number": args.pr_number if args.pr_number and args.pr_number > 0 else None,
            "preview_url": args.preview_url,
        },
    }
    return entry


def write_dashboard_entry(path: str, payload: dict) -> None:
    file_path = Path(path)
    file_path.parent.mkdir(parents=True, exist_ok=True)
    with file_path.open("w") as outfile:
        json.dump(payload, outfile, indent=2)
        outfile.write("\n")


def main() -> int:
    args = parse_args()

    test_log = read_text(args.test_log)
    coverage_log = read_text(args.coverage_log)
    benchmarks = parse_benchmarks(args.benchmark_json)
    base_benchmarks = (
        parse_benchmarks(args.base_benchmark_json)
        if args.base_benchmark_json
        else []
    )
    deltas = benchmark_deltas(benchmarks, base_benchmarks)

    passed, failed = parse_test_summary(test_log)
    coverage_summary = parse_coverage_summary(coverage_log)
    dashboard_entry = build_dashboard_entry(
        args, passed, failed, coverage_summary, benchmarks
    )

    if args.dashboard_json:
        write_dashboard_entry(args.dashboard_json, dashboard_entry)

    print("## CI Summary")
    print()
    print("| Check | Status | Details |")
    print("| --- | --- | --- |")
    print(
        f"| Tests | {status_label(args.test_status)} | {passed} passed / {failed} failed |"
    )
    print(
        f"| Benchmarks | {status_label(args.bench_status)} | {len(benchmarks)} benchmark(s) captured |"
    )
    print(
        f"| Coverage | {status_label(args.coverage_status)} | {coverage_summary} |"
    )
    print()

    if benchmarks:
        print("### Benchmark Means")
        print()
        print("| Benchmark | Mean (ms) | Min (ms) | Max (ms) | Iterations |")
        print("| --- | ---: | ---: | ---: | ---: |")
        for benchmark in benchmarks:
            print(
                "| {name} | {mean_ms:.3f} | {min_ms:.3f} | {max_ms:.3f} | {iterations} |".format(
                    **benchmark
                )
            )
        print()

    if deltas:
        base_branch = args.base_branch or "base branch"
        print(f"### Benchmark Diff vs {base_branch}")
        print()
        print("| Benchmark | Base Mean (ms) | PR Mean (ms) | Delta (ms) | Change |")
        print("| --- | ---: | ---: | ---: | --- |")
        for delta in deltas:
            print(
                "| {name} | {base_mean_ms:.3f} | {current_mean_ms:.3f} | {delta_ms:+.3f} | {change_label} |".format(
                    **delta
                )
            )
        print()

    print(f"Benchmark dashboard: {args.pages_url}")
    if args.preview_url:
        print(f"PR preview: {args.preview_url}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
