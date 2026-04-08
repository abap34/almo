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


def main() -> int:
    args = parse_args()

    test_log = read_text(args.test_log)
    coverage_log = read_text(args.coverage_log)
    benchmarks = parse_benchmarks(args.benchmark_json)

    passed, failed = parse_test_summary(test_log)
    coverage_summary = parse_coverage_summary(coverage_log)

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

    print(f"Benchmark history: {args.pages_url}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
