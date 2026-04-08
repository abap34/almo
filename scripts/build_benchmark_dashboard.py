#!/usr/bin/env python3

import argparse
import io
import json
import shutil
import urllib.error
import urllib.parse
import urllib.request
import zipfile
from datetime import datetime, timezone
from pathlib import Path


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--repo", required=True)
    parser.add_argument("--workflow", required=True)
    parser.add_argument("--out-dir", required=True)
    parser.add_argument("--token")
    parser.add_argument("--artifact-name", default="dashboard-entry")
    parser.add_argument("--max-runs", type=int, default=40)
    parser.add_argument("--fixture-json")
    return parser.parse_args()


def github_request(url: str, token: str | None) -> dict:
    request = urllib.request.Request(url)
    request.add_header("Accept", "application/vnd.github+json")
    request.add_header("X-GitHub-Api-Version", "2022-11-28")
    if token:
        request.add_header("Authorization", f"Bearer {token}")

    with urllib.request.urlopen(request) as response:
        return json.load(response)


def download_artifact(url: str, token: str | None) -> bytes:
    request = urllib.request.Request(url)
    request.add_header("Accept", "application/vnd.github+json")
    request.add_header("X-GitHub-Api-Version", "2022-11-28")
    if token:
        request.add_header("Authorization", f"Bearer {token}")

    with urllib.request.urlopen(request) as response:
        return response.read()


def load_dashboard_entry(archive_bytes: bytes) -> dict:
    with zipfile.ZipFile(io.BytesIO(archive_bytes)) as archive:
        for member in archive.namelist():
            if member.endswith(".json"):
                with archive.open(member) as infile:
                    return json.load(infile)
    raise RuntimeError("dashboard-entry artifact does not contain a JSON file")


def normalize_entry(run: dict, summary: dict) -> dict:
    pull_requests = run.get("pull_requests", [])
    pr_number = summary.get("context", {}).get("pr_number")
    if pr_number is None and pull_requests:
        pr_number = pull_requests[0].get("number")

    return {
        "run_id": run["id"],
        "run_url": run["html_url"],
        "title": run.get("display_title") or run.get("name") or "",
        "event": run.get("event"),
        "branch": run.get("head_branch"),
        "commit": run.get("head_sha"),
        "created_at": run.get("created_at"),
        "updated_at": run.get("updated_at"),
        "conclusion": run.get("conclusion"),
        "pr_number": pr_number,
        "summary": {
            "test_status": summary.get("test_status"),
            "bench_status": summary.get("bench_status"),
            "coverage_status": summary.get("coverage_status"),
            "tests": summary.get("tests", {}),
            "coverage_summary": summary.get("coverage_summary"),
            "benchmarks": summary.get("benchmarks", []),
        },
    }


def fetch_dashboard_entries(args: argparse.Namespace) -> list[dict]:
    if args.fixture_json:
        with Path(args.fixture_json).open() as infile:
            payload = json.load(infile)
        return payload.get("entries", [])

    if not args.token:
        raise RuntimeError("--token is required unless --fixture-json is used")

    encoded_workflow = urllib.parse.quote(args.workflow, safe="")
    runs_url = (
        f"https://api.github.com/repos/{args.repo}/actions/workflows/"
        f"{encoded_workflow}/runs?per_page={args.max_runs}"
    )
    runs_payload = github_request(runs_url, args.token)

    entries = []
    for run in runs_payload.get("workflow_runs", []):
        if run.get("status") != "completed":
            continue

        artifacts_url = (
            f"https://api.github.com/repos/{args.repo}/actions/runs/"
            f"{run['id']}/artifacts?per_page=100"
        )
        artifacts_payload = github_request(artifacts_url, args.token)
        artifact = next(
            (
                item
                for item in artifacts_payload.get("artifacts", [])
                if item.get("name") == args.artifact_name and not item.get("expired")
            ),
            None,
        )
        if artifact is None:
            continue

        try:
            archive_bytes = download_artifact(
                artifact["archive_download_url"], args.token
            )
            summary = load_dashboard_entry(archive_bytes)
        except (urllib.error.HTTPError, RuntimeError, zipfile.BadZipFile):
            continue

        entries.append(normalize_entry(run, summary))

    entries.sort(key=lambda entry: entry.get("updated_at") or entry.get("created_at") or "")
    return entries


def write_redirect_index(path: Path) -> None:
    path.write_text(
        """<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <meta http-equiv="refresh" content="0; url=./dev/bench/" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>ALMO Benchmark Dashboard</title>
  </head>
  <body>
    <p><a href="./dev/bench/">Open the benchmark dashboard.</a></p>
  </body>
</html>
""",
        encoding="utf-8",
    )


def build_payload(repo: str, entries: list[dict]) -> dict:
    return {
        "repo": repo,
        "generated_at": datetime.now(timezone.utc).isoformat(),
        "entries": entries,
    }


def write_site(args: argparse.Namespace, payload: dict) -> None:
    out_dir = Path(args.out_dir)
    bench_dir = out_dir / "dev" / "bench"
    source_dir = Path("scripts/benchmark_pages")

    shutil.rmtree(out_dir, ignore_errors=True)
    bench_dir.mkdir(parents=True, exist_ok=True)

    shutil.copy2(source_dir / "index.html", bench_dir / "index.html")
    shutil.copy2(source_dir / "styles.css", bench_dir / "styles.css")
    shutil.copy2(source_dir / "app.js", bench_dir / "app.js")

    with (bench_dir / "dashboard.json").open("w") as outfile:
        json.dump(payload, outfile, indent=2)
        outfile.write("\n")

    write_redirect_index(out_dir / "index.html")
    (out_dir / ".nojekyll").write_text("", encoding="utf-8")


def main() -> int:
    args = parse_args()
    payload = build_payload(args.repo, fetch_dashboard_entries(args))
    write_site(args, payload)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
