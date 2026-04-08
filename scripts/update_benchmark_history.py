#!/usr/bin/env python3

import argparse
import json
from pathlib import Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--benchmark-json", required=True)
    parser.add_argument("--history-json", required=True)
    parser.add_argument("--repo", required=True)
    parser.add_argument("--commit", required=True)
    parser.add_argument("--branch", required=True)
    parser.add_argument("--timestamp", required=True)
    parser.add_argument("--run-url", required=True)
    parser.add_argument("--max-entries", type=int, default=60)
    return parser.parse_args()


def main() -> int:
    args = parse_args()

    with Path(args.benchmark_json).open() as infile:
        benchmark_payload = json.load(infile)

    history_path = Path(args.history_json)
    if history_path.exists():
        with history_path.open() as infile:
            history_payload = json.load(infile)
    else:
        history_payload = {"repo": args.repo, "entries": []}

    entry = {
        "timestamp": args.timestamp,
        "commit": args.commit,
        "branch": args.branch,
        "run_url": args.run_url,
        "benchmarks": benchmark_payload.get("benchmarks", []),
    }

    entries = history_payload.get("entries", [])
    if entries and entries[-1]["commit"] == args.commit and entries[-1]["branch"] == args.branch:
        entries[-1] = entry
    else:
        entries.append(entry)

    history_payload["repo"] = args.repo
    history_payload["entries"] = entries[-args.max_entries :]

    history_path.parent.mkdir(parents=True, exist_ok=True)
    with history_path.open("w") as outfile:
        json.dump(history_payload, outfile, indent=2)
        outfile.write("\n")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
