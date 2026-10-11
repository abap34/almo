#!/usr/bin/env python3

import io
import json
import sys
import tempfile
import unittest
import urllib.error
from pathlib import Path
from unittest.mock import patch

import build_benchmark_dashboard as dashboard


FIXTURE = Path(__file__).parent / "benchmark_pages" / "fixture_dashboard.json"


class BenchmarkHistoryTests(unittest.TestCase):
    def setUp(self):
        self.payload = json.loads(FIXTURE.read_text())
        self.entries = self.payload["entries"]

    def test_expired_and_out_of_window_runs_are_retained(self):
        fresh = json.loads(json.dumps(self.entries[-1]))
        fresh["summary"]["benchmarks"][0]["mean_ms"] = 1.25
        merged = dashboard.merge_entries(self.entries, [fresh])
        self.assertEqual(len(merged), len(self.entries))
        self.assertEqual(merged[-1]["summary"]["benchmarks"][0]["mean_ms"], 1.25)
        self.assertEqual(dashboard.merge_entries(merged, []), merged)
        self.assertEqual(dashboard.merge_entries(merged, [fresh]), merged)

    def test_measurement_order_ignores_completion_time(self):
        first, second = self.entries[:2]
        first["updated_at"] = "2026-05-01T09:00:00Z"
        merged = dashboard.merge_entries([second, first], [])
        self.assertEqual([e["run_id"] for e in merged], [9101, 9102])
        # A rerun replaces the original run and uses its new start time.
        rerun = {**first, "run_started_at": "2026-05-02T09:00:00Z"}
        merged = dashboard.merge_entries(merged, [rerun])
        self.assertEqual([e["run_id"] for e in merged], [9102, 9101])

    def test_expired_html_links_are_not_carried_forward(self):
        entry = self.entries[0]
        entry["summary"].update(coverage_url="old-coverage", preview_url="old-preview")
        merged = dashboard.merge_entries([entry], [])
        self.assertIsNone(merged[0]["summary"]["coverage_url"])
        self.assertIsNone(merged[0]["summary"]["preview_url"])
        self.assertEqual(entry["summary"]["coverage_url"], "old-coverage")

    def test_load_history_without_api_authorization(self):
        with patch.object(dashboard.urllib.request, "urlopen", return_value=io.BytesIO(
            json.dumps(self.payload).encode()
        )) as urlopen:
            self.assertEqual(dashboard.load_published_history("https://example.com/history", "Abap34/Almo"), self.entries)
        request = urlopen.call_args.args[0]
        self.assertFalse(request.has_header("Authorization"))
        self.assertEqual(urlopen.call_args.kwargs["timeout"], 30)

    def test_only_missing_history_allows_an_empty_initial_deployment(self):
        for code in (404, 403, 500, 503):
            error = urllib.error.HTTPError("https://example.com", code, "error", {}, None)
            self.addCleanup(error.close)
            with self.subTest(code=code), patch.object(
                dashboard.urllib.request, "urlopen",
                side_effect=error,
            ):
                if code == 404:
                    self.assertEqual(dashboard.load_published_history("https://example.com", "abap34/almo"), [])
                else:
                    with self.assertRaises(urllib.error.HTTPError):
                        dashboard.load_published_history("https://example.com", "abap34/almo")

    def test_wrong_repository_or_malformed_history_aborts(self):
        for payload in (
            {**self.payload, "repo": "someone/else"},
            {**self.payload, "entries": None},
            {**self.payload, "entries": [{"run_id": 1}]},
            [],
        ):
            with self.subTest(payload=payload), patch.object(
                dashboard.urllib.request, "urlopen", return_value=io.BytesIO(json.dumps(payload).encode())
            ), self.assertRaises(RuntimeError):
                dashboard.load_published_history("https://example.com", "abap34/almo")

    def test_rebuilt_site_contains_old_and_new_measurements(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            history = root / "history.json"
            history.write_text(json.dumps(self.payload))
            fresh = root / "fresh.json"
            fresh.write_text(json.dumps({"entries": [self.entries[-1]]}))
            out_dir = root / "site"
            with patch.object(sys, "argv", [
                "build_benchmark_dashboard.py", "--repo", "abap34/almo",
                "--out-dir", str(out_dir), "--fixture-json", str(fresh),
                "--history-url", history.as_uri(),
            ]):
                self.assertEqual(dashboard.main(), 0)
            result = json.loads((out_dir / "dev/bench/dashboard.json").read_text())
            self.assertEqual([e["run_id"] for e in result["entries"]], [e["run_id"] for e in self.entries])
            self.assertTrue((out_dir / "dev/coverage/runs/9102/index.html").exists())
            self.assertTrue((out_dir / "dev/previews/pr-174/index.html").exists())
            html = (out_dir / "dev/bench/index.html").read_text()
            for asset in ("styles.css", "app.js"):
                content = (out_dir / "dev/bench" / asset).read_bytes()
                version = dashboard.hashlib.sha256(content).hexdigest()[:12]
                self.assertIn(f'"./{asset}?v={version}"', html)

    def test_history_failure_leaves_previous_output_untouched(self):
        with tempfile.TemporaryDirectory() as temp:
            out_dir = Path(temp) / "site"
            out_dir.mkdir()
            (out_dir / "index.html").write_text("previous site")
            with patch.object(sys, "argv", [
                "build_benchmark_dashboard.py", "--repo", "abap34/almo",
                "--out-dir", str(out_dir), "--fixture-json", str(FIXTURE),
                "--history-url", "https://example.com/history",
            ]), patch.object(dashboard, "load_published_history", side_effect=RuntimeError("unavailable")):
                with self.assertRaises(RuntimeError):
                    dashboard.main()
            self.assertEqual((out_dir / "index.html").read_text(), "previous site")


if __name__ == "__main__":
    unittest.main()
