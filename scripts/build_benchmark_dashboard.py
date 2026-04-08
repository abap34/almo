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
    parser.add_argument("--out-dir", required=True)
    parser.add_argument("--workflow")
    parser.add_argument("--token")
    parser.add_argument("--artifact-name", default="dashboard-entry")
    parser.add_argument("--coverage-artifact-name", default="coverage-report")
    parser.add_argument("--preview-artifact-name", default="preview-site")
    parser.add_argument("--max-runs", type=int, default=40)
    parser.add_argument("--fixture-json")
    parser.add_argument("--entry-json")
    parser.add_argument("--coverage-html-dir")
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


def load_local_dashboard_entry(path: str) -> dict:
    with Path(path).open() as infile:
        return json.load(infile)


def find_artifact_by_name(artifacts: list[dict], name: str) -> dict | None:
    return next(
        (
            artifact
            for artifact in artifacts
            if artifact.get("name") == name and not artifact.get("expired")
        ),
        None,
    )


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
            "coverage_url": None,
            "preview_url": None,
        },
    }


def normalize_local_entry(summary: dict) -> dict:
    context = summary.get("context", {})
    run_id = context.get("run_id") or 0
    pr_number = context.get("pr_number")
    branch = context.get("branch")
    event_name = context.get("event_name")
    title = (
        f"Preview for PR #{pr_number}"
        if pr_number
        else f"Preview for {branch or 'unknown branch'}"
    )
    conclusion = (
        "success"
        if all(
            summary.get(key) == "success"
            for key in ("test_status", "bench_status", "coverage_status")
        )
        else "failure"
    )

    return {
        "run_id": run_id,
        "run_url": context.get("run_url"),
        "title": title,
        "event": event_name,
        "branch": branch,
        "commit": context.get("commit"),
        "created_at": datetime.now(timezone.utc).isoformat(),
        "updated_at": datetime.now(timezone.utc).isoformat(),
        "conclusion": conclusion,
        "pr_number": pr_number,
        "summary": {
            "test_status": summary.get("test_status"),
            "bench_status": summary.get("bench_status"),
            "coverage_status": summary.get("coverage_status"),
            "tests": summary.get("tests", {}),
            "coverage_summary": summary.get("coverage_summary"),
            "benchmarks": summary.get("benchmarks", []),
            "coverage_url": None,
            "preview_url": None,
        },
    }


def fetch_dashboard_entries(args: argparse.Namespace) -> tuple[list[dict], dict[int, str]]:
    if args.fixture_json:
        with Path(args.fixture_json).open() as infile:
            payload = json.load(infile)
        return payload.get("entries", []), {}

    if not args.token:
        raise RuntimeError("--token is required unless --fixture-json is used")
    if not args.workflow:
        raise RuntimeError("--workflow is required when fetching Actions runs")

    encoded_workflow = urllib.parse.quote(args.workflow, safe="")
    runs_url = (
        f"https://api.github.com/repos/{args.repo}/actions/workflows/"
        f"{encoded_workflow}/runs?per_page={args.max_runs}"
    )
    runs_payload = github_request(runs_url, args.token)

    entries = []
    coverage_archives = {}
    for run in runs_payload.get("workflow_runs", []):
        if run.get("status") != "completed":
            continue

        artifacts_url = (
            f"https://api.github.com/repos/{args.repo}/actions/runs/"
            f"{run['id']}/artifacts?per_page=100"
        )
        artifacts_payload = github_request(artifacts_url, args.token)
        artifacts = artifacts_payload.get("artifacts", [])
        artifact = find_artifact_by_name(artifacts, args.artifact_name)
        if artifact is None:
            continue

        coverage_artifact = find_artifact_by_name(artifacts, args.coverage_artifact_name)
        if coverage_artifact is not None:
            coverage_archives[run["id"]] = coverage_artifact["archive_download_url"]

        try:
            archive_bytes = download_artifact(
                artifact["archive_download_url"], args.token
            )
            summary = load_dashboard_entry(archive_bytes)
        except (urllib.error.HTTPError, RuntimeError, zipfile.BadZipFile):
            continue

        entries.append(normalize_entry(run, summary))

    entries.sort(key=lambda entry: entry.get("updated_at") or entry.get("created_at") or "")
    return entries, coverage_archives


def selected_coverage_entries(entries: list[dict]) -> list[dict]:
    selected = []

    latest_main = next(
        (
            entry
            for entry in reversed(entries)
            if entry.get("event") == "push" and entry.get("branch") == "main"
        ),
        None,
    )
    if latest_main is not None:
        selected.append(latest_main)

    latest_by_pr = {}
    for entry in entries:
        pr_number = entry.get("pr_number")
        if pr_number:
            latest_by_pr[pr_number] = entry

    selected.extend(latest_by_pr.values())
    return selected


def selected_preview_entries(entries: list[dict]) -> list[dict]:
    latest_by_pr = {}
    for entry in entries:
        pr_number = entry.get("pr_number")
        if pr_number:
            latest_by_pr[pr_number] = entry
    return list(latest_by_pr.values())


def write_fixture_coverage_page(target_dir: Path, entry: dict) -> None:
    target_dir.mkdir(parents=True, exist_ok=True)
    target_dir.joinpath("index.html").write_text(
        f"""<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>Coverage Preview</title>
    <style>
      body {{
        margin: 0;
        padding: 32px;
        font-family: sans-serif;
        background: #f6efe6;
        color: #17202a;
      }}
      .card {{
        max-width: 720px;
        padding: 24px;
        border-radius: 20px;
        background: white;
        box-shadow: 0 18px 48px rgba(23, 32, 42, 0.1);
      }}
      a {{
        color: #8d341a;
      }}
    </style>
  </head>
  <body>
    <div class="card">
      <h1>Coverage Preview</h1>
      <p>Run: {entry["run_id"]}</p>
      <p>Branch: {entry.get("branch") or "unknown"}</p>
      <p>Coverage: {entry["summary"].get("coverage_summary") or "Unavailable"}</p>
      <p><a href="../../bench/?pr={entry.get("pr_number") or ""}">Back to dashboard</a></p>
    </div>
  </body>
</html>
""",
        encoding="utf-8",
    )


def extract_coverage_html(archive_bytes: bytes, target_dir: Path) -> None:
    target_dir.mkdir(parents=True, exist_ok=True)

    with zipfile.ZipFile(io.BytesIO(archive_bytes)) as archive:
        html_members = [
            member
            for member in archive.infolist()
            if "build/coverage/html/" in member.filename and not member.is_dir()
        ]
        if not html_members:
            raise RuntimeError("coverage-report artifact does not contain HTML output")

        for member in html_members:
            relative_path = member.filename.split("build/coverage/html/", 1)[1]
            if not relative_path:
                continue
            output_path = target_dir / relative_path
            output_path.parent.mkdir(parents=True, exist_ok=True)
            with archive.open(member) as infile, output_path.open("wb") as outfile:
                shutil.copyfileobj(infile, outfile)


def preview_relative_url(pr_number: int) -> str:
    return f"../previews/pr-{pr_number}/"


def set_preview_urls(entries: list[dict]) -> None:
    for entry in selected_preview_entries(entries):
        pr_number = entry.get("pr_number")
        if pr_number:
            entry["summary"]["preview_url"] = preview_relative_url(pr_number)


def extract_preview_site(archive_bytes: bytes, target_dir: Path) -> None:
    target_dir.mkdir(parents=True, exist_ok=True)

    with zipfile.ZipFile(io.BytesIO(archive_bytes)) as archive:
        members = [member for member in archive.infolist() if not member.is_dir()]
        if not members:
            raise RuntimeError("preview-site artifact is empty")

        root_prefix = None
        if all(len(Path(member.filename).parts) > 1 for member in members):
            first_parts = {Path(member.filename).parts[0] for member in members}
            if len(first_parts) == 1:
                root_prefix = next(iter(first_parts))

        for member in members:
            relative_path = Path(member.filename)
            if root_prefix and relative_path.parts and relative_path.parts[0] == root_prefix:
                relative_path = Path(*relative_path.parts[1:])
            if not relative_path.parts:
                continue

            output_path = target_dir / relative_path
            output_path.parent.mkdir(parents=True, exist_ok=True)
            with archive.open(member) as infile, output_path.open("wb") as outfile:
                shutil.copyfileobj(infile, outfile)


def write_fixture_preview_site(target_dir: Path, entry: dict) -> None:
    target_dir.mkdir(parents=True, exist_ok=True)
    pr_number = entry.get("pr_number")
    branch = entry.get("branch") or "unknown"
    target_dir.joinpath("index.html").write_text(
        f"""<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>PR Preview</title>
    <style>
      body {{
        margin: 0;
        padding: 32px;
        font-family: sans-serif;
        background: #f6efe6;
        color: #17202a;
      }}
      .card {{
        max-width: 720px;
        padding: 24px;
        border-radius: 20px;
        background: white;
        box-shadow: 0 18px 48px rgba(23, 32, 42, 0.1);
      }}
      a {{
        color: #8d341a;
      }}
    </style>
  </head>
  <body>
    <div class="card">
      <h1>PR Preview</h1>
      <p>PR: #{pr_number}</p>
      <p>Branch: {branch}</p>
      <p><a href="../../bench/?pr={pr_number}">Back to dashboard</a></p>
    </div>
  </body>
</html>
""",
        encoding="utf-8",
    )


def attach_coverage_pages(
    entries: list[dict],
    coverage_archives: dict[int, str],
    out_dir: Path,
    token: str | None,
    fixture_mode: bool,
) -> None:
    coverage_root = out_dir / "dev" / "coverage" / "runs"

    for entry in selected_coverage_entries(entries):
        if entry["summary"].get("coverage_status") != "success":
            continue

        run_id = entry["run_id"]
        target_dir = coverage_root / str(run_id)
        relative_url = f"../coverage/runs/{run_id}/index.html"

        try:
            if fixture_mode:
                write_fixture_coverage_page(target_dir, entry)
            else:
                archive_url = coverage_archives.get(run_id)
                if archive_url is None:
                    continue
                extract_coverage_html(download_artifact(archive_url, token), target_dir)
        except (urllib.error.HTTPError, RuntimeError, zipfile.BadZipFile):
            continue

        entry["summary"]["coverage_url"] = relative_url


def attach_preview_sites(
    entries: list[dict],
    preview_archives: dict[int, str],
    out_dir: Path,
    token: str | None,
    fixture_mode: bool,
) -> None:
    preview_root = out_dir / "dev" / "previews"
    set_preview_urls(entries)

    for entry in selected_preview_entries(entries):
        pr_number = entry.get("pr_number")
        if not pr_number:
            continue

        target_dir = preview_root / f"pr-{pr_number}"
        try:
            if fixture_mode:
                write_fixture_preview_site(target_dir, entry)
            else:
                archive_url = preview_archives.get(entry["run_id"])
                if archive_url is None:
                    entry["summary"]["preview_url"] = None
                    continue
                extract_preview_site(download_artifact(archive_url, token), target_dir)
        except (urllib.error.HTTPError, RuntimeError, zipfile.BadZipFile):
            entry["summary"]["preview_url"] = None
            continue


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


def attach_local_coverage(entry: dict, coverage_html_dir: str | None, out_dir: Path) -> None:
    if entry["summary"].get("coverage_status") != "success":
        return
    if not coverage_html_dir:
        return

    source_dir = Path(coverage_html_dir)
    if not source_dir.exists():
        return

    target_dir = out_dir / "dev" / "coverage" / "runs" / str(entry["run_id"])
    shutil.copytree(source_dir, target_dir, dirs_exist_ok=True)
    entry["summary"]["coverage_url"] = f"../coverage/runs/{entry['run_id']}/index.html"


def build_local_payload(args: argparse.Namespace) -> dict:
    if not args.entry_json:
        raise RuntimeError("--entry-json is required for local preview generation")

    entry = normalize_local_entry(load_local_dashboard_entry(args.entry_json))
    payload = build_payload(args.repo, [entry])
    write_site(args, payload)
    attach_local_coverage(entry, args.coverage_html_dir, Path(args.out_dir))
    return payload


def main() -> int:
    args = parse_args()
    if args.entry_json:
        payload = build_local_payload(args)
    else:
        entries, coverage_archives = fetch_dashboard_entries(args)
        payload = build_payload(args.repo, entries)
        write_site(args, payload)
        attach_coverage_pages(
            payload["entries"],
            coverage_archives,
            Path(args.out_dir),
            args.token,
            args.fixture_json is not None,
        )

        preview_archives = {}
        if args.fixture_json:
            preview_archives = {}
        else:
            if not args.token:
                raise RuntimeError("--token is required when fetching preview artifacts")
            preview_archives = {}
            encoded_workflow = urllib.parse.quote(args.workflow, safe="")
            runs_url = (
                f"https://api.github.com/repos/{args.repo}/actions/workflows/"
                f"{encoded_workflow}/runs?per_page={args.max_runs}"
            )
            runs_payload = github_request(runs_url, args.token)
            for run in runs_payload.get("workflow_runs", []):
                if run.get("status") != "completed":
                    continue
                artifacts_url = (
                    f"https://api.github.com/repos/{args.repo}/actions/runs/"
                    f"{run['id']}/artifacts?per_page=100"
                )
                artifacts_payload = github_request(artifacts_url, args.token)
                artifact = find_artifact_by_name(
                    artifacts_payload.get("artifacts", []), args.preview_artifact_name
                )
                if artifact is not None:
                    preview_archives[run["id"]] = artifact["archive_download_url"]

        attach_preview_sites(
            payload["entries"],
            preview_archives,
            Path(args.out_dir),
            args.token,
            args.fixture_json is not None,
        )

    with (Path(args.out_dir) / "dev" / "bench" / "dashboard.json").open("w") as outfile:
        json.dump(payload, outfile, indent=2)
        outfile.write("\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
