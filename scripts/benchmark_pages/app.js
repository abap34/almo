async function loadDashboard() {
  const response = await fetch("./dashboard.json", { cache: "no-store" });
  if (!response.ok) throw new Error("Failed to load dashboard data");
  return response.json();
}

function shortCommit(commit) {
  return (commit || "").slice(0, 7);
}

function formatMs(value) {
  return `${value.toFixed(3)}`;
}

function formatDelta(value) {
  const sign = value > 0 ? "+" : "";
  return `${sign}${value.toFixed(3)}`;
}

function formatDeltaPercent(value) {
  if (!Number.isFinite(value)) return "n/a";
  const sign = value > 0 ? "+" : "";
  return `${sign}${value.toFixed(1)}%`;
}

function formatTimestamp(value) {
  const d = new Date(value);
  return d.toLocaleString("en-US", {
    month: "short", day: "numeric", year: "numeric",
    hour: "2-digit", minute: "2-digit",
  });
}

function formatDate(value) {
  const d = new Date(value);
  return d.toLocaleString("en-US", { month: "short", day: "numeric" });
}

function statusClass(status) {
  return { success: "success", failure: "failure", cancelled: "muted", skipped: "muted" }[status] || "muted";
}

function benchmarkNames(entries) {
  const names = new Set();
  entries.forEach((e) => (e.summary?.benchmarks || []).forEach((b) => names.add(b.name)));
  return Array.from(names);
}

function benchmarkSeries(entries, benchmarkName) {
  return entries
    .map((entry) => {
      const b = (entry.summary?.benchmarks || []).find((x) => x.name === benchmarkName);
      if (!b) return null;
      return {
        label: shortCommit(entry.commit),
        date: formatDate(entry.updated_at || entry.created_at),
        timestamp: entry.updated_at || entry.created_at,
        href: entry.run_url,
        value: b.mean_ms,
        min: b.min_ms,
        max: b.max_ms,
      };
    })
    .filter(Boolean);
}

function latestBenchmarkMap(entry) {
  const map = new Map();
  (entry?.summary?.benchmarks || []).forEach((b) => map.set(b.name, b));
  return map;
}

function mainEntries(entries) {
  return entries.filter(
    (e) =>
      e.event === "push" &&
      e.branch === "main" &&
      e.summary?.bench_status === "success" &&
      (e.summary?.benchmarks || []).length > 0
  );
}

function pullRequestGroups(entries) {
  const groups = new Map();
  entries
    .filter((e) => e.pr_number)
    .forEach((e) => {
      if (!groups.has(e.pr_number)) groups.set(e.pr_number, []);
      groups.get(e.pr_number).push(e);
    });

  return Array.from(groups.entries())
    .map(([number, groupEntries]) => ({
      number,
      entries: groupEntries.sort(
        (a, b) => new Date(a.updated_at || a.created_at) - new Date(b.updated_at || b.created_at)
      ),
    }))
    .sort((a, b) => {
      const al = a.entries[a.entries.length - 1];
      const bl = b.entries[b.entries.length - 1];
      return new Date(bl.updated_at || bl.created_at) - new Date(al.updated_at || al.created_at);
    });
}

function setText(id, value) {
  const el = document.getElementById(id);
  if (el) el.textContent = value;
}

function setLink(id, href) {
  const el = document.getElementById(id);
  if (!el) return;
  if (!href) {
    el.classList.add("hidden");
    el.removeAttribute("href");
  } else {
    el.classList.remove("hidden");
    el.href = href;
  }
}

function updateUrl(state) {
  const url = new URL(window.location.href);
  state.selectedPrNumber
    ? url.searchParams.set("pr", String(state.selectedPrNumber))
    : url.searchParams.delete("pr");
  state.selectedBenchmark
    ? url.searchParams.set("benchmark", state.selectedBenchmark)
    : url.searchParams.delete("benchmark");
  window.history.replaceState({}, "", url);
}

function renderTabs(containerId, names, selectedName, onSelect) {
  const container = document.getElementById(containerId);
  if (!container) return;
  container.innerHTML = "";
  names.forEach((name) => {
    const btn = document.createElement("button");
    btn.type = "button";
    btn.className = `tab${name === selectedName ? " active" : ""}`;
    btn.textContent = name;
    btn.addEventListener("click", () => onSelect(name));
    container.appendChild(btn);
  });
}

function renderChart(svgId, series, captionId) {
  const svg = document.getElementById(svgId);
  const caption = document.getElementById(captionId);
  if (!svg) return;

  if (series.length === 0) {
    svg.innerHTML = "";
    if (caption) caption.textContent = "No data";
    return;
  }

  if (caption) caption.textContent = `${series.length} run${series.length !== 1 ? "s" : ""}`;

  const W = 960;
  const H = 260;
  const pad = { top: 28, right: 32, bottom: 44, left: 72 };
  const iW = W - pad.left - pad.right;
  const iH = H - pad.top - pad.bottom;

  const values = series.map((p) => p.value);
  const rawMin = Math.min(...values);
  const rawMax = Math.max(...values);
  const span = Math.max(rawMax - rawMin, 0.1);
  const yMin = rawMin - span * 0.15;
  const yMax = rawMax + span * 0.15;
  const ySpan = yMax - yMin;

  const xPos = (i) =>
    pad.left + (series.length === 1 ? iW / 2 : (iW * i) / (series.length - 1));
  const yPos = (v) =>
    pad.top + iH - ((v - yMin) / ySpan) * iH;

  // grid & axis labels
  const gridCount = 4;
  let gridLines = "";
  let yLabels = "";
  for (let i = 0; i <= gridCount; i++) {
    const ratio = i / gridCount;
    const y = pad.top + iH * ratio;
    const val = yMax - (yMax - yMin) * ratio;
    gridLines += `<line class="chart-grid-line" x1="${pad.left}" y1="${y}" x2="${W - pad.right}" y2="${y}" />`;
    yLabels += `<text class="chart-axis-label" x="${pad.left - 8}" y="${y + 4}" text-anchor="end">${val.toFixed(2)}</text>`;
  }

  // x-axis labels — show up to 8
  const labelStep = Math.max(1, Math.ceil(series.length / 8));
  let xLabels = "";
  series.forEach((p, i) => {
    if (i % labelStep !== 0 && i !== series.length - 1) return;
    xLabels += `<text class="chart-tick" x="${xPos(i)}" y="${H - 6}" text-anchor="middle">${p.label}</text>`;
  });

  // line path
  const linePath = series.map((p, i) => `${i === 0 ? "M" : "L"}${xPos(i)},${yPos(p.value)}`).join(" ");

  // area path (closes at bottom)
  const areaPath =
    linePath +
    ` L${xPos(series.length - 1)},${pad.top + iH} L${xPos(0)},${pad.top + iH} Z`;

  // dots + value labels (only for small series or last point)
  const showAllLabels = series.length <= 8;
  let dots = "";
  series.forEach((p, i) => {
    const cx = xPos(i);
    const cy = yPos(p.value);
    const isLast = i === series.length - 1;
    dots += `
      <a href="${p.href}" target="_blank" rel="noreferrer">
        <circle class="chart-dot" cx="${cx}" cy="${cy}" r="4">
          <title>${p.date}: ${p.value.toFixed(3)} ms</title>
        </circle>
      </a>`;
    if (showAllLabels || isLast) {
      const anchor = cx < pad.left + 40 ? "start" : cx > W - pad.right - 40 ? "end" : "middle";
      dots += `<text class="chart-value-label" x="${cx}" y="${cy - 10}" text-anchor="${anchor}">${p.value.toFixed(2)}</text>`;
    }
  });

  svg.innerHTML = `
    ${gridLines}
    <line class="chart-axis-line" x1="${pad.left}" y1="${pad.top}" x2="${pad.left}" y2="${pad.top + iH}" />
    <line class="chart-axis-line" x1="${pad.left}" y1="${pad.top + iH}" x2="${W - pad.right}" y2="${pad.top + iH}" />
    <path class="chart-area" d="${areaPath}" />
    <path class="chart-line" d="${linePath}" />
    ${dots}
    ${yLabels}
    ${xLabels}
  `;
}

function renderMainLatestTable(entry) {
  const tbody = document.getElementById("main-latest-table");
  if (!tbody) return;
  tbody.innerHTML = "";
  (entry?.summary?.benchmarks || []).forEach((b) => {
    const row = document.createElement("tr");
    row.innerHTML = `
      <td>${b.name}</td>
      <td class="mono">${b.mean_ms.toFixed(3)}</td>
      <td class="mono">${b.min_ms.toFixed(3)}</td>
      <td class="mono">${b.max_ms.toFixed(3)}</td>
      <td class="mono">${b.iterations}</td>
    `;
    tbody.appendChild(row);
  });
}

function renderMeta(payload, mainRunEntries, prGroups) {
  setText("repo-name", payload.repo);
  setText("generated-at", `Updated ${formatTimestamp(payload.generated_at)}`);
  setText("main-run-count", String(mainRunEntries.length));
  setText("pr-count", String(prGroups.length));
  const names = benchmarkNames(mainRunEntries.concat(...prGroups.map((g) => g.entries)));
  setText("bench-count", String(names.length));
}

function renderPrList(groups, selectedPrNumber, onSelect) {
  const container = document.getElementById("pr-list");
  if (!container) return;
  container.innerHTML = "";

  if (groups.length === 0) {
    const empty = document.createElement("div");
    empty.style.cssText = "padding:24px 16px;color:var(--text-muted);font-size:13px;";
    empty.textContent = "No pull requests yet.";
    container.appendChild(empty);
    return;
  }

  groups.forEach((group) => {
    const latest = group.entries[group.entries.length - 1];
    const btn = document.createElement("button");
    btn.type = "button";
    btn.className = `pr-item${group.number === selectedPrNumber ? " active" : ""}`;
    btn.addEventListener("click", () => onSelect(group.number));

    const num = document.createElement("div");
    num.className = "pr-item-number";
    num.textContent = `#${group.number}`;

    const title = document.createElement("div");
    title.className = "pr-item-title";
    title.textContent = latest.title || latest.branch;

    const foot = document.createElement("div");
    foot.className = "pr-item-foot";

    const time = document.createElement("span");
    time.className = "pr-item-time";
    time.textContent = formatDate(latest.updated_at || latest.created_at);

    const tone = statusClass(
      latest.summary?.coverage_status === "failure" ? "failure" : latest.conclusion
    );
    const badge = document.createElement("span");
    badge.className = `inline-badge ${tone}`;
    badge.textContent =
      latest.summary?.coverage_status === "failure" ? "cov failed" : latest.conclusion;

    foot.appendChild(time);
    foot.appendChild(badge);
    btn.appendChild(num);
    btn.appendChild(title);
    btn.appendChild(foot);
    container.appendChild(btn);
  });
}

function renderStatusBadges(entry) {
  const container = document.getElementById("pr-statuses");
  if (!container) return;
  container.innerHTML = "";

  const items = [
    ["Tests", entry.summary?.test_status, `${entry.summary?.tests?.passed || 0} passed`],
    ["Bench", entry.summary?.bench_status, `${(entry.summary?.benchmarks || []).length} benchmarks`],
    ["Coverage", entry.summary?.coverage_status, entry.summary?.coverage_summary || "Unavailable"],
  ];

  items.forEach(([label, status, detail]) => {
    const badge = document.createElement("div");
    badge.className = `status-badge ${statusClass(status)}`;
    badge.innerHTML = `<span class="badge-label">${label}</span><span class="badge-value">${detail}</span>`;
    container.appendChild(badge);
  });
}

function renderComparisonTable(prEntry, mainEntry) {
  const tbody = document.getElementById("pr-compare-table");
  if (!tbody) return;
  tbody.innerHTML = "";

  const baseline = latestBenchmarkMap(mainEntry);
  (prEntry.summary?.benchmarks || []).forEach((b) => {
    const base = baseline.get(b.name);
    const delta = base ? b.mean_ms - base.mean_ms : NaN;
    const pct = base ? (delta / base.mean_ms) * 100 : NaN;
    const deltaClass = !base ? "" : delta > 0 ? " class=\"positive\"" : delta < 0 ? " class=\"negative\"" : "";

    const row = document.createElement("tr");
    row.innerHTML = `
      <td>${b.name}</td>
      <td class="mono">${base ? base.mean_ms.toFixed(3) : "n/a"}</td>
      <td class="mono">${b.mean_ms.toFixed(3)}</td>
      <td class="mono"${deltaClass}>${base ? formatDelta(delta) : "n/a"}</td>
      <td class="mono"${deltaClass}>${base ? formatDeltaPercent(pct) : "n/a"}</td>
    `;
    tbody.appendChild(row);
  });
}

function renderRunHistory(entries) {
  const container = document.getElementById("pr-run-history");
  if (!container) return;
  container.innerHTML = "";

  entries
    .slice()
    .reverse()
    .forEach((entry) => {
      const item = document.createElement("a");
      item.className = "run-history-item";
      item.href = entry.run_url;
      item.target = "_blank";
      item.rel = "noreferrer";

      const commit = document.createElement("span");
      commit.className = "run-history-commit";
      commit.textContent = shortCommit(entry.commit);

      const info = document.createElement("div");
      info.className = "run-history-info";

      const time = document.createElement("div");
      time.className = "run-history-time";
      time.textContent = formatTimestamp(entry.updated_at || entry.created_at);

      const stats = document.createElement("div");
      stats.className = "run-history-stats";
      stats.textContent = `${entry.summary?.tests?.passed || 0} passed · ${
        entry.summary?.coverage_summary || "coverage n/a"
      }`;

      const tone = statusClass(entry.conclusion);
      const badge = document.createElement("span");
      badge.className = `inline-badge ${tone}`;
      badge.textContent = entry.conclusion;

      info.appendChild(time);
      info.appendChild(stats);
      item.appendChild(commit);
      item.appendChild(info);
      item.appendChild(badge);
      container.appendChild(item);
    });
}

function renderPrDetail(group, mainEntry, selectedBenchmark, onSelectBenchmark) {
  const empty = document.getElementById("pr-empty");
  const detail = document.getElementById("pr-detail");
  if (!empty || !detail) return;

  if (!group) {
    empty.classList.remove("hidden");
    detail.classList.add("hidden");
    return;
  }

  empty.classList.add("hidden");
  detail.classList.remove("hidden");

  const latest = group.entries[group.entries.length - 1];
  setText("pr-title", `#${group.number} ${latest.title || latest.branch}`);
  setText(
    "pr-meta",
    `${latest.branch} · ${shortCommit(latest.commit)} · ${formatTimestamp(
      latest.updated_at || latest.created_at
    )} · ${group.entries.length} run${group.entries.length !== 1 ? "s" : ""}`
  );
  setLink("pr-run-link", latest.run_url);
  setLink("pr-preview-link", latest.summary?.preview_url || null);
  setLink("pr-coverage-link", latest.summary?.coverage_url || null);

  renderStatusBadges(latest);

  const names = benchmarkNames(group.entries);
  const benchmark = names.includes(selectedBenchmark) ? selectedBenchmark : names[0];
  renderTabs("pr-benchmark-tabs", names, benchmark, onSelectBenchmark);

  setText("pr-chart-title", benchmark || "—");
  renderChart("pr-chart", benchmarkSeries(group.entries, benchmark), "pr-chart-caption");
  renderComparisonTable(latest, mainEntry);
  renderRunHistory(group.entries);
}

function renderDashboard(payload) {
  const allEntries = payload.entries || [];

  if (allEntries.length === 0) {
    setText("repo-name", payload.repo || "Unknown");
    setText("generated-at", `Updated ${formatTimestamp(payload.generated_at || new Date().toISOString())}`);
    setText("main-run-count", "0");
    setText("pr-count", "0");
    setText("bench-count", "0");
    setLink("main-coverage-link", null);
    renderPrDetail(null, null, null, () => {});
    return;
  }

  const mainRunEntries = mainEntries(allEntries);
  const prGroups = pullRequestGroups(allEntries);
  const mainNames = benchmarkNames(mainRunEntries);
  const params = new URLSearchParams(window.location.search);

  const state = {
    selectedBenchmark: params.get("benchmark") || mainNames[0] || benchmarkNames(allEntries)[0],
    selectedPrNumber: Number(params.get("pr")) || (prGroups[0] ? prGroups[0].number : null),
  };

  function render() {
    const latestMain = mainRunEntries[mainRunEntries.length - 1];
    const selectedPrGroup = prGroups.find((g) => g.number === state.selectedPrNumber) || null;
    const prBenchNames = selectedPrGroup ? benchmarkNames(selectedPrGroup.entries) : [];
    const availableNames = mainNames.length > 0 ? mainNames : prBenchNames;

    if (!availableNames.includes(state.selectedBenchmark)) {
      state.selectedBenchmark = availableNames[0];
    }

    updateUrl(state);
    renderMeta(payload, mainRunEntries, prGroups);
    setLink("main-coverage-link", latestMain?.summary?.coverage_url || null);

    renderTabs("main-benchmark-tabs", availableNames, state.selectedBenchmark, (name) => {
      state.selectedBenchmark = name;
      render();
    });

    setText("main-chart-title", state.selectedBenchmark || "No benchmark data");
    renderChart("main-chart", benchmarkSeries(mainRunEntries, state.selectedBenchmark), "main-chart-caption");
    renderMainLatestTable(latestMain);

    renderPrList(prGroups, state.selectedPrNumber, (number) => {
      state.selectedPrNumber = number;
      render();
    });

    renderPrDetail(selectedPrGroup, latestMain, state.selectedBenchmark, (name) => {
      state.selectedBenchmark = name;
      render();
    });
  }

  render();
}

loadDashboard()
  .then(renderDashboard)
  .catch((error) => {
    document.body.innerHTML = `
      <div style="display:flex;align-items:center;justify-content:center;min-height:100vh;font-family:monospace;color:#555;">
        Failed to load dashboard: ${error.message}
      </div>`;
  });
