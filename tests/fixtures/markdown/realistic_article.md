# Building A Faster Markdown Preview Pipeline

This note documents the changes we made to reduce preview latency in our internal docs editor. The full benchmark appendix lives at [the engineering tracker](https://example.com/engineering/preview-latency).

## Background

Teams were reporting that large pages felt sluggish once they mixed prose, charts, and snippets. We traced the issue to repeated parsing work in the preview layer and to expensive DOM updates around embedded media.[^perf-note]

> The fastest path was not rewriting everything.
> We just needed to stop reparsing the same content.

- Capture parser output once per edit batch.
- Reuse the rendered summary for cheap diffs.
- Keep media-heavy content in the document model instead of reserializing it.

## Data Snapshot

| Scenario | Before | After |
| :--- | ---: | ---: |
| Cold parse | 182 ms | 141 ms |
| Warm preview | 97 ms | 44 ms |

### Code Change

```cpp
std::string render_preview(const Document& document) {
    return renderer.render(document);
}
```

We also validated the rollout with an architecture sketch:

![Preview pipeline](https://example.com/assets/preview-pipeline.png)

$$
T_{total} = T_{parse} + T_{render} + T_{sync}
$$

## Rollout Notes

The initial rollout landed behind a feature flag and stayed there for one week. After that, we widened the rollout and checked that the p95 editor interaction time stayed below the team target.

[^perf-note]: Measurements were collected from the internal preview service during the first rollout window.
