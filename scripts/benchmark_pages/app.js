async function loadHistory() {
  const response = await fetch("./history.json", { cache: "no-store" });
  if (!response.ok) {
    throw new Error("Failed to load benchmark history");
  }
  return response.json();
}

function shortCommit(commit) {
  return commit.slice(0, 7);
}

function formatMs(value) {
  return `${value.toFixed(3)} ms`;
}

function uniqueBenchmarkNames(entries) {
  const names = new Set();
  entries.forEach((entry) => {
    entry.benchmarks.forEach((benchmark) => names.add(benchmark.name));
  });
  return Array.from(names);
}

function benchmarkSeries(entries, benchmarkName) {
  return entries
    .map((entry) => {
      const benchmark = entry.benchmarks.find((item) => item.name === benchmarkName);
      if (!benchmark) {
        return null;
      }
      return {
        commit: entry.commit,
        timestamp: entry.timestamp,
        run_url: entry.run_url,
        mean_ms: benchmark.mean_ms,
      };
    })
    .filter(Boolean);
}

function renderHeader(history) {
  const latest = history.entries[history.entries.length - 1];
  document.getElementById("repo-name").textContent = history.repo;
  document.getElementById("latest-commit").innerHTML =
    latest.run_url
      ? `<a href="${latest.run_url}">${shortCommit(latest.commit)}</a>`
      : shortCommit(latest.commit);
  document.getElementById("latest-run").textContent = new Date(latest.timestamp).toLocaleString();
}

function renderLatestTable(entries) {
  const latest = entries[entries.length - 1];
  const tbody = document.getElementById("latest-table");
  tbody.innerHTML = "";

  latest.benchmarks.forEach((benchmark) => {
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

function renderTabs(names, onSelect) {
  const tabs = document.getElementById("benchmark-tabs");
  tabs.innerHTML = "";

  names.forEach((name, index) => {
    const button = document.createElement("button");
    button.className = `tab${index === 0 ? " active" : ""}`;
    button.textContent = name;
    button.type = "button";
    button.addEventListener("click", () => {
      tabs.querySelectorAll(".tab").forEach((tab) => tab.classList.remove("active"));
      button.classList.add("active");
      onSelect(name);
    });
    tabs.appendChild(button);
  });
}

function renderChart(series, benchmarkName) {
  const svg = document.getElementById("chart");
  const title = document.getElementById("chart-title");
  const caption = document.getElementById("chart-caption");
  title.textContent = benchmarkName;

  if (series.length === 0) {
    caption.textContent = "No data available";
    svg.innerHTML = "";
    return;
  }

  caption.textContent = `Tracking ${series.length} run(s)`;

  const width = 960;
  const height = 360;
  const margin = { top: 24, right: 24, bottom: 46, left: 64 };
  const innerWidth = width - margin.left - margin.right;
  const innerHeight = height - margin.top - margin.bottom;

  const values = series.map((point) => point.mean_ms);
  const min = Math.min(...values);
  const max = Math.max(...values);
  const span = Math.max(max - min, 1);

  const x = (index) =>
    margin.left + (series.length === 1 ? innerWidth / 2 : (innerWidth * index) / (series.length - 1));
  const y = (value) =>
    margin.top + innerHeight - ((value - min) / span) * innerHeight;

  const gridLines = [0, 0.25, 0.5, 0.75, 1].map((ratio) => {
    const yPos = margin.top + innerHeight * ratio;
    return `<line class="chart-grid" x1="${margin.left}" y1="${yPos}" x2="${width - margin.right}" y2="${yPos}" />`;
  });

  const path = series
    .map((point, index) => `${index === 0 ? "M" : "L"} ${x(index)} ${y(point.mean_ms)}`)
    .join(" ");

  const points = series
    .map((point, index) => {
      const xPos = x(index);
      const yPos = y(point.mean_ms);
      return `
        <a href="${point.run_url || "#"}">
          <circle class="chart-point" cx="${xPos}" cy="${yPos}" r="5" />
        </a>
        <text class="chart-label" x="${xPos}" y="${height - 14}" text-anchor="middle">${shortCommit(point.commit)}</text>
        <text class="chart-value" x="${xPos}" y="${yPos - 12}" text-anchor="middle">${point.mean_ms.toFixed(2)}</text>
      `;
    })
    .join("");

  svg.innerHTML = `
    ${gridLines.join("")}
    <line class="chart-axis" x1="${margin.left}" y1="${height - margin.bottom}" x2="${width - margin.right}" y2="${height - margin.bottom}" />
    <line class="chart-axis" x1="${margin.left}" y1="${margin.top}" x2="${margin.left}" y2="${height - margin.bottom}" />
    <path class="chart-line" d="${path}" />
    ${points}
    <text class="chart-label" x="${margin.left}" y="${margin.top - 8}">${formatMs(max)}</text>
    <text class="chart-label" x="${margin.left}" y="${height - margin.bottom + 18}">${formatMs(min)}</text>
  `;
}

loadHistory()
  .then((history) => {
    renderHeader(history);
    renderLatestTable(history.entries);

    const names = uniqueBenchmarkNames(history.entries);
    const selectBenchmark = (name) => renderChart(benchmarkSeries(history.entries, name), name);
    renderTabs(names, selectBenchmark);
    selectBenchmark(names[0]);
  })
  .catch((error) => {
    document.body.innerHTML = `<pre>${error.message}</pre>`;
  });
