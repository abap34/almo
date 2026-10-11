const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");
const test = require("node:test");
const vm = require("node:vm");

function loadApp() {
  const elements = new Map();
  const createElement = () => ({
    children: [], attributes: {}, innerHTML: "", textContent: "",
    appendChild(child) { this.children.push(child); if (child.id) elements.set(child.id, child); },
    setAttribute(name, value) { this.attributes[name] = value; },
    getAttribute(name) { return this.attributes[name]; },
  });
  elements.set("main-charts", createElement());
  const app = vm.createContext({
    document: { getElementById: (id) => elements.get(id), createElement, createElementNS: createElement },
    fetch: () => new Promise(() => {}),
  });
  vm.runInContext(fs.readFileSync(path.join(__dirname, "app.js"), "utf8"), app);
  return { app, elements };
}

const fixture = JSON.parse(fs.readFileSync(path.join(__dirname, "fixture_dashboard.json"), "utf8"));

test("main trends include each benchmark and all available measurements", () => {
  const { app, elements } = loadApp();
  const main = app.mainEntries(fixture.entries.toReversed());
  app.renderMainCharts(main);
  const charts = elements.get("main-charts").children;
  assert.equal(charts.length, 3);
  for (let i = 0; i < charts.length; ++i) {
    const markup = elements.get(`main-chart-${i}`).innerHTML;
    assert.equal((markup.match(/class="chart-dot"/g) || []).length, 2);
    assert.ok(markup.includes("1111111"));
    assert.ok(markup.includes("2222222"));
    assert.ok(markup.includes("Apr"));
  }
});

test("a new benchmark gets a single point without invented history", () => {
  const { app, elements } = loadApp();
  const main = app.mainEntries(structuredClone(fixture.entries));
  main[1].summary.benchmarks.push({ name: "new_case", mean_ms: 0.00005 });
  app.renderMainCharts(main);
  assert.equal(elements.get("main-charts").children.length, 4);
  const markup = elements.get("main-chart-3").innerHTML;
  assert.equal((markup.match(/class="chart-dot"/g) || []).length, 1);
  assert.ok(markup.includes("0.00005"));
  assert.ok(!markup.includes("1111111"));
  const axisLabels = [...markup.matchAll(/class="chart-axis-label"[^>]*>([^<]+)</g)].map((m) => m[1]);
  assert.ok(axisLabels.every((value) => Number(value) >= 0));
  assert.ok(new Set(axisLabels).size > 1);
});

test("main and PR measurements are sorted by start time instead of completion time", () => {
  const { app } = loadApp();
  const entries = structuredClone(fixture.entries);
  entries[0].updated_at = "2026-05-01T09:00:00Z";
  entries[2].updated_at = "2026-05-01T09:00:00Z";
  assert.deepEqual(Array.from(app.mainEntries(entries), (e) => e.run_id), [9101, 9102]);
  const group = app.pullRequestGroups(entries).find((g) => g.number === 174);
  assert.deepEqual(Array.from(group.entries, (e) => e.run_id), [9201, 9202]);
  const series = app.benchmarkSeries(entries.toReversed(), "parse_small_document");
  assert.equal(series[0].timestamp, entries[0].created_at);
  entries[0].run_started_at = "2026-05-02T09:00:00Z";
  assert.deepEqual(Array.from(app.mainEntries(entries), (e) => e.run_id), [9102, 9101]);
});

test("a missing or invalid measurement is omitted from the series", () => {
  const { app } = loadApp();
  const entries = structuredClone(fixture.entries);
  entries[0].summary.benchmarks = [];
  entries[1].summary.benchmarks[0].mean_ms = null;
  assert.equal(app.benchmarkSeries(entries, "parse_small_document").length, 3);
});

test("no main results produces an explicit empty state", () => {
  const { app, elements } = loadApp();
  app.renderMainCharts([]);
  assert.equal(elements.get("main-charts").children[0].textContent, "No benchmark data");
});
