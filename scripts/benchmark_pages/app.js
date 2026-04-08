async function loadDashboard() {
  const response = await fetch("./dashboard.json", { cache: "no-store" });
  if (!response.ok) {
    throw new Error("Failed to load dashboard data");
  }
  return response.json();
}

function shortCommit(commit) {
  return (commit || "").slice(0, 7);
}

function formatMs(value) {
  return `${value.toFixed(3)} ms`;
}

function formatDelta(value) {
  const sign = value > 0 ? "+" : "";
  return `${sign}${value.toFixed(3)} ms`;
}

function formatDeltaPercent(value) {
  if (!Number.isFinite(value)) {
    return "n/a";
  }
  const sign = value > 0 ? "+" : "";
  return `${sign}${value.toFixed(1)}%`;
}

function formatTimestamp(value) {
  return new Date(value).toLocaleString();
}

function statusTone(status) {
  return {
    success: "success",
    failure: "failure",
    cancelled: "muted",
    skipped: "muted",
  }[status] || "muted";
}

function benchmarkNames(entries) {
  const names = new Set();
  entries.forEach((entry) => {
    (entry.summary?.benchmarks || []).forEach((benchmark) => names.add(benchmark.name));
  });
  return Array.from(names);
}

function benchmarkSeries(entries, benchmarkName) {
  return entries
    .map((entry) => {
      const benchmark = (entry.summary?.benchmarks || []).find((item) => item.name === benchmarkName);
      if (!benchmark) {
        return null;
      }
      return {
        label: shortCommit(entry.commit),
        timestamp: entry.updated_at || entry.created_at,
        href: entry.run_url,
        value: benchmark.mean_ms,
        meta: entry,
      };
    })
    .filter(Boolean);
}

function latestBenchmarkMap(entry) {
  const map = new Map();
  (entry?.summary?.benchmarks || []).forEach((benchmark) => {
    map.set(benchmark.name, benchmark);
  });
  return map;
}

function mainEntries(entries) {
  return entries.filter(
    (entry) =>
      entry.event === "push" &&
      entry.branch === "main" &&
      entry.summary?.bench_status === "success" &&
      (entry.summary?.benchmarks || []).length > 0
  );
}

function pullRequestGroups(entries) {
  const groups = new Map();

  entries
    .filter((entry) => entry.pr_number)
    .forEach((entry) => {
      if (!groups.has(entry.pr_number)) {
        groups.set(entry.pr_number, []);
      }
      groups.get(entry.pr_number).push(entry);
    });

  return Array.from(groups.entries())
    .map(([number, groupEntries]) => ({
      number,
      entries: groupEntries.sort(
        (left, right) =>
          new Date(left.updated_at || left.created_at) - new Date(right.updated_at || right.created_at)
      ),
    }))
    .sort((left, right) => {
      const leftLatest = left.entries[left.entries.length - 1];
      const rightLatest = right.entries[right.entries.length - 1];
      return new Date(rightLatest.updated_at || rightLatest.created_at) -
        new Date(leftLatest.updated_at || leftLatest.created_at);
    });
}

function setText(id, value) {
  document.getElementById(id).textContent = value;
}

function updateUrl(state) {
  const url = new URL(window.location.href);
  if (state.selectedPrNumber) {
    url.searchParams.set("pr", String(state.selectedPrNumber));
  } else {
    url.searchParams.delete("pr");
  }
  if (state.selectedBenchmark) {
    url.searchParams.set("benchmark", state.selectedBenchmark);
  } else {
    url.searchParams.delete("benchmark");
  }
  window.history.replaceState({}, "", url);
}

function renderTabs(containerId, names, selectedName, onSelect) {
  const container = document.getElementById(containerId);
  container.innerHTML = "";

  names.forEach((name) => {
    const button = document.createElement("button");
    button.type = "button";
    button.className = `tab${name === selectedName ? " active" : ""}`;
    button.textContent = name;
    button.addEventListener("click", () => onSelect(name));
    container.appendChild(button);
  });
}

function renderChart(svgId, series, captionId) {
  const svg = document.getElementById(svgId);
  const caption = document.getElementById(captionId);

  if (series.length === 0) {
    svg.innerHTML = "";
    caption.textContent = "No data available";
    return;
  }

  caption.textContent = `${series.length} run(s) tracked`;

  const width = 960;
  const height = 340;
  const margin = { top: 24, right: 24, bottom: 48, left: 64 };
  const innerWidth = width - margin.left - margin.right;
  const innerHeight = height - margin.top - margin.bottom;

  const values = series.map((point) => point.value);
  const min = Math.min(...values);
  const max = Math.max(...values);
  const span = Math.max(max - min, 1);
  const labelStep = Math.max(1, Math.ceil(series.length / 6));

  const x = (index) =>
    margin.left + (series.length === 1 ? innerWidth / 2 : (innerWidth * index) / (series.length - 1));
  const y = (value) =>
    margin.top + innerHeight - ((value - min) / span) * innerHeight;

  const path = series
    .map((point, index) => `${index === 0 ? "M" : "L"} ${x(index)} ${y(point.value)}`)
    .join(" ");

  const grid = [0, 0.25, 0.5, 0.75, 1]
    .map((ratio) => {
      const yPos = margin.top + innerHeight * ratio;
      return `<line class="chart-grid" x1="${margin.left}" y1="${yPos}" x2="${width - margin.right}" y2="${yPos}" />`;
    })
    .join("");

  const labels = series
    .map((point, index) => {
      if (index % labelStep !== 0 && index !== series.length - 1) {
        return "";
      }
      return `<text class="chart-label" x="${x(index)}" y="${height - 14}" text-anchor="middle">${point.label}</text>`;
    })
    .join("");

  const points = series
    .map((point, index) => {
      const xPos = x(index);
      const yPos = y(point.value);
      return `
        <a href="${point.href}" target="_blank" rel="noreferrer">
          <circle class="chart-point" cx="${xPos}" cy="${yPos}" r="5" />
        </a>
        <text class="chart-value" x="${xPos}" y="${yPos - 12}" text-anchor="middle">${point.value.toFixed(2)}</text>
      `;
    })
    .join("");

  svg.innerHTML = `
    ${grid}
    <line class="chart-axis" x1="${margin.left}" y1="${height - margin.bottom}" x2="${width - margin.right}" y2="${height - margin.bottom}" />
    <line class="chart-axis" x1="${margin.left}" y1="${margin.top}" x2="${margin.left}" y2="${height - margin.bottom}" />
    <path class="chart-line" d="${path}" />
    ${points}
    ${labels}
    <text class="chart-label" x="${margin.left}" y="${margin.top - 8}">${formatMs(max)}</text>
    <text class="chart-label" x="${margin.left}" y="${height - margin.bottom + 18}">${formatMs(min)}</text>
  `;
}

function renderMainLatestTable(entry) {
  const tbody = document.getElementById("main-latest-table");
  tbody.innerHTML = "";

  (entry?.summary?.benchmarks || []).forEach((benchmark) => {
    const row = document.createElement("tr");
    row.innerHTML = `
      <td>${benchmark.name}</td>
      <td>${benchmark.mean_ms.toFixed(3)}</td>
      <td>${benchmark.min_ms.toFixed(3)}</td>
      <td>${benchmark.max_ms.toFixed(3)}</td>
      <td>${benchmark.iterations}</td>
    `;
    tbody.appendChild(row);
  });
}

function renderMeta(payload, mainRunEntries, prGroups) {
  setText("repo-name", payload.repo);
  setText("generated-at", formatTimestamp(payload.generated_at));
  setText("main-run-count", String(mainRunEntries.length));
  setText("pr-count", String(prGroups.length));
}

function renderPrList(groups, selectedPrNumber, onSelect) {
  const container = document.getElementById("pr-list");
  container.innerHTML = "";

  groups.forEach((group) => {
    const latest = group.entries[group.entries.length - 1];
    const button = document.createElement("button");
    button.type = "button";
    button.className = `pr-item${group.number === selectedPrNumber ? " active" : ""}`;
    button.addEventListener("click", () => onSelect(group.number));

    const title = document.createElement("div");
    title.className = "pr-item-title";
    title.textContent = `#${group.number} ${latest.title}`;

    const meta = document.createElement("div");
    meta.className = "pr-item-meta";
    meta.textContent = `${latest.branch} · ${shortCommit(latest.commit)} · ${formatTimestamp(
      latest.updated_at || latest.created_at
    )}`;

    const tone = statusTone(latest.summary?.coverage_status === "failure" ? "failure" : latest.conclusion);
    const badge = document.createElement("span");
    badge.className = `status-badge ${tone}`;
    badge.textContent = latest.summary?.coverage_status === "failure" ? "coverage failed" : latest.conclusion;

    button.appendChild(title);
    button.appendChild(meta);
    button.appendChild(badge);
    container.appendChild(button);
  });
}

function renderStatusBadges(entry) {
  const container = document.getElementById("pr-statuses");
  container.innerHTML = "";

  const badges = [
    ["tests", entry.summary?.test_status, `${entry.summary?.tests?.passed || 0} passed`],
    ["bench", entry.summary?.bench_status, `${(entry.summary?.benchmarks || []).length} benchmarks`],
    ["coverage", entry.summary?.coverage_status, entry.summary?.coverage_summary || "Unavailable"],
  ];

  badges.forEach(([label, status, detail]) => {
    const badge = document.createElement("div");
    badge.className = `status-card ${statusTone(status)}`;
    const name = document.createElement("div");
    name.className = "status-name";
    name.textContent = label;
    const value = document.createElement("div");
    value.className = "status-detail";
    value.textContent = detail;
    badge.appendChild(name);
    badge.appendChild(value);
    container.appendChild(badge);
  });
}

function renderComparisonTable(prEntry, mainEntry) {
  const tbody = document.getElementById("pr-compare-table");
  tbody.innerHTML = "";

  const baseline = latestBenchmarkMap(mainEntry);
  (prEntry.summary?.benchmarks || []).forEach((benchmark) => {
    const base = baseline.get(benchmark.name);
    const delta = base ? benchmark.mean_ms - base.mean_ms : NaN;
    const deltaPercent = base ? (delta / base.mean_ms) * 100 : NaN;

    const row = document.createElement("tr");
    row.innerHTML = `
      <td>${benchmark.name}</td>
      <td>${base ? base.mean_ms.toFixed(3) : "n/a"}</td>
      <td>${benchmark.mean_ms.toFixed(3)}</td>
      <td>${base ? formatDelta(delta) : "n/a"}</td>
      <td>${base ? formatDeltaPercent(deltaPercent) : "n/a"}</td>
    `;
    tbody.appendChild(row);
  });
}

function renderRunHistory(entries) {
  const container = document.getElementById("pr-run-history");
  container.innerHTML = "";

  entries
    .slice()
    .reverse()
    .forEach((entry) => {
      const item = document.createElement("a");
      item.className = "history-item";
      item.href = entry.run_url;
      item.target = "_blank";
      item.rel = "noreferrer";

      const top = document.createElement("div");
      top.className = "history-top";
      top.textContent = `${shortCommit(entry.commit)} · ${formatTimestamp(
        entry.updated_at || entry.created_at
      )}`;

      const bottom = document.createElement("div");
      bottom.className = "history-bottom";
      bottom.textContent = `${entry.summary?.tests?.passed || 0} tests passed · ${
        entry.summary?.coverage_summary || "coverage unavailable"
      }`;

      item.appendChild(top);
      item.appendChild(bottom);
      container.appendChild(item);
    });
}

function renderPrDetail(group, mainEntry, selectedBenchmark, onSelectBenchmark) {
  const empty = document.getElementById("pr-empty");
  const detail = document.getElementById("pr-detail");

  if (!group) {
    empty.classList.remove("hidden");
    detail.classList.add("hidden");
    return;
  }

  empty.classList.add("hidden");
  detail.classList.remove("hidden");

  const latest = group.entries[group.entries.length - 1];
  document.getElementById("pr-title").textContent = `#${group.number} ${latest.title}`;
  document.getElementById("pr-meta").textContent = `${latest.branch} · latest ${formatTimestamp(
    latest.updated_at || latest.created_at
  )} · ${group.entries.length} run(s)`;
  document.getElementById("pr-run-link").href = latest.run_url;

  renderStatusBadges(latest);

  const names = benchmarkNames(group.entries);
  const benchmark = names.includes(selectedBenchmark) ? selectedBenchmark : names[0];
  renderTabs("pr-benchmark-tabs", names, benchmark, onSelectBenchmark);

  document.getElementById("pr-chart-title").textContent = benchmark;
  renderChart("pr-chart", benchmarkSeries(group.entries, benchmark), "pr-chart-caption");
  renderComparisonTable(latest, mainEntry);
  renderRunHistory(group.entries);
}

function renderDashboard(payload) {
  const allEntries = payload.entries || [];
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
    const selectedPrGroup = prGroups.find((group) => group.number === state.selectedPrNumber) || null;
    const prBenchmarkNames = selectedPrGroup ? benchmarkNames(selectedPrGroup.entries) : [];
    const availableNames = mainNames.length > 0 ? mainNames : prBenchmarkNames;

    if (!availableNames.includes(state.selectedBenchmark)) {
      state.selectedBenchmark = availableNames[0];
    }

    updateUrl(state);
    renderMeta(payload, mainRunEntries, prGroups);
    renderTabs("main-benchmark-tabs", availableNames, state.selectedBenchmark, (name) => {
      state.selectedBenchmark = name;
      render();
    });

    document.getElementById("main-chart-title").textContent = state.selectedBenchmark || "No benchmark";
    renderChart(
      "main-chart",
      benchmarkSeries(mainRunEntries, state.selectedBenchmark),
      "main-chart-caption"
    );
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
    document.body.innerHTML = `<pre>${error.message}</pre>`;
  });
