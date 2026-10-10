import assert from "node:assert/strict";
import { createRequire } from "node:module";
import test from "node:test";

const require = createRequire(import.meta.url);
const mod = await require("../build_wasm/almo.js")();
const create = mod.cwrap("almo_incremental_parser_create", "number", []);
const destroy = mod.cwrap("almo_incremental_parser_destroy", null, ["number"]);
const update = mod.cwrap("almo_incremental_parser_set_source", "number", ["number", "number", "number"]);
const render = mod.cwrap("almo_render", "number", ["number", "number"]);

function withSource(source, fn) {
  const length = mod.lengthBytesUTF8(source);
  const ptr = mod._malloc(length + 1);
  assert.notEqual(ptr, 0);
  try {
    mod.stringToUTF8(source, ptr, length + 1);
    return mod.UTF8ToString(fn(ptr, length));
  } finally {
    mod._free(ptr);
  }
}
const setSource = (handle, source) => JSON.parse(withSource(source, (ptr, length) => update(handle, ptr, length)));
const normalize = (html) => html.replace(/(id=|"id":)"[0-9]+"/g, '$1"UUID"');
const headingIds = (html) => [...html.matchAll(/<h[1-6] id="([0-9]+)"/g)].map((m) => m[1]);

test("local edits reparse a subset and preserve untouched heading UUIDs", (t) => {
  const handle = create();
  assert.notEqual(handle, 0);
  t.after(() => destroy(handle));
  const before = "# First\n\nalpha **bold**\n\n# Last\n\nomega";
  const first = setSource(handle, before);
  const after = before.replace("alpha", "日本語😀");
  const second = setSource(handle, after);
  assert.equal(first.ok, true);
  assert.equal(second.ok, true);
  assert.deepEqual(headingIds(second.html), headingIds(first.html));
  assert.equal(second.reparsedRanges.length, 1);
  assert.ok(second.reparsedRanges[0].begin > 0);
  assert.ok(second.reparsedRanges[0].end < Buffer.byteLength(after));
  assert.ok(second.timings.parseMs >= 0);
  assert.ok(second.timings.renderMs >= 0);
  const unchanged = setSource(handle, after);
  assert.deepEqual(unchanged.reparsedRanges, []);
  assert.equal(unchanged.html, second.html);
});

test("incomplete tables return diagnostics and the same handle recovers", (t) => {
  const handle = create();
  t.after(() => destroy(handle));
  const good = "# 日本語😀\n\ntext";
  const first = setSource(handle, good);
  const bad = good + "\n\n| a | b |";
  const failed = setSource(handle, bad);
  assert.equal(failed.ok, false);
  assert.equal(failed.phase, "parse");
  assert.match(failed.diagnostics[0].message, /alignment/);
  assert.equal(failed.diagnostics[0].severity, "error");
  assert.equal(failed.diagnostics[0].range.begin, Buffer.byteLength(bad));
  const rolledBack = setSource(handle, good);
  assert.deepEqual(rolledBack.reparsedRanges, []);
  assert.equal(rolledBack.html, first.html);
  const fixed = setSource(handle, bad + "\n| - | - |\n| c | d |");
  assert.equal(fixed.ok, true);
  assert.match(fixed.html, /<table/);
  assert.deepEqual(fixed.diagnostics, []);
});

test("nested warnings use original UTF-8 offsets and disappear after an edit", (t) => {
  const handle = create();
  t.after(() => destroy(handle));
  const source = "日本語😀\n\n:::outer\n> > [^]: warning\n:::\n";
  const result = setSource(handle, source);
  assert.equal(result.ok, true);
  const begin = Buffer.byteLength(source.slice(0, source.indexOf("[^]")));
  assert.deepEqual(result.diagnostics[0].range, { begin, end: begin + Buffer.byteLength("[^]: warning") });
  assert.equal(result.diagnostics[0].severity, "warning");
  assert.deepEqual(setSource(handle, source.replace("[^]", "[^note]")).diagnostics, []);
});

test("independent handles keep separate trees and response buffers", (t) => {
  const a = create(), b = create();
  assert.notEqual(a, b);
  t.after(() => { destroy(a); destroy(b); });
  const ptr = update(a, 0, 0);
  const retained = mod.UTF8ToString(ptr);
  setSource(b, "# other");
  assert.equal(mod.UTF8ToString(ptr), retained);
  const first = setSource(a, "# first");
  const second = setSource(b, "# other\n\nchanged");
  assert.doesNotMatch(second.html, /first/);
  assert.equal(setSource(a, "# first").html, first.html);
});

test("incremental output matches full rendering, including moved footnotes", (t) => {
  const handle = create();
  t.after(() => destroy(handle));
  for (const source of [
    "", "# title\n\nhello **bold** and 日本語😀",
    "# title\n\ntext [^a]\n\n[^a]: first\n\n# end",
    "# title\n\ntext [^a]\n\n[^a]: changed\n\n# end",
    "| a | b |\n| - | - |\n| c | d |", "> quote\n\n- first\n- second",
    "```cpp\nint x = 42;\n```\n\n$$ x^2 $$",
  ]) {
    const result = setSource(handle, source);
    assert.equal(result.ok, true);
    assert.equal(normalize(result.html), normalize(withSource(source, render)));
    assert.equal(normalize(setSource(handle, source).html), normalize(result.html));
  }
});

test("JSON responses preserve embedded NUL bytes", (t) => {
  const handle = create();
  t.after(() => destroy(handle));
  const source = 'hello\u0000world\u0001\b\f\t"\\日本語😀';
  const result = setSource(handle, source);
  assert.equal(result.ok, true);
  assert.ok(result.html.includes(source));
});

test("render failures are distinguished from parse failures and can recover", (t) => {
  const handle = create();
  t.after(() => destroy(handle));
  const source = ":::judge\ntitle=test\nsample_in=/missing/in\nsample_out=/missing/out\nin=/missing/*.in\nout=/missing/*.out\nsource=/missing/code\n:::";
  const result = setSource(handle, source);
  assert.equal(result.ok, false);
  assert.equal(result.phase, "render");
  assert.equal(result.diagnostics[0].severity, "error");
  assert.equal(setSource(handle, "# recovered").ok, true);
});

test("destroy is idempotent, stale handles fail, and fresh handles still work", () => {
  for (let i = 0; i < 100; ++i) {
    const handle = create();
    assert.equal(setSource(handle, "# working").ok, true);
    destroy(handle);
    destroy(handle);
    assert.equal(setSource(handle, "ignored").phase, "input");
  }
  destroy(0);
  assert.equal(setSource(0, "ignored").ok, false);
});
