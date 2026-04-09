# Release Notes 1.4.0

This release improves fixture coverage, benchmark reporting, and dashboard ergonomics. Related tracking issue: [#fixtures](https://example.com/issues/fixtures).

## Highlights

- Added realistic Markdown fixtures for article-style content.
- Expanded benchmark scenarios for table-heavy and SVG-heavy documents.
- Improved PR summaries with benchmark diffs against the merge target.

---

## Upgrade Checklist

1. Pull the latest `main`.
2. Run the CI workflow locally if you are modifying benchmark output.
3. Regenerate dashboard fixtures if the benchmark shape changes.

> We recommend comparing benchmark trends, not single runs, before calling a regression.

## Compatibility Matrix

| Surface | Status | Notes |
| :--- | :-: | :--- |
| Parser fixtures | OK | Added more real-world samples |
| Benchmark comments | OK | Shows diff versus base branch |
| Dashboard pages | OK | No schema changes |
