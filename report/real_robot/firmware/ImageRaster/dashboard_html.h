// GENERATED FILE - do not edit by hand.
// Source: dashboard/dashboard.html. After editing it, run:
//     python3 dashboard/make_header.py
#pragma once
#include <Arduino.h>

const char DASHBOARD_HTML[] PROGMEM = R"DASHBOARD(<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1, viewport-fit=cover">
<title>RasterBot Console</title>
<meta name="claude:viewport-fit-sides" content="cover">
<style>
/* Layout: instrument console. The rebuilt scan image is the hero on the left,
   run controls and settings sit in a right rail, the raw sensor trace runs below.
   No web fonts or CDNs: this page is served by the robot over WiFi with no internet. */
:root {
  --bg: #eef1f5;
  --panel: #ffffff;
  --panel-2: #f4f6f9;
  --ink: #121722;
  --ink-2: #465063;
  --ink-3: #6f7a8e;
  --line: #d8dee7;
  --accent: #2343c4;
  --accent-ink: #ffffff;
  --accent-soft: #e4e9fb;
  --path: #e0245e;
  --good: #18864a;
  --warn: #a86400;
  --bad: #c42f2f;
  --empty-a: #e9edf2;
  --empty-b: #dde3ea;
  --band: rgba(35, 67, 196, 0.07);
  --s1: #2a78d6;
  --s2: #eb6834;
  --s3: #1baf7a;
  --s4: #eda100;
  --s5: #e87ba4;
  --diff-more: #d4380d;
  --diff-less: #1f6fd1;
  --font-ui: system-ui, -apple-system, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif;
  --font-data: ui-monospace, SFMono-Regular, Menlo, Consolas, "Liberation Mono", monospace;
}
@media (prefers-color-scheme: dark) {
  :root:not([data-theme="light"]) {
    --bg: #0e1218;
    --panel: #161b24;
    --panel-2: #1c2230;
    --ink: #e8ecf3;
    --ink-2: #aab3c3;
    --ink-3: #8690a3;
    --line: #2a3242;
    --accent: #8ea4ff;
    --accent-ink: #0e1430;
    --accent-soft: #222b4a;
    --path: #ff4f86;
    --good: #46c27f;
    --warn: #e3a33a;
    --bad: #f06a6a;
    --empty-a: #1d232e;
    --empty-b: #242b37;
    --band: rgba(142, 164, 255, 0.09);
    --s1: #3987e5;
    --s2: #d95926;
    --s3: #199e70;
    --s4: #c98500;
    --s5: #d55181;
    --diff-more: #ff7a45;
    --diff-less: #4f9bff;
    color-scheme: dark;
  }
}
:root[data-theme="dark"] {
  --bg: #0e1218;
  --panel: #161b24;
  --panel-2: #1c2230;
  --ink: #e8ecf3;
  --ink-2: #aab3c3;
  --ink-3: #8690a3;
  --line: #2a3242;
  --accent: #8ea4ff;
  --accent-ink: #0e1430;
  --accent-soft: #222b4a;
  --path: #ff4f86;
  --good: #46c27f;
  --warn: #e3a33a;
  --bad: #f06a6a;
  --empty-a: #1d232e;
  --empty-b: #242b37;
  --band: rgba(142, 164, 255, 0.09);
  --s1: #3987e5;
  --s2: #d95926;
  --s3: #199e70;
  --s4: #c98500;
  --s5: #d55181;
  --diff-more: #ff7a45;
  --diff-less: #4f9bff;
  color-scheme: dark;
}

* { box-sizing: border-box; }
[hidden] { display: none !important; }
body {
  margin: 0;
  background: var(--bg);
  color: var(--ink);
  font: 14px/1.45 var(--font-ui);
  -webkit-font-smoothing: antialiased;
}
.wrap {
  max-width: 1320px;
  margin: 0 auto;
  padding-block: 18px 36px;
  padding-left: max(16px, env(safe-area-inset-left, 0px));
  padding-right: max(16px, env(safe-area-inset-right, 0px));
  display: grid;
  gap: 16px;
}

/* ---------- header ---------- */
.top { display: flex; flex-wrap: wrap; align-items: center; gap: 10px 18px; }
.brand { display: flex; align-items: baseline; gap: 10px; flex-wrap: wrap; margin-right: auto; }
.brand h1 { margin: 0; font-size: 21px; font-weight: 650; letter-spacing: -0.01em; text-wrap: balance; }
.brand span { color: var(--ink-3); font-size: 13px; }
.chip {
  display: inline-flex; align-items: center; gap: 7px;
  padding: 4px 10px; border-radius: 999px;
  border: 1px solid var(--line); background: var(--panel);
  font: 12px/1.3 var(--font-data); color: var(--ink-2); white-space: nowrap;
}
.dot { width: 8px; height: 8px; border-radius: 50%; background: var(--ink-3); flex: none; }
.chip.ok .dot { background: var(--good); }
.chip.off .dot { background: var(--bad); }
.chip.demo .dot { background: var(--warn); }

.seg { display: inline-flex; border: 1px solid var(--line); border-radius: 8px; overflow: hidden; background: var(--panel); }
.seg button {
  border: 0; background: transparent; color: var(--ink-2);
  font: 600 12px/1 var(--font-ui); padding: 8px 12px; cursor: pointer;
}
.seg button + button { border-left: 1px solid var(--line); }
.seg button[aria-pressed="true"] { background: var(--accent-soft); color: var(--accent); }
.seg button:disabled { opacity: 0.4; cursor: not-allowed; }

/* ---------- main grid ---------- */
.grid { display: grid; grid-template-columns: minmax(0, 1fr) 340px; gap: 16px; align-items: start; }
.main { display: grid; gap: 16px; min-width: 0; }
.panel { background: var(--panel); border: 1px solid var(--line); border-radius: 10px; min-width: 0; }
.panel-head {
  display: flex; flex-wrap: wrap; align-items: center; gap: 8px 14px;
  padding: 12px 14px; border-bottom: 1px solid var(--line);
}
.panel-head h2 { margin: 0; font-size: 15px; font-weight: 650; margin-right: auto; }
.panel-body { padding: 14px; }
.label {
  font: 600 11px/1.2 var(--font-data); letter-spacing: 0.08em; text-transform: uppercase; color: var(--ink-3);
}
.mono { font-family: var(--font-data); font-variant-numeric: tabular-nums; }
.muted { color: var(--ink-3); }

/* ---------- image ---------- */
.stage { position: relative; padding: 14px; }
#img-canvas { display: block; width: 100%; border-radius: 6px; cursor: crosshair; touch-action: none; }
.empty-note {
  position: absolute; inset: 14px; display: grid; place-content: center; gap: 6px;
  text-align: center; padding: 24px; pointer-events: none;
}
.empty-note strong { font-size: 16px; }
.empty-note p { margin: 0; color: var(--ink-2); max-width: 46ch; }
.readout {
  display: flex; flex-wrap: wrap; gap: 6px 18px; padding: 0 14px 14px;
  font: 12px/1.4 var(--font-data); color: var(--ink-2); font-variant-numeric: tabular-nums;
}
.readout b { color: var(--ink); font-weight: 600; }
.check { display: inline-flex; align-items: center; gap: 6px; font-size: 13px; color: var(--ink-2); cursor: pointer; }

/* ---------- sensor chart ---------- */
.legend { display: flex; flex-wrap: wrap; gap: 6px 14px; font: 12px/1.3 var(--font-data); color: var(--ink-2); }
.legend i { display: inline-block; width: 14px; height: 3px; border-radius: 2px; vertical-align: middle; margin-right: 6px; }
.chart-wrap { position: relative; padding: 6px 14px 14px; }
#chart-canvas { display: block; width: 100%; height: 210px; }
.tip {
  position: absolute; pointer-events: none; z-index: 2;
  background: var(--panel); border: 1px solid var(--line); border-radius: 8px;
  padding: 8px 10px; font: 12px/1.5 var(--font-data); color: var(--ink);
  box-shadow: 0 6px 20px rgba(0, 0, 0, 0.12); white-space: nowrap; font-variant-numeric: tabular-nums;
}
.tip .sw { display: inline-block; width: 8px; height: 8px; border-radius: 2px; margin-right: 6px; }

/* ---------- side rail ---------- */
.rail { display: grid; gap: 16px; min-width: 0; }
.state-row { display: flex; align-items: center; gap: 10px; flex-wrap: wrap; }
.pill {
  display: inline-flex; align-items: center; gap: 6px;
  padding: 5px 10px; border-radius: 6px; font: 700 12px/1 var(--font-data); letter-spacing: 0.04em;
  background: var(--panel-2); color: var(--ink-2); border: 1px solid var(--line);
}
.pill.run { background: var(--accent-soft); color: var(--accent); border-color: transparent; }
.pill.done { color: var(--good); }
.pill.stop { color: var(--bad); }
.stats { display: grid; grid-template-columns: repeat(3, minmax(0, 1fr)); gap: 10px; margin: 14px 0; }
.stat .v { font: 600 18px/1.2 var(--font-data); font-variant-numeric: tabular-nums; margin-top: 3px; }
.meter { height: 4px; border-radius: 2px; background: var(--panel-2); overflow: hidden; margin-top: 6px; }
.meter span { display: block; height: 100%; width: 0; background: var(--accent); }
.msg { font-size: 13px; color: var(--ink-2); min-height: 1.4em; }
.msg.err { color: var(--bad); }
.btns { display: flex; flex-wrap: wrap; gap: 8px; margin-top: 12px; }
button.btn {
  font: 600 13px/1 var(--font-ui); padding: 10px 14px; border-radius: 8px; cursor: pointer;
  border: 1px solid var(--line); background: var(--panel); color: var(--ink);
}
button.btn:hover:not(:disabled) { border-color: var(--ink-3); }
button.btn.primary { background: var(--accent); border-color: var(--accent); color: var(--accent-ink); }
button.btn.danger { color: var(--bad); }
button.btn:disabled { opacity: 0.45; cursor: not-allowed; }
button:focus-visible, input:focus-visible, select:focus-visible, label.file:focus-within {
  outline: 2px solid var(--accent); outline-offset: 2px;
}
.note { font-size: 12.5px; color: var(--ink-2); background: var(--panel-2); border-radius: 8px; padding: 10px 12px; margin-top: 12px; }
.note code { font-family: var(--font-data); color: var(--ink); }

.bars { display: grid; grid-template-columns: repeat(5, minmax(0, 1fr)); gap: 10px; align-items: end; }
.bar { display: grid; gap: 6px; justify-items: center; }
.bar .track { width: 100%; height: 96px; background: var(--panel-2); border-radius: 4px; position: relative; overflow: hidden; }
.bar .fill { position: absolute; left: 0; right: 0; bottom: 0; height: 0; border-radius: 4px 4px 0 0; transition: height 0.2s; }
.bar .n { font: 600 12px/1 var(--font-data); color: var(--ink-2); }
.bar .val { font: 12px/1 var(--font-data); color: var(--ink); font-variant-numeric: tabular-nums; }

.fields { display: grid; grid-template-columns: repeat(2, minmax(0, 1fr)); gap: 10px 12px; }
.fields.three { grid-template-columns: repeat(3, minmax(0, 1fr)); }
form.groups { display: grid; gap: 10px; }
.sw-dot { display: inline-block; width: 8px; height: 8px; border-radius: 2px; margin-right: 6px; }
table.geo td { padding: 3px 6px; }
table.geo input {
  width: 6.5em; font: 13px/1.2 var(--font-data); color: var(--ink); text-align: right;
  background: var(--panel-2); border: 1px solid var(--line); border-radius: 6px; padding: 5px 6px;
}
form.groups .fields { margin-bottom: 6px; }
.field { display: grid; gap: 4px; min-width: 0; }
.field.wide { grid-column: 1 / -1; }
.field label { font-size: 12px; color: var(--ink-2); }
.field input, .field select {
  width: 100%; min-width: 0; font: 13px/1.2 var(--font-data); color: var(--ink);
  background: var(--panel-2); border: 1px solid var(--line); border-radius: 6px; padding: 7px 8px;
}
.field input:disabled, .field select:disabled { opacity: 0.6; }
.sub { margin: 16px 0 8px; }
table.cal { width: 100%; border-collapse: collapse; font: 12px/1.4 var(--font-data); font-variant-numeric: tabular-nums; }
table.cal th { text-align: left; font-weight: 600; color: var(--ink-3); padding: 4px 6px; border-bottom: 1px solid var(--line); }
table.cal td { padding: 4px 6px; border-bottom: 1px solid var(--line); color: var(--ink-2); }
table.cal td:first-child { color: var(--ink); }
table.cal td.num, table.cal th.num { text-align: right; }
.data-row { display: flex; flex-wrap: wrap; gap: 8px; align-items: center; }
label.file { position: relative; overflow: hidden; display: inline-flex; }
label.file input { position: absolute; inset: 0; opacity: 0; cursor: pointer; }
label.file span {
  font: 600 13px/1 var(--font-ui); padding: 10px 14px; border-radius: 8px;
  border: 1px solid var(--line); background: var(--panel); color: var(--ink); cursor: pointer;
}
/* ---------- template comparison ---------- */
.tpl-fields { margin-top: 12px; }
.tpl-grid { display: grid; grid-template-columns: repeat(3, minmax(0, 1fr)); gap: 12px; margin-top: 14px; }
.tpl-grid figure { margin: 0; min-width: 0; display: grid; gap: 6px; }
.tpl-grid figcaption { font: 600 12px/1.3 var(--font-data); color: var(--ink-2); display: flex; flex-wrap: wrap; gap: 4px 10px; align-items: center; }
.tpl-grid canvas { display: block; width: 100%; border-radius: 6px; background: var(--empty-a); }
.tpl-key { font-weight: 400; color: var(--ink-3); display: inline-flex; align-items: center; gap: 4px 6px; flex-wrap: wrap; }
.tpl-key i { display: inline-block; width: 10px; height: 10px; border-radius: 2px; }
.tpl-key i.more { background: var(--diff-more); }
.tpl-key i.less { background: var(--diff-less); margin-left: 6px; }
.tpl-msg { font-size: 12px; margin: 0; padding: 0 14px 14px; }
.tpl-msg code { font-family: var(--font-data); color: var(--ink-2); }
@media (max-width: 640px) { .tpl-grid, .tpl-fields { grid-template-columns: minmax(0, 1fr); } }
@media (prefers-reduced-motion: reduce) { .bar .fill { transition: none; } }
@media (max-width: 940px) {
  /* One column: image first, then run controls, then the rest. */
  .grid { grid-template-columns: minmax(0, 1fr); }
  .main, .rail { display: contents; }
  #sec-img { order: 1; }
  #sec-run { order: 2; }
  #sec-tpl { order: 3; }
  #sec-chart { order: 3; }
  .rail > section { order: 4; }
  .rail > #sec-run { order: 2; }
}
</style>
</head>
<body>
<div class="wrap">
  <header class="top">
    <div class="brand">
      <h1>RasterBot Console</h1>
      <span>3Pi+ surface-sensor image rasterisation</span>
    </div>
    <div class="seg" id="view-seg" role="group" aria-label="Data shown">
      <button type="button" id="view-live" data-view="live" aria-pressed="false">Live robot</button>
      <button type="button" id="view-demo" data-view="demo" aria-pressed="true">Demo</button>
      <button type="button" id="view-file" data-view="file" aria-pressed="false" hidden>File</button>
    </div>
    <span class="chip" id="conn-chip"><span class="dot"></span><span id="conn-text">Checking robot…</span></span>
  </header>

  <div class="grid">
    <div class="main">
      <!-- Rebuilt image -->
      <section class="panel" id="sec-img" aria-labelledby="img-title">
        <div class="panel-head">
          <h2 id="img-title">Rebuilt image</h2>
          <span class="label" id="img-source">Demo data</span>
          <label class="check"><input type="checkbox" id="opt-path" checked> Robot path</label>
          <label class="check"><input type="checkbox" id="opt-smooth"> Smooth pixels</label>
        </div>
        <div class="stage" id="stage">
          <canvas id="img-canvas" aria-label="Image rebuilt from the surface sensor readings"></canvas>
          <div class="empty-note" id="empty-note" hidden>
            <strong>No scan data yet</strong>
            <p>Place the robot facing along the first row with its sensor bar on the start edge of your printed image, then press <b>Start scan</b> or the StampC3 button.</p>
          </div>
        </div>
        <div class="readout">
          <span>Area <b id="ro-size">–</b></span>
          <span>Cells <b id="ro-cells">–</b></span>
          <span>Coverage <b id="ro-cov">–</b></span>
          <span>Samples used <b id="ro-used">–</b></span>
          <span>Row gaps <b id="ro-rows">–</b></span>
          <span id="ro-hover">Hover the image for a reading</span>
        </div>
      </section>

      <!-- Template comparison -->
      <section class="panel" id="sec-tpl" aria-labelledby="tpl-title">
        <div class="panel-head">
          <h2 id="tpl-title">Compare with your template</h2>
          <span class="label" id="tpl-name">No template</span>
        </div>
        <div class="panel-body">
          <div class="data-row">
            <label class="file"><input type="file" id="tpl-in" accept=".png,.jpg,.jpeg,.svg,.webp,.pdf,image/png,image/jpeg,image/svg+xml,image/webp,application/pdf"><span>Load template…</span></label>
            <button type="button" class="btn primary" id="tpl-align" disabled>Auto-align</button>
            <label class="check"><input type="checkbox" id="tpl-scale" checked> Fit scale too</label>
            <label class="check"><input type="checkbox" id="tpl-blur"> Blur template like the sensor</label>
            <button type="button" class="btn" id="tpl-clear" disabled>Remove</button>
          </div>
          <div class="fields three tpl-fields">
            <div class="field"><label for="tpl-w">Printed picture width (mm)</label><input id="tpl-w" type="number" min="5" max="2000" step="0.5" disabled></div>
            <div class="field"><label for="tpl-x">Left edge at x (mm)</label><input id="tpl-x" type="number" step="0.5" disabled></div>
            <div class="field"><label for="tpl-y">Bottom edge at y (mm)</label><input id="tpl-y" type="number" step="0.5" disabled></div>
          </div>
          <div class="tpl-grid">
            <figure><figcaption>Template</figcaption><canvas id="tpl-c-ref" aria-label="Your template over the scanned area"></canvas></figure>
            <figure><figcaption>Robot scan</figcaption><canvas id="tpl-c-scan" aria-label="The robot's rebuilt image"></canvas></figure>
            <figure><figcaption>Difference <span class="tpl-key"><i class="more"></i>extra ink <i class="less"></i>missed ink</span></figcaption><canvas id="tpl-c-diff" aria-label="Where the scan and the template differ"></canvas></figure>
          </div>
        </div>
        <div class="readout">
          <span>Error vs template <b id="tpl-err">–</b></span>
          <span>Match (correlation) <b id="tpl-corr">–</b></span>
          <span>Compared <b id="tpl-area">–</b></span>
          <span>Scale x / y <b id="tpl-sc">–</b></span>
        </div>
        <p class="muted tpl-msg" id="tpl-msg">Load the picture you printed (PNG, JPG or SVG) to see it next to the robot's scan. Sheets from <code>tools/make_sheet.py</code>: load <code>&lt;name&gt;_template.png</code> and it lines up by itself.</p>
      </section>

      <!-- Sensor traces -->
      <section class="panel" id="sec-chart" aria-labelledby="chart-title">
        <div class="panel-head">
          <h2 id="chart-title">Surface sensor readings</h2>
          <div class="legend" id="legend"></div>
        </div>
        <div class="chart-wrap" id="chart-wrap">
          <canvas id="chart-canvas" aria-label="DN1 to DN5 readings over time"></canvas>
          <div class="tip" id="chart-tip" hidden></div>
        </div>
      </section>
    </div>

    <aside class="rail">
      <!-- Run -->
      <section class="panel" id="sec-run" aria-labelledby="run-title">
        <div class="panel-head"><h2 id="run-title">Scan run</h2><span class="label" id="run-id">–</span></div>
        <div class="panel-body">
          <div class="state-row">
            <span class="pill" id="state-pill">WAITING</span>
            <span class="mono muted" id="pose-text"></span>
          </div>
          <div class="stats">
            <div class="stat"><div class="label">Row</div><div class="v" id="st-row">–</div></div>
            <div class="stat"><div class="label">Time</div><div class="v" id="st-time">–</div></div>
            <div class="stat"><div class="label">Samples</div><div class="v" id="st-n">–</div>
              <div class="meter" title="Robot log memory used"><span id="st-meter"></span></div></div>
          </div>
          <div class="msg" id="run-msg" role="status" aria-live="polite"></div>
          <div class="btns" id="robot-btns">
            <button type="button" class="btn primary" id="btn-start">Start scan</button>
            <button type="button" class="btn danger" id="btn-stop">Stop</button>
            <button type="button" class="btn" id="btn-clear">Clear robot log</button>
          </div>
          <div class="btns" id="demo-btns" hidden>
            <button type="button" class="btn primary" id="btn-replay">Replay demo scan</button>
          </div>
          <div class="note" id="offline-note" hidden>
            Not connected to a robot, so this shows simulated data. For live scanning, join the robot's WiFi and open <code>http://192.168.4.1:8080</code>.
          </div>
        </div>
      </section>

      <!-- Live sensors -->
      <section class="panel" aria-labelledby="live-title">
        <div class="panel-head"><h2 id="live-title">Sensors now</h2><span class="label">raw, higher = darker</span></div>
        <div class="panel-body"><div class="bars" id="bars"></div></div>
      </section>

      <!-- Robot settings -->
      <section class="panel" aria-labelledby="cfg-title">
        <div class="panel-head"><h2 id="cfg-title">Scan settings</h2><span class="label" id="cfg-target">robot</span></div>
        <div class="panel-body">
          <form id="cfg-form" autocomplete="off" class="groups">
            <div class="label">Pattern</div>
            <div class="fields">
              <div class="field wide"><label for="cfg-pattern">Path</label>
                <select id="cfg-pattern"><option value="0">Back and forth, no turns (sharper)</option><option value="1">U-turns at row ends</option></select></div>
              <div class="field"><label for="cfg-rows">Rows</label><input id="cfg-rows" type="number" min="1" max="150" step="1" value="17"></div>
              <div class="field"><label for="cfg-row_len">Row length (mm)</label><input id="cfg-row_len" type="number" min="20" max="1500" step="1" value="160"></div>
              <div class="field"><label for="cfg-pitch">Row pitch (mm)</label><input id="cfg-pitch" type="number" min="2" max="300" step="0.5" value="7"></div>
              <div class="field"><label for="cfg-side">Next row is on</label>
                <select id="cfg-side"><option value="1">the left</option><option value="-1">the right</option></select></div>
              <div class="field" id="fld-margin"><label for="cfg-margin">Run-in past row ends (mm)</label><input id="cfg-margin" type="number" min="0" max="200" step="1" value="50"></div>
            </div>
            <p class="muted" id="scan-est" style="font-size:12px;margin:0"></p>
            <div class="label">Motors (PWM)</div>
            <div class="fields three">
              <div class="field"><label for="cfg-scan_pwm">Scan</label><input id="cfg-scan_pwm" type="number" min="5" max="120" step="1" value="30"></div>
              <div class="field" id="fld-turn_pwm"><label for="cfg-turn_pwm">Turn max</label><input id="cfg-turn_pwm" type="number" min="5" max="120" step="1" value="30"></div>
              <div class="field" id="fld-turn_min"><label for="cfg-turn_min">Turn min</label><input id="cfg-turn_min" type="number" min="0" max="120" step="1" value="24"></div>
            </div>
            <div class="label">Geometry (mm, from your calibration)</div>
            <div class="fields">
              <div class="field" id="fld-lead"><label for="cfg-lead">DN3 ahead of axle (row overrun)</label><input id="cfg-lead" type="number" min="0" max="80" step="0.5" value="40"></div>
              <div class="field"><label for="cfg-sep">Wheel separation</label><input id="cfg-sep" type="number" min="20" max="200" step="0.1" value="82.5"></div>
              <div class="field"><label for="cfg-r_left">Left wheel radius</label><input id="cfg-r_left" type="number" min="5" max="50" step="0.01" value="16"></div>
              <div class="field"><label for="cfg-r_right">Right wheel radius</label><input id="cfg-r_right" type="number" min="5" max="50" step="0.01" value="16"></div>
            </div>
          </form>
          <div class="btns">
            <button type="button" class="btn primary" id="btn-apply">Send to robot</button>
            <span class="msg" id="cfg-msg"></span>
          </div>
        </div>
      </section>

      <!-- Image settings -->
      <section class="panel" aria-labelledby="rec-title">
        <div class="panel-head"><h2 id="rec-title">Image settings</h2><span class="label">this page only</span></div>
        <div class="panel-body">
          <form class="fields" id="rec-form" autocomplete="off">
            <div class="field"><label for="opt-cell">Cell size (mm)</label><input id="opt-cell" type="number" min="0.5" max="20" step="0.5" value="1"></div>
            <div class="field"><label for="opt-foot">Sensor footprint (mm)</label><input id="opt-foot" type="number" min="0" max="40" step="0.5" value="4"></div>
            <div class="field"><label for="opt-delay">Sensor delay (ms)</label><input id="opt-delay" type="number" min="-100" max="200" step="5" value="0"></div>
            <div class="field"><label for="opt-tone">Tone</label>
              <select id="opt-tone"><option value="smooth">Smooth (raw tones)</option><option value="contrast">High contrast</option><option value="bw">Black and white</option></select></div>
            <div class="field"><label for="opt-deblur">Deblur</label>
              <select id="opt-deblur"><option value="0">Off</option><option value="8">Light</option><option value="20" selected>Strong</option></select></div>
            <div class="field"><label for="opt-blur">Sensor blur σ (mm)</label><input id="opt-blur" type="number" min="0.5" max="10" step="0.25" value="3"></div>
            <div class="field wide">
              <div class="data-row">
                <button type="button" class="btn primary" id="btn-autoalign">Auto-calibrate from this scan</button>
              </div>
              <span class="muted" style="font-size:12px" id="align-msg">Fits the sensor delay and the DN1/DN5 and DN2/DN4 positions so that overlapping rows agree.</span>
            </div>
            <div class="field wide">
              <span class="label" style="margin-top:4px">Sensor positions (mm from axle centre)</span>
              <div style="overflow-x:auto">
                <table class="cal geo" id="geo-table">
                  <tr><th>Use</th><th>Sensor</th><th class="num">Ahead</th><th class="num">Left (+) / right (−)</th></tr>
                  <tr><td><input type="checkbox" id="geo-u1" checked aria-label="Use DN1 in the image"></td><td><span class="sw-dot" style="background:var(--s1)"></span>DN1</td><td class="num"><input id="geo-f1" type="number" step="0.5" aria-label="DN1 ahead of axle (mm)"></td><td class="num"><input id="geo-l1" type="number" step="0.5" aria-label="DN1 left of centre (mm)"></td></tr>
                  <tr><td><input type="checkbox" id="geo-u2" checked aria-label="Use DN2 in the image"></td><td><span class="sw-dot" style="background:var(--s2)"></span>DN2</td><td class="num"><input id="geo-f2" type="number" step="0.5" aria-label="DN2 ahead of axle (mm)"></td><td class="num"><input id="geo-l2" type="number" step="0.5" aria-label="DN2 left of centre (mm)"></td></tr>
                  <tr><td><input type="checkbox" id="geo-u3" checked aria-label="Use DN3 in the image"></td><td><span class="sw-dot" style="background:var(--s3)"></span>DN3</td><td class="num"><input id="geo-f3" type="number" step="0.5" aria-label="DN3 ahead of axle (mm)"></td><td class="num"><input id="geo-l3" type="number" step="0.5" aria-label="DN3 left of centre (mm)"></td></tr>
                  <tr><td><input type="checkbox" id="geo-u4" checked aria-label="Use DN4 in the image"></td><td><span class="sw-dot" style="background:var(--s4)"></span>DN4</td><td class="num"><input id="geo-f4" type="number" step="0.5" aria-label="DN4 ahead of axle (mm)"></td><td class="num"><input id="geo-l4" type="number" step="0.5" aria-label="DN4 left of centre (mm)"></td></tr>
                  <tr><td><input type="checkbox" id="geo-u5" checked aria-label="Use DN5 in the image"></td><td><span class="sw-dot" style="background:var(--s5)"></span>DN5</td><td class="num"><input id="geo-f5" type="number" step="0.5" aria-label="DN5 ahead of axle (mm)"></td><td class="num"><input id="geo-l5" type="number" step="0.5" aria-label="DN5 left of centre (mm)"></td></tr>
                </table>
              </div>
              <div class="data-row" style="margin-top:6px">
                <button type="button" class="btn" id="btn-geo-reset">Reset to 3Pi+ layout</button>
                <span class="muted" style="font-size:12px" id="pitch-hint"></span>
              </div>
            </div>
            <div class="field wide"><label for="opt-calib">Brightness scaling</label>
              <select id="opt-calib">
                <option value="auto">Each sensor scaled to its own range</option>
                <option value="raw">One shared range (shows sensor differences)</option>
              </select></div>
            <div class="field wide"><label for="opt-use">Samples to paint</label>
              <select id="opt-use">
                <option value="scan">Scan rows only</option>
                <option value="moving">Everything while moving</option>
                <option value="all">Everything logged</option>
              </select></div>
          </form>
          <p class="muted" style="font-size:12px;margin:10px 0 0">The 3Pi+ sensors are not in a straight bar: DN2–DN4 sit close together at the front, and DN1/DN5 sit further out and further back. The defaults were measured from a photo of the underside (±2 mm), so check them with a ruler. The axle centre is midway between the wheel centres. Footprint is the width each reading is spread over. These settings are remembered in this browser.</p>
          <div class="label sub">Per-sensor range used (2nd–98th percentile)</div>
          <div style="overflow-x:auto"><table class="cal" id="cal-table"></table></div>
        </div>
      </section>

      <!-- Data -->
      <section class="panel" aria-labelledby="data-title">
        <div class="panel-head"><h2 id="data-title">Data</h2></div>
        <div class="panel-body">
          <div class="data-row">
            <label class="file"><input type="file" id="file-in" accept=".csv,.txt,text/csv,text/plain"><span>Load CSV…</span></label>
            <button type="button" class="btn" id="btn-csv">Download CSV</button>
            <button type="button" class="btn" id="btn-png">Save image PNG</button>
          </div>
          <p class="muted" style="font-size:12px;margin:10px 0 0" id="data-note">Loads this page's CSV export or the template's 14-column Serial log.</p>
        </div>
      </section>
    </aside>
  </div>
</div>

<script>
(() => {
  'use strict';

  // ------------------------------------------------------------------ setup
  const $ = (id) => document.getElementById(id);
  // Must match Controller_c::Mode in Controller.h
  const STATE_NAMES = ['WAITING', 'SETTLING', 'SCANNING_ROW', 'TURNING_OUT', 'SHIFTING', 'TURNING_IN', 'FINISHED', 'STOPPED', 'OVERRUN', 'MARGIN'];
  const SCAN_ROW = 2;
  const MOVING = new Set([2, 3, 4, 5, 8, 9]);
  const RUNNING = new Set([1, 2, 3, 4, 5, 8, 9]);
  const SERIES = ['--s1', '--s2', '--s3', '--s4', '--s5'];

  const sets = {
    live: { samples: [], run: 0, nextSeq: 0, lost: 0, name: 'Live robot data' },
    demo: { samples: [], full: [], run: 0, name: 'Demo data (simulated)' },
    file: { samples: [], run: 0, name: '' },
  };
  let view = 'demo';
  let base = null;            // robot URL prefix ('' when this page came from the robot)
  let connected = false;
  let robotStatus = null;
  let cfgDirty = false;
  let rec = null;             // last reconstruction
  let imgXform = null;        // world -> canvas mapping of last draw
  let chartXform = null;

  const isHttps = location.protocol === 'https:';
  if (location.protocol === 'http:' && location.port === '8080') base = '';
  else if (location.protocol === 'file:') base = 'http://192.168.4.1:8080';
  else if (location.protocol === 'http:') base = 'http://192.168.4.1:8080';

  const css = (name) => getComputedStyle(document.documentElement).getPropertyValue(name).trim();

  // Downloads are blocked inside a hosted preview; they work from the robot or a local file.
  if (isHttps) {
    $('btn-csv').hidden = true;
    $('btn-png').hidden = true;
    $('data-note').textContent = 'Loads this page\'s CSV export or the template\'s 14-column Serial log. Downloads work when the page is opened from the robot.';
  }

  // ---------------------------------------------------------- small helpers
  const fmt = (v, d = 0) => (Number.isFinite(v) ? v.toFixed(d) : '–');
  const clamp = (v, a, b) => Math.min(b, Math.max(a, v));
  function niceStep(range, target) {
    const raw = range / Math.max(1, target);
    const p = Math.pow(10, Math.floor(Math.log10(raw)));
    const m = raw / p;
    return (m < 1.5 ? 1 : m < 3.5 ? 2 : m < 7.5 ? 5 : 10) * p;
  }
  function percentile(sorted, q) {
    if (!sorted.length) return NaN;
    const i = clamp(Math.round(q * (sorted.length - 1)), 0, sorted.length - 1);
    return sorted[i];
  }
  function setMsg(el, text, isErr) {
    el.textContent = text || '';
    el.classList.toggle('err', !!isErr);
  }

  // ------------------------------------------------------------ options
  // 3Pi+ sensor layout, measured from a photo of the underside: [ahead, left] in mm.
  // DN1 is the robot's left-most sensor (Pololu numbering).
  const LAYOUT_3PI = [[28, 27], [40, 9], [41, 0], [40, -9], [28, -27]];
  function setLayout(layout) {
    layout.forEach(([f, l], i) => { $('geo-f' + (i + 1)).value = String(f); $('geo-l' + (i + 1)).value = String(l); });
  }
  function readLayout() {
    return LAYOUT_3PI.map(([f0, l0], i) => {
      const f = parseFloat($('geo-f' + (i + 1)).value), l = parseFloat($('geo-l' + (i + 1)).value);
      return [Number.isFinite(f) ? clamp(f, -80, 80) : f0, Number.isFinite(l) ? clamp(l, -80, 80) : l0];
    });
  }
  function imageOpts() {
    const num = (id, d) => { const v = parseFloat($(id).value); return Number.isFinite(v) ? v : d; };
    return {
      cell: clamp(num('opt-cell', 1), 0.5, 20),
      foot: clamp(num('opt-foot', 4), 0, 40),
      delay: clamp(num('opt-delay', 0), -100, 200),
      tone: $('opt-tone').value,
      deblur: parseInt($('opt-deblur').value, 10) || 0,
      blur: clamp(num('opt-blur', 3), 0.5, 10),
      useSensor: [1, 2, 3, 4, 5].map((i) => $('geo-u' + i).checked),
      sensors: readLayout(),
      calib: $('opt-calib').value,
      use: $('opt-use').value,
      path: $('opt-path').checked,
      smooth: $('opt-smooth').checked,
    };
  }
  const CFG_KEYS = ['pattern', 'margin', 'rows', 'row_len', 'pitch', 'side', 'scan_pwm', 'turn_pwm', 'turn_min', 'lead', 'sep', 'r_left', 'r_right'];
  function readCfg() {
    const c = {};
    for (const k of CFG_KEYS) c[k] = parseFloat($('cfg-' + k).value);
    return c;
  }
  function writeCfg(c) {
    for (const k of CFG_KEYS) if (c && c[k] !== undefined) $('cfg-' + k).value = String(c[k]);
  }
  // Sensor positions in the robot frame (x forward, y left). DN1 is the left-most.
  function sensorOffsets(o) {
    return o.sensors;
  }

  // Row pitch that makes DN2-DN4 tracks interleave evenly with the next row.
  function updatePitchHint() {
    const L = readLayout();
    const gap = (Math.abs(L[1][1] - L[2][1]) + Math.abs(L[2][1] - L[3][1])) / 2;
    $('pitch-hint').textContent = gap > 0.5
      ? `Even coverage: row pitch ${fmt(gap * 0.75, 2)} mm (≈${fmt(gap / 4, 1)} mm detail), ${fmt(gap * 1.5, 1)} mm (≈${fmt(gap / 2, 1)} mm) or ${fmt(gap * 3, 1)} mm (fastest, ≈${fmt(gap, 1)} mm).`
      : '';
  }

  // Remember the image settings in this browser (per-viewer convenience only).
  const STORE_KEY = 'rasterbot-image-settings-v4';
  const STORE_IDS = ['opt-cell', 'opt-foot', 'opt-delay', 'opt-tone', 'opt-deblur', 'opt-blur', 'opt-calib', 'opt-use', 'opt-path', 'opt-smooth',
    'geo-u1', 'geo-u2', 'geo-u3', 'geo-u4', 'geo-u5',
    'geo-f1', 'geo-l1', 'geo-f2', 'geo-l2', 'geo-f3', 'geo-l3', 'geo-f4', 'geo-l4', 'geo-f5', 'geo-l5'];
  function saveImageSettings() {
    try {
      const v = {};
      for (const id of STORE_IDS) v[id] = $(id).type === 'checkbox' ? $(id).checked : $(id).value;
      localStorage.setItem(STORE_KEY, JSON.stringify(v));
    } catch (e) { /* storage unavailable */ }
  }
  function loadImageSettings() {
    setLayout(LAYOUT_3PI);
    try {
      let v = JSON.parse(localStorage.getItem(STORE_KEY) || 'null');
      if (!v) {
        // Keep sensor positions saved by the previous version of this page.
        const old = JSON.parse(localStorage.getItem('rasterbot-image-settings-v3') || localStorage.getItem('rasterbot-image-settings-v2') || 'null');
        if (!old) return;
        v = {};
        for (const k of Object.keys(old)) if (k.startsWith('geo-')) v[k] = old[k];
      }
      for (const id of STORE_IDS) {
        if (!(id in v)) continue;
        if ($(id).type === 'checkbox') $(id).checked = !!v[id]; else $(id).value = v[id];
      }
    } catch (e) { /* storage unavailable */ }
  }

  // --------------------------------------------------------- reconstruction
  function reconstruct(samples, o) {
    const unknownState = samples.length && samples[0].state < 0;
    const used = samples.filter((s) => {
      if (s.state < 0 || o.use === 'all') return true;
      if (o.use === 'moving') return MOVING.has(s.state);
      return s.state === SCAN_ROW;
    });
    if (!used.length) return null;

    // Brightness scaling: robust low/high per sensor (or shared).
    const per = [[], [], [], [], []];
    for (const s of used) for (let i = 0; i < 5; i++) per[i].push(s.dn[i]);
    per.forEach((a) => a.sort((p, q) => p - q));
    const ranges = per.map((a) => ({ lo: percentile(a, 0.02), hi: percentile(a, 0.98) }));
    // A sensor whose track never crossed ink has a tiny range: stretching that
    // to full black would turn its noise into fake dark streaks. Give it the
    // typical range of the sensors that did see ink instead.
    const spans = ranges.map((r) => r.hi - r.lo);
    const maxSpan = Math.max(...spans);
    const goodSpans = spans.filter((s) => s >= 0.35 * maxSpan).sort((p, q) => p - q);
    const typical = goodSpans.length ? goodSpans[Math.floor(goodSpans.length / 2)] : maxSpan;
    let scale = ranges.map((r, i) => (spans[i] >= 0.35 * maxSpan ? r : { lo: r.lo, hi: r.lo + typical, weak: true }));
    if (o.calib === 'raw') {
      const all = per.flat().sort((p, q) => p - q);
      const shared = { lo: percentile(all, 0.02), hi: percentile(all, 0.98) };
      scale = ranges.map(() => shared);
    }

    // Each reading belongs to where the robot was `delay` ms before it was
    // logged (sensor read time + transfer). Interpolate that pose along the path.
    const poses = posesAtDelay(samples, used, o.delay || 0);

    // Sensor points in world coordinates.
    const offs = sensorOffsets(o);
    const active = [0, 1, 2, 3, 4].filter((i) => !o.useSensor || o.useSensor[i]);
    if (!active.length) return null;
    const n = used.length * active.length;
    const px = new Float32Array(n), py = new Float32Array(n), pd = new Float32Array(n);
    let k = 0, x0 = Infinity, x1 = -Infinity, y0 = Infinity, y1 = -Infinity;
    for (let u = 0; u < used.length; u++) {
      const s = used[u];
      const P = poses[u];
      const c = Math.cos(P.th), sn = Math.sin(P.th);
      for (const i of active) {
        const [fx, ly] = offs[i];
        const X = P.x + fx * c - ly * sn;
        const Y = P.y + fx * sn + ly * c;
        const span = scale[i].hi - scale[i].lo;
        const d = span > 1e-6 ? clamp((s.dn[i] - scale[i].lo) / span, 0, 1) : 0;
        px[k] = X; py[k] = Y; pd[k] = d; k++;
        if (X < x0) x0 = X; if (X > x1) x1 = X; if (Y < y0) y0 = Y; if (Y > y1) y1 = Y;
      }
    }

    // Grid (cap the cell count so very large scans stay responsive).
    const margin = Math.max(o.foot / 2, o.cell);
    let cell = o.cell;
    const spanX = x1 - x0 + 2 * margin, spanY = y1 - y0 + 2 * margin;
    const MAX_CELLS = 260000;
    if ((spanX / cell) * (spanY / cell) > MAX_CELLS) cell = Math.sqrt((spanX * spanY) / MAX_CELLS);
    const W = Math.max(1, Math.ceil(spanX / cell));
    const H = Math.max(1, Math.ceil(spanY / cell));
    const minX = x0 - margin, minY = y0 - margin;
    const sum = new Float32Array(W * H), wsum = new Float32Array(W * H), sum2 = new Float32Array(W * H);

    const sig = Math.max(o.foot / 2.355, 0.05);
    const inv2s2 = 1 / (2 * sig * sig);
    const rad = o.foot <= cell ? 0 : Math.ceil(o.foot / cell);
    const foot2 = o.foot * o.foot;
    for (let p = 0; p < n; p++) {
      const fi = (px[p] - minX) / cell - 0.5, fj = (py[p] - minY) / cell - 0.5;
      const ci = Math.round(fi), cj = Math.round(fj);
      if (rad === 0) {
        if (ci >= 0 && ci < W && cj >= 0 && cj < H) { sum[cj * W + ci] += pd[p]; sum2[cj * W + ci] += pd[p] * pd[p]; wsum[cj * W + ci] += 1; }
        continue;
      }
      for (let j = cj - rad; j <= cj + rad; j++) {
        if (j < 0 || j >= H) continue;
        const dy = (j - fj) * cell;
        for (let i = ci - rad; i <= ci + rad; i++) {
          if (i < 0 || i >= W) continue;
          const dx = (i - fi) * cell;
          const d2 = dx * dx + dy * dy;
          if (d2 > foot2) continue;
          const w = Math.exp(-d2 * inv2s2);
          sum[j * W + i] += w * pd[p];
          sum2[j * W + i] += w * pd[p] * pd[p];
          wsum[j * W + i] += w;
        }
      }
    }
    const val = new Float32Array(W * H);
    const thresh = rad === 0 ? 0.5 : 0.2;
    let filled = 0, inside = 0, varSum = 0, wTot = 0;
    for (let j = 0; j < H; j++) {
      const cy = minY + (j + 0.5) * cell;
      for (let i = 0; i < W; i++) {
        const idx = j * W + i;
        const ok = wsum[idx] >= thresh;
        val[idx] = ok ? sum[idx] / wsum[idx] : NaN;
        // Disagreement between the readings that landed in this cell.
        if (ok) { varSum += sum2[idx] - (sum[idx] * sum[idx]) / wsum[idx]; wTot += wsum[idx]; }
        const cx = minX + (i + 0.5) * cell;
        if (cx >= x0 && cx <= x1 && cy >= y0 && cy <= y1) { inside++; if (ok) filled++; }
      }
    }
    return { W, H, cell, minX, minY, val, wsum, ranges, scale, used: used.length, coverage: inside ? filled / inside : 0, unknownState,
      disagreement: wTot ? varSum / wTot : 0 };
  }

  // Pose of each used sample `delay` ms earlier, interpolated over all samples.
  function posesAtDelay(all, used, delay) {
    if (!delay) return used.map((s) => s);
    const out = new Array(used.length);
    let j = 0;
    for (let u = 0; u < used.length; u++) {
      const t = used[u].t - delay;
      while (j < all.length - 2 && all[j + 1].t <= t) j++;
      while (j > 0 && all[j].t > t) j--;
      const a = all[j], b = all[Math.min(j + 1, all.length - 1)];
      if (b === a || b.t - a.t > 250 || t <= a.t) { out[u] = t <= a.t ? a : (t >= b.t ? b : a); continue; }
      const f = clamp((t - a.t) / (b.t - a.t), 0, 1);
      out[u] = { x: a.x + (b.x - a.x) * f, y: a.y + (b.y - a.y) * f, th: a.th + (b.th - a.th) * f };
    }
    return out;
  }

  // Richardson-Lucy deconvolution of the rebuilt image. The blur being undone is
  // the sensor's own spot (sigma = o.blur) combined with the footprint splat.
  function gaussKernel(sigma) {
    const r = Math.max(1, Math.ceil(3 * sigma));
    const k = new Float32Array(2 * r + 1);
    let t = 0;
    for (let i = -r; i <= r; i++) { k[i + r] = Math.exp(-(i * i) / (2 * sigma * sigma)); t += k[i + r]; }
    for (let i = 0; i < k.length; i++) k[i] /= t;
    return k;
  }
  function convolve(src, W, H, k) {
    const r = (k.length - 1) >> 1, tmp = new Float32Array(W * H), out = new Float32Array(W * H);
    for (let j = 0; j < H; j++) {
      const row = j * W;
      for (let i = 0; i < W; i++) {
        let a = 0;
        for (let q = -r; q <= r; q++) { const ii = i + q; if (ii >= 0 && ii < W) a += src[row + ii] * k[q + r]; }
        tmp[row + i] = a;
      }
    }
    for (let j = 0; j < H; j++) {
      for (let i = 0; i < W; i++) {
        let a = 0;
        for (let q = -r; q <= r; q++) { const jj = j + q; if (jj >= 0 && jj < H) a += tmp[jj * W + i] * k[q + r]; }
        out[j * W + i] = a;
      }
    }
    return out;
  }
  function deblur(rec, o) {
    const { W, H, val } = rec, N = W * H, EPS = 0.02;
    const sigma = Math.sqrt(o.blur * o.blur + Math.pow(o.foot / 2.355, 2)) / rec.cell;
    const k = gaussKernel(sigma);
    const mask = new Float32Array(N), data = new Float32Array(N);
    for (let i = 0; i < N; i++) { const ok = !Number.isNaN(val[i]); mask[i] = ok ? 1 : 0; data[i] = ok ? val[i] + EPS : 0; }
    const mconv = convolve(mask, W, H, k);
    const blurM = (x) => {
      const m = new Float32Array(N);
      for (let i = 0; i < N; i++) m[i] = x[i] * mask[i];
      const c = convolve(m, W, H, k);
      for (let i = 0; i < N; i++) c[i] = mconv[i] > 1e-6 ? c[i] / mconv[i] : 0;
      return c;
    };
    let est = data.slice();
    const ratio = new Float32Array(N);
    for (let it = 0; it < o.deblur; it++) {
      const cv = blurM(est);
      for (let i = 0; i < N; i++) ratio[i] = mask[i] ? data[i] / Math.max(cv[i], 1e-4) : 1;
      const corr = blurM(ratio);
      // Darkness can't exceed 100%: keeping the estimate in range stops ringing inside big shapes.
      for (let i = 0; i < N; i++) if (mask[i]) est[i] = clamp(est[i] * corr[i], EPS * 0.5, 1 + EPS);
    }
    const out = new Float32Array(N);
    for (let i = 0; i < N; i++) out[i] = mask[i] ? clamp(est[i] - EPS, 0, 1) : NaN;
    return out;
  }
  // Rebuilding and deblurring are the slow parts: reuse them while nothing changed.
  let recCache = { key: '', rec: null, show: null };
  function rebuild(ds, o) {
    const S = ds.samples;
    const key = [view, ds.run, S.length, S.length ? S[S.length - 1].t : 0, JSON.stringify(o)].join('|');
    if (recCache.key === key) return recCache;
    const r = S.length ? reconstruct(S, o) : null;
    let show = r ? r.val : null;
    const scanning = view === 'live' && robotStatus && RUNNING.has(robotStatus.mode);
    if (r && o.deblur > 0 && !scanning) show = deblur(r, o);
    recCache = { key, rec: r, show };
    return recCache;
  }

  // Tone curve applied when drawing (0 = white, 1 = black).
  function toneMap(v, tone) {
    if (tone === 'bw') return v >= 0.5 ? 1 : 0;
    if (tone === 'contrast') {
      const k = 9, lo = 1 / (1 + Math.exp(k * 0.5)), hi = 1 / (1 + Math.exp(-k * 0.5));
      return (1 / (1 + Math.exp(-k * (v - 0.5))) - lo) / (hi - lo);
    }
    return v;
  }

  // ------------------------------------------------------------- image draw
  const imgCanvas = $('img-canvas');
  const off = document.createElement('canvas');
  function hatchPattern(ctx, dpr) {
    const t = document.createElement('canvas');
    const s = Math.round(8 * dpr);
    t.width = t.height = s;
    const g = t.getContext('2d');
    g.fillStyle = css('--empty-a'); g.fillRect(0, 0, s, s);
    g.strokeStyle = css('--empty-b'); g.lineWidth = Math.max(1, dpr * 1.5);
    g.beginPath(); g.moveTo(-1, s + 1); g.lineTo(s + 1, -1); g.stroke();
    return ctx.createPattern(t, 'repeat');
  }

  function drawImage() {
    const ds = sets[view];
    const o = imageOpts();
    const built = rebuild(ds, o);
    rec = built.rec;
    const shown = built.show;
    const dpr = window.devicePixelRatio || 1;
    const stage = $('stage');
    const cssW = Math.max(200, stage.clientWidth - 28);
    const maxH = Math.max(260, Math.min(620, window.innerHeight * 0.66));
    $('empty-note').hidden = !!(rec || ds.samples.length);

    // View bounds: the rebuilt grid, plus the robot path when it is shown.
    let vx0 = 0, vx1 = 1, vy0 = 0, vy1 = 1;
    if (rec) {
      vx0 = rec.minX; vx1 = rec.minX + rec.W * rec.cell;
      vy0 = rec.minY; vy1 = rec.minY + rec.H * rec.cell;
      if (o.path) {
        const S = ds.samples, pad = 8;
        const step = Math.max(1, Math.floor(S.length / 4000));
        for (let i = 0; i < S.length; i += step) {
          const s = S[i];
          if (s.x - pad < vx0) vx0 = s.x - pad; if (s.x + pad > vx1) vx1 = s.x + pad;
          if (s.y - pad < vy0) vy0 = s.y - pad; if (s.y + pad > vy1) vy1 = s.y + pad;
        }
      }
    }
    let cssH = rec ? Math.min(maxH, cssW * ((vy1 - vy0) / (vx1 - vx0))) : Math.min(maxH, cssW * 0.6);
    cssH = Math.max(cssH, 220);
    imgCanvas.style.height = cssH + 'px';
    imgCanvas.width = Math.round(cssW * dpr);
    imgCanvas.height = Math.round(cssH * dpr);
    const ctx = imgCanvas.getContext('2d');
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    ctx.fillStyle = hatchPattern(ctx, 1);
    ctx.fillRect(0, 0, cssW, cssH);

    updateReadout(o);
    renderCalTable();
    if (!rec) { imgXform = null; return; }

    const worldW = vx1 - vx0, worldH = vy1 - vy0;
    const sc = Math.min(cssW / worldW, cssH / worldH);
    const ox = (cssW - worldW * sc) / 2, oy = (cssH - worldH * sc) / 2;
    imgXform = { sc, ox, oy, minX: vx0, maxY: vy1 };
    const toPx = (x, y) => [ox + (x - vx0) * sc, oy + (vy1 - y) * sc];

    // Cells -> offscreen bitmap (row 0 of the bitmap is the top = largest y).
    off.width = rec.W; off.height = rec.H;
    const octx = off.getContext('2d');
    const im = octx.createImageData(rec.W, rec.H);
    for (let j = 0; j < rec.H; j++) {
      const r = rec.H - 1 - j;
      for (let i = 0; i < rec.W; i++) {
        const v = shown[j * rec.W + i];
        const q = (r * rec.W + i) * 4;
        if (Number.isNaN(v)) { im.data[q + 3] = 0; continue; }
        const g = Math.round(255 * (1 - toneMap(v, o.tone)));
        im.data[q] = g; im.data[q + 1] = g; im.data[q + 2] = g; im.data[q + 3] = 255;
      }
    }
    octx.putImageData(im, 0, 0);
    ctx.imageSmoothingEnabled = o.smooth;
    const [gx, gy] = toPx(rec.minX, rec.minY + rec.H * rec.cell);
    ctx.drawImage(off, gx, gy, rec.W * rec.cell * sc, rec.H * rec.cell * sc);

    // Overlay: path, start, robot outline at last pose.
    if (o.path) {
      const S = ds.samples;
      ctx.lineWidth = 1.5; ctx.strokeStyle = css('--path'); ctx.globalAlpha = 0.85;
      ctx.beginPath();
      const step = Math.max(1, Math.floor(S.length / 4000));
      for (let i = 0; i < S.length; i += step) {
        const [X, Y] = toPx(S[i].x, S[i].y);
        if (i === 0) ctx.moveTo(X, Y); else ctx.lineTo(X, Y);
      }
      ctx.stroke(); ctx.globalAlpha = 1;
      const [sx, sy] = toPx(S[0].x, S[0].y);
      ctx.fillStyle = css('--path');
      ctx.beginPath(); ctx.arc(sx, sy, 4, 0, Math.PI * 2); ctx.fill();
      ctx.font = '600 11px ' + css('--font-data');
      ctx.fillText('start', sx + 7, sy - 6);

      const last = S[S.length - 1];
      const [lx, ly] = toPx(last.x, last.y);
      ctx.strokeStyle = css('--path'); ctx.lineWidth = 1.5;
      ctx.setLineDash([4, 3]);
      ctx.beginPath(); ctx.arc(lx, ly, 48.5 * sc, 0, Math.PI * 2); ctx.stroke();
      ctx.setLineDash([]);
      const c = Math.cos(last.th), s = Math.sin(last.th);
      sensorOffsets(o).forEach(([fx, fy], i) => {
        const [X, Y] = toPx(last.x + fx * c - fy * s, last.y + fx * s + fy * c);
        ctx.fillStyle = css(SERIES[i]);
        ctx.beginPath(); ctx.arc(X, Y, 3.5, 0, Math.PI * 2); ctx.fill();
      });
    }

    // Scale bar (bottom-left).
    const target = (cssW * 0.18) / sc;
    const len = [5, 10, 20, 25, 50, 100, 200, 250, 500].reduce((b, v) => (Math.abs(v - target) < Math.abs(b - target) ? v : b), 50);
    const bw = len * sc, bx = cssW - bw - 14, by = cssH - 14;
    ctx.fillStyle = css('--panel'); ctx.globalAlpha = 0.85;
    ctx.fillRect(bx - 6, by - 20, bw + 12, 28); ctx.globalAlpha = 1;
    ctx.fillStyle = css('--ink');
    ctx.fillRect(bx, by - 3, bw, 3);
    ctx.font = '600 11px ' + css('--font-data');
    ctx.fillText(len + ' mm', bx, by - 7);
  }

  function updateReadout(o) {
    const ds = sets[view];
    $('img-source').textContent = view === 'file' ? 'File: ' + ds.name : ds.name;
    if (!rec) {
      ['ro-size', 'ro-cells', 'ro-cov', 'ro-used'].forEach((id) => ($(id).textContent = '–'));
      return;
    }
    $('ro-size').textContent = `${fmt(rec.W * rec.cell)} × ${fmt(rec.H * rec.cell)} mm`;
    $('ro-cells').textContent = `${rec.W} × ${rec.H} @ ${fmt(rec.cell, rec.cell % 1 ? 1 : 0)} mm`;
    $('ro-cov').textContent = fmt(rec.coverage * 100) + '%';
    $('ro-used').textContent = `${rec.used} of ${ds.samples.length}` + (rec.unknownState ? ' (no state column)' : '');
    $('ro-rows').textContent = rowGapText(ds.samples);
  }

  // How far apart the scanned rows really were (from odometry), vs the target pitch.
  function rowGapText(S) {
    const byRow = new Map();
    for (const s of S) {
      if (s.state !== SCAN_ROW) continue;
      if (!byRow.has(s.row)) byRow.set(s.row, []);
      byRow.get(s.row).push(s.y);
    }
    const rows = [...byRow.keys()].sort((a, b) => a - b);
    if (rows.length < 2) return '–';
    const med = (a) => { const b = a.slice().sort((p, q) => p - q); return b[Math.floor(b.length / 2)]; };
    const ys = rows.map((r) => med(byRow.get(r)));
    const gaps = [];
    for (let i = 1; i < ys.length; i++) gaps.push(Math.abs(ys[i] - ys[i - 1]));
    const lo = Math.min(...gaps), hi = Math.max(...gaps);
    const target = view === 'live' && robotStatus && robotStatus.cfg ? robotStatus.cfg.pitch : readCfg().pitch;
    return `${fmt(lo, 1)}–${fmt(hi, 1)} mm (target ${fmt(target, 1)})`;
  }

  // Rough samples/time a scan will need, so it can be checked against the robot's log size.
  function measuredSpeed(S) {
    let d = 0, t = 0;
    for (let i = 1; i < S.length; i++) {
      const a = S[i - 1], b = S[i];
      if (a.state !== SCAN_ROW || b.state !== SCAN_ROW || a.row !== b.row) continue;
      const dt = b.t - a.t;
      if (dt <= 0 || dt > 60) continue;
      d += Math.hypot(b.x - a.x, b.y - a.y); t += dt;
    }
    return t > 2000 ? (d / t) * 1000 : null;
  }
  function updateEstimate() {
    const c = readCfg();
    if (Object.values(c).some((v) => !Number.isFinite(v))) { $('scan-est').textContent = ''; return; }
    const measured = measuredSpeed(sets.live.samples) || measuredSpeed(sets[view].samples);
    const v = measured || 55;                      // mm/s
    const R = Math.max(1, Math.round(c.rows));
    let samples, secs;
    if (c.pattern === 0) {
      const rowT = c.row_len / v, runT = (2 * c.margin) / v;
      samples = R * rowT * 50 + (R - 1) * runT * 25;
      secs = R * rowT + (R - 1) * (runT + 0.3) + 1;
    } else {
      const rowT = (c.row_len + 2 * c.lead) / v, shiftT = c.pitch / v, turnT = 1.6;
      samples = R * rowT * 50 + (R - 1) * (shiftT * 50 + 2 * turnT * 25);
      secs = R * rowT + (R - 1) * (shiftT + 2 * turnT + 1.2) + 1;
    }
    const cap = robotStatus && robotStatus.cap ? robotStatus.cap : null;
    const mins = secs >= 90 ? `${Math.floor(secs / 60)} min ${Math.round(secs % 60)} s` : `${Math.round(secs)} s`;
    let txt = `About ${mins} and ${Math.round(samples / 50) * 50} samples` + (measured ? ' (speed from your last scan).' : '.');
    const big = cap && samples > 0.92 * cap;
    if (big) txt += ` That's more than the robot's ${cap}-sample memory, so keep this page open on the robot's WiFi until the scan ends.`;
    $('scan-est').textContent = txt;
    $('scan-est').style.color = big ? 'var(--warn)' : '';
  }
  function syncPatternFields() {
    const uturn = $('cfg-pattern').value === '1';
    $('fld-margin').hidden = uturn;
    $('fld-lead').hidden = !uturn;
    $('fld-turn_pwm').hidden = !uturn;
    $('fld-turn_min').hidden = !uturn;
  }

  function renderCalTable() {
    const t = $('cal-table');
    if (!rec) { t.innerHTML = '<tr><td class="muted">No data</td></tr>'; return; }
    let h = '<tr><th>Sensor</th><th class="num">White</th><th class="num">Black</th><th class="num">Span</th></tr>';
    let weak = 0;
    rec.ranges.forEach((r, i) => {
      const w = rec.scale[i] && rec.scale[i].weak;
      if (w) weak++;
      h += `<tr><td><span class="sw-dot" style="background:var(${SERIES[i]})"></span>DN${i + 1}</td>` +
        `<td class="num">${fmt(r.lo)}</td><td class="num">${fmt(r.hi)}</td><td class="num">${fmt(r.hi - r.lo)}${w ? ' *' : ''}</td></tr>`;
    });
    if (weak) h += `<tr><td colspan="4" class="muted">* Barely saw ink in this scan, so it uses the typical range of the others.</td></tr>`;
    t.innerHTML = h;
  }

  imgCanvas.addEventListener('pointermove', (e) => {
    if (!rec || !imgXform) return;
    const b = imgCanvas.getBoundingClientRect();
    const X = imgXform.minX + (e.clientX - b.left - imgXform.ox) / imgXform.sc;
    const Y = imgXform.maxY - (e.clientY - b.top - imgXform.oy) / imgXform.sc;
    const i = Math.floor((X - rec.minX) / rec.cell), j = Math.floor((Y - rec.minY) / rec.cell);
    let txt = `x ${fmt(X)} mm · y ${fmt(Y)} mm`;
    if (i >= 0 && i < rec.W && j >= 0 && j < rec.H) {
      const v = rec.val[j * rec.W + i];
      txt += Number.isNaN(v) ? ' · not scanned' : ` · darkness ${fmt(v, 2)}`;
    }
    $('ro-hover').textContent = txt;
  });
  imgCanvas.addEventListener('pointerleave', () => { $('ro-hover').textContent = 'Hover the image for a reading'; });

  // ------------------------------------------------------------- chart draw
  const chartCanvas = $('chart-canvas');
  function drawChart() {
    const S = sets[view].samples;
    const dpr = window.devicePixelRatio || 1;
    const W = chartCanvas.clientWidth, H = chartCanvas.clientHeight;
    chartCanvas.width = Math.round(W * dpr); chartCanvas.height = Math.round(H * dpr);
    const ctx = chartCanvas.getContext('2d');
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    ctx.clearRect(0, 0, W, H);
    const L = 46, R = 10, T = 8, B = 24;
    const pw = W - L - R, ph = H - T - B;
    ctx.font = '11px ' + css('--font-data');
    if (S.length < 2) {
      ctx.fillStyle = css('--ink-3');
      ctx.fillText('Readings appear here while the robot scans.', L, T + ph / 2);
      chartXform = null;
      return;
    }
    const t0 = S[0].t, t1 = S[S.length - 1].t;
    let ymax = 0;
    for (const s of S) for (const v of s.dn) if (v > ymax) ymax = v;
    const ystep = niceStep(ymax, 4);
    ymax = Math.max(ystep, Math.ceil(ymax / ystep) * ystep);
    const xs = (t) => L + ((t - t0) / Math.max(1, t1 - t0)) * pw;
    const ys = (v) => T + ph - (v / ymax) * ph;
    chartXform = { xs, t0, t1, L, pw, T, ph, ys };

    // Bands: where the robot was scanning a row.
    ctx.fillStyle = css('--band');
    let bandStart = null;
    for (let i = 0; i <= S.length; i++) {
      const on = i < S.length && S[i].state === SCAN_ROW;
      if (on && bandStart === null) bandStart = S[i].t;
      if (!on && bandStart !== null) { const a = xs(bandStart), b = xs(S[i - 1].t); ctx.fillRect(a, T, Math.max(1, b - a), ph); bandStart = null; }
    }

    // Grid + y labels.
    ctx.strokeStyle = css('--line'); ctx.lineWidth = 1; ctx.fillStyle = css('--ink-3');
    ctx.textAlign = 'right'; ctx.textBaseline = 'middle';
    for (let v = 0; v <= ymax + 1e-6; v += ystep) {
      const y = Math.round(ys(v)) + 0.5;
      ctx.beginPath(); ctx.moveTo(L, y); ctx.lineTo(L + pw, y); ctx.stroke();
      ctx.fillText(String(Math.round(v)), L - 6, y);
    }
    // x labels (seconds).
    const dur = (t1 - t0) / 1000;
    const xstep = niceStep(dur, Math.max(2, Math.floor(pw / 90)));
    ctx.textAlign = 'center'; ctx.textBaseline = 'top';
    for (let s = 0; s <= dur + 1e-6; s += xstep) ctx.fillText(fmt(s, xstep < 1 ? 1 : 0) + ' s', xs(t0 + s * 1000), T + ph + 6);

    // Lines (decimate to about two points per pixel).
    const step = Math.max(1, Math.floor(S.length / (pw * 2)));
    ctx.lineWidth = 1.5; ctx.lineJoin = 'round';
    for (let k = 0; k < 5; k++) {
      ctx.strokeStyle = css(SERIES[k]);
      ctx.beginPath();
      for (let i = 0; i < S.length; i += step) {
        const x = xs(S[i].t), y = ys(S[i].dn[k]);
        if (i === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y);
      }
      ctx.stroke();
    }
    if (chartHoverT !== null) drawChartHover(ctx);
  }

  let chartHoverT = null;
  function nearestIndex(S, t) {
    let lo = 0, hi = S.length - 1;
    while (hi - lo > 1) { const m = (lo + hi) >> 1; if (S[m].t < t) lo = m; else hi = m; }
    return Math.abs(S[lo].t - t) <= Math.abs(S[hi].t - t) ? lo : hi;
  }
  function drawChartHover(ctx) {
    const S = sets[view].samples;
    if (!chartXform || S.length < 2) return;
    const i = nearestIndex(S, chartHoverT);
    const s = S[i];
    const x = chartXform.xs(s.t);
    ctx.strokeStyle = css('--ink-3'); ctx.lineWidth = 1;
    ctx.beginPath(); ctx.moveTo(Math.round(x) + 0.5, chartXform.T); ctx.lineTo(Math.round(x) + 0.5, chartXform.T + chartXform.ph); ctx.stroke();
    for (let k = 0; k < 5; k++) {
      const y = chartXform.ys(s.dn[k]);
      ctx.fillStyle = css('--panel'); ctx.beginPath(); ctx.arc(x, y, 5, 0, Math.PI * 2); ctx.fill();
      ctx.fillStyle = css(SERIES[k]); ctx.beginPath(); ctx.arc(x, y, 3.5, 0, Math.PI * 2); ctx.fill();
    }
    const tip = $('chart-tip');
    const stateName = s.state >= 0 ? STATE_NAMES[s.state] || '?' : '';
    let h = `<div class="muted">${fmt((s.t - S[0].t) / 1000, 2)} s ${stateName ? '· ' + stateName : ''}</div>`;
    for (let k = 0; k < 5; k++) h += `<div><span class="sw" style="background:var(${SERIES[k]})"></span>DN${k + 1} ${s.dn[k]}</div>`;
    tip.innerHTML = h;
    tip.hidden = false;
    const wrap = $('chart-wrap');
    const left = x + 14 + 14 > wrap.clientWidth - 160 ? x + 14 - 150 : x + 14 + 14;
    tip.style.left = clamp(left, 4, wrap.clientWidth - 150) + 'px';
    tip.style.top = '10px';
  }
  chartCanvas.addEventListener('pointermove', (e) => {
    const S = sets[view].samples;
    if (!chartXform || S.length < 2) return;
    const b = chartCanvas.getBoundingClientRect();
    const px = clamp(e.clientX - b.left, chartXform.L, chartXform.L + chartXform.pw);
    chartHoverT = chartXform.t0 + ((px - chartXform.L) / chartXform.pw) * (chartXform.t1 - chartXform.t0);
    drawChart();
  });
  chartCanvas.addEventListener('pointerleave', () => { chartHoverT = null; $('chart-tip').hidden = true; drawChart(); });

  // ------------------------------------------------------------ side panels
  function buildStatic() {
    $('legend').innerHTML = SERIES.map((v, i) => `<span><i style="background:var(${v})"></i>DN${i + 1}</span>`).join('');
    $('bars').innerHTML = SERIES.map((v, i) =>
      `<div class="bar"><div class="track"><div class="fill" id="bar-${i}" style="background:var(${v})"></div></div>` +
      `<div class="val" id="barv-${i}">–</div><div class="n">DN${i + 1}</div></div>`).join('');
  }
  function setBars(dn) {
    const top = Math.max(2500, ...(dn || [0]));
    for (let i = 0; i < 5; i++) {
      const v = dn ? dn[i] : NaN;
      $('bar-' + i).style.height = Number.isFinite(v) ? (100 * v / top) + '%' : '0';
      $('barv-' + i).textContent = Number.isFinite(v) ? String(v) : '–';
    }
  }
  function setPill(stateIdx) {
    const p = $('state-pill');
    const name = stateIdx >= 0 ? STATE_NAMES[stateIdx] || 'UNKNOWN' : 'NO STATE';
    p.textContent = name.replace('_', ' ');
    p.className = 'pill' + (RUNNING.has(stateIdx) ? ' run' : stateIdx === 6 ? ' done' : stateIdx === 7 ? ' stop' : '');
  }

  function renderRunPanel() {
    const ds = sets[view];
    const S = ds.samples;
    const live = view === 'live';
    $('robot-btns').hidden = !live;
    $('demo-btns').hidden = view !== 'demo';
    $('offline-note').hidden = connected || view === 'file';
    $('cfg-target').textContent = live ? 'robot' : view === 'demo' ? 'demo' : 'robot';
    $('btn-apply').textContent = view === 'demo' && !connected ? 'Run demo with these' : 'Send to robot';
    $('btn-apply').disabled = view === 'file' && !connected;

    const st = live && robotStatus;
    const running = st && RUNNING.has(robotStatus.mode);
    $('btn-start').disabled = !connected || running;
    $('btn-stop').disabled = !connected;
    $('btn-clear').disabled = !connected || running;
    for (const k of CFG_KEYS) $('cfg-' + k).disabled = !!running;

    if (st) {
      setPill(robotStatus.mode);
      const rows = robotStatus.cfg ? robotStatus.cfg.rows : 0;
      $('st-row').textContent = RUNNING.has(robotStatus.mode) || robotStatus.mode === 6 ? `${robotStatus.row + 1}/${rows}` : `–/${rows}`;
      $('pose-text').textContent = `x ${fmt(robotStatus.x)} · y ${fmt(robotStatus.y)} mm · heading ${fmt(robotStatus.th * 180 / Math.PI)}°`;
      $('st-n').textContent = String(robotStatus.n);
      $('st-meter').style.width = robotStatus.cap ? (100 * robotStatus.n / robotStatus.cap) + '%' : '0';
      $('run-id').textContent = robotStatus.run ? 'Run ' + robotStatus.run : 'No run yet';
      const lost = sets.live.lost;
      setMsg($('run-msg'), lost
        ? `${robotStatus.msg}. ${lost} samples were overwritten before this page downloaded them (page closed or WiFi dropped during a long scan).`
        : robotStatus.msg, robotStatus.mode === 7 || lost > 0);
      setBars(robotStatus.dn);
    } else {
      const last = S[S.length - 1];
      const demoDone = view === 'demo' && S.length && S.length >= ds.full.length;
      setPill(demoDone ? 6 : last ? last.state : 0);
      const rows = S.length ? Math.max(...S.map((s) => s.row)) + 1 : 0;
      $('st-row').textContent = S.length ? (last.state < 0 ? '–' : `${last.row + 1}/${view === 'demo' ? readCfg().rows : rows}`) : '–';
      $('pose-text').textContent = last ? `x ${fmt(last.x)} · y ${fmt(last.y)} mm · heading ${fmt(last.th * 180 / Math.PI)}°` : '';
      $('st-n').textContent = String(S.length);
      $('st-meter').style.width = view === 'demo' && ds.full.length ? (100 * S.length / ds.full.length) + '%' : '100%';
      $('run-id').textContent = view === 'demo' ? 'Simulated' : view === 'file' ? 'From file' : 'Not connected';
      if (view !== 'live') setMsg($('run-msg'), view === 'demo' ? 'Simulated robot with slightly mismatched wheels and five unevenly calibrated sensors.' : `${S.length} samples loaded.`);
      else setMsg($('run-msg'), 'Waiting for the robot…');
      setBars(last ? last.dn : null);
    }
    const S2 = ds.samples;
    $('st-time').textContent = S2.length > 1 ? fmt((S2[S2.length - 1].t - S2[0].t) / 1000, 1) + ' s' : '–';
    $('view-live').disabled = !connected && !sets.live.samples.length;
    for (const b of document.querySelectorAll('#view-seg button')) b.setAttribute('aria-pressed', String(b.dataset.view === view));
  }

  function setConnChip() {
    const chip = $('conn-chip');
    chip.className = 'chip ' + (connected ? 'ok' : base === null ? 'demo' : 'off');
    $('conn-text').textContent = connected
      ? 'Robot connected' + (base ? ' · ' + base.replace('http://', '') : '')
      : base === null ? 'Preview: no robot link' : 'Robot not reachable';
  }

  // ---------------------------------------------------------- render loop
  let renderQueued = false, lastRender = 0;
  function render(force) {
    if (renderQueued) return;
    renderQueued = true;
    const wait = force ? 0 : Math.max(0, 200 - (performance.now() - lastRender));
    setTimeout(() => requestAnimationFrame(() => {
      renderQueued = false; lastRender = performance.now();
      drawImage(); drawTemplate(); drawChart(); renderRunPanel();
    }), wait);
  }
  function setView(v) {
    view = v;
    chartHoverT = null; $('chart-tip').hidden = true;
    render(true);
  }

  // ----------------------------------------------------------- robot link
  async function robotFetch(path) {
    const ctl = new AbortController();
    const timer = setTimeout(() => ctl.abort(), 2500);
    try {
      const r = await fetch(base + path, { cache: 'no-store', signal: ctl.signal });
      return r;
    } finally { clearTimeout(timer); }
  }

  let statusFails = 0;
  async function pollStatus() {
    try {
      const r = await robotFetch('/status');
      robotStatus = await r.json();
      statusFails = 0;
      if (!connected) {
        connected = true;
        if (view === 'demo') view = 'live';
        stopReplay();
      }
      if (!cfgDirty && robotStatus.cfg) { writeCfg(robotStatus.cfg); syncPatternFields(); }
      updateEstimate();
    } catch (e) {
      statusFails++;
      if (statusFails >= 3 && connected) { connected = false; robotStatus = null; }
    }
    setConnChip();
    render();
    setTimeout(pollStatus, connected ? 500 : 2000);
  }

  async function pollData() {
    let more = false;
    if (connected) {
      try {
        // The robot keeps a ring buffer and this page downloads it while the
        // robot scans, so a scan can be longer than the robot's memory.
        const live = sets.live;
        const r = await robotFetch('/data?from=' + live.nextSeq);
        const lines = (await r.text()).split('\n');
        const head = Object.fromEntries((lines[0] || '').replace('#', '').split(',').map((kv) => kv.split('=')));
        const run = +head.run, n = +head.n;
        if (run !== live.run) {
          // New run: start again from its first sample.
          const first = live.run === 0 && live.nextSeq === 0;
          live.run = run; live.samples = []; live.nextSeq = 0; live.lost = 0;
          if (run > 0 && view !== 'live') view = 'live';
          if (!first) { more = true; throw new Error('restart'); }
        }
        if (n < live.nextSeq) { live.samples = []; live.nextSeq = 0; live.lost = 0; }  // robot log was cleared
        for (let i = 1; i < lines.length; i++) {
          const s = parseRow(lines[i].split(','), 'robot');
          if (!s || s.seq < live.nextSeq) continue;
          if (s.seq > live.nextSeq) live.lost += s.seq - live.nextSeq;   // overwritten before we got them
          live.samples.push(s);
          live.nextSeq = s.seq + 1;
        }
        more = live.nextSeq < n;
        if (view === 'live') render();
      } catch (e) { /* retry on next poll */ }
    }
    setTimeout(pollData, more ? 40 : 300);
  }

  async function robotCommand(path, msgEl, okText) {
    try {
      const r = await robotFetch(path);
      const j = await r.json();
      if (!r.ok || j.ok === false) { setMsg(msgEl, j.error || 'The robot refused that.', true); return j; }
      setMsg(msgEl, okText);
      return j;
    } catch (e) {
      setMsg(msgEl, 'No reply from the robot. Check you are on its WiFi.', true);
      return null;
    }
  }

  $('btn-start').addEventListener('click', () => { setView('live'); robotCommand('/start', $('run-msg'), 'Starting in 1 s. Hands off the robot.'); });
  $('btn-stop').addEventListener('click', () => robotCommand('/stop', $('run-msg'), 'Stop sent.'));
  $('btn-clear').addEventListener('click', async () => {
    const j = await robotCommand('/clear', $('run-msg'), 'Robot log cleared.');
    if (j && j.ok) { sets.live.samples = []; sets.live.nextSeq = 0; sets.live.lost = 0; render(true); }
  });
  $('cfg-form').addEventListener('input', () => { cfgDirty = true; setMsg($('cfg-msg'), 'Not sent yet'); syncPatternFields(); updateEstimate(); });
  $('cfg-form').addEventListener('change', () => { syncPatternFields(); updateEstimate(); });
  $('btn-apply').addEventListener('click', async () => {
    const c = readCfg();
    if (Object.values(c).some((v) => !Number.isFinite(v))) { setMsg($('cfg-msg'), 'Fill in every field with a number.', true); return; }
    if (!connected) { runDemo(true); cfgDirty = false; setMsg($('cfg-msg'), 'Demo re-run with these settings.'); return; }
    const q = CFG_KEYS.map((k) => `${k}=${encodeURIComponent(c[k])}`).join('&');
    const j = await robotCommand('/config?' + q, $('cfg-msg'), 'Saved on the robot.');
    if (j && j.cfg) { cfgDirty = false; writeCfg(j.cfg); }
  });

  // -------------------------------------------------------------- CSV in/out
  // Robot/dashboard format: seq,t_ms,x,y,theta,dn1..dn5,state,row
  function parseRow(f, kind) {
    if (!f || f.length < 12 || f[0] === '' || f[0][0] === '#') return null;
    const v = f.map(Number);
    if (kind === 'robot') {
      if (v.slice(0, 12).some((x) => !Number.isFinite(x))) return null;
      return { seq: v[0], t: v[1], x: v[2], y: v[3], th: v[4], dn: v.slice(5, 10), state: v[10], row: v[11] };
    }
    return null;
  }
  function parseCSV(text) {
    const lines = text.split(/\r?\n/).map((l) => l.trim()).filter((l) => l && !l.startsWith('#'));
    if (!lines.length) throw new Error('The file is empty.');
    let idx = null;
    const first = lines[0].split(',').map((s) => s.trim().toLowerCase());
    if (first.some((s) => /[a-z]/.test(s))) {
      const find = (...names) => first.findIndex((h) => names.includes(h));
      idx = {
        t: find('t_ms', 'timestamp_ms', 'time_ms', 't', 'ms'),
        x: find('x_mm', 'x'), y: find('y_mm', 'y'), th: find('theta_rad', 'theta', 'th'),
        dn: [1, 2, 3, 4, 5].map((k) => find('dn' + k)),
        state: find('state'), row: find('row'),
      };
      if ([idx.t, idx.x, idx.y, idx.th, ...idx.dn].some((i) => i < 0)) throw new Error('Header must include t_ms, x, y, theta and dn1 to dn5.');
      lines.shift();
    } else {
      const nc = first.length;
      if (nc === 12) idx = { t: 1, x: 2, y: 3, th: 4, dn: [5, 6, 7, 8, 9], state: 10, row: 11 };
      else if (nc === 14) idx = { t: 0, x: 1, y: 2, th: 3, dn: [8, 9, 10, 11, 12], state: -1, row: -1 };
      else throw new Error(`Expected 12 or 14 columns, found ${nc}.`);
    }
    const out = [];
    for (const l of lines) {
      const f = l.split(',').map(Number);
      const s = {
        seq: out.length, t: f[idx.t], x: f[idx.x], y: f[idx.y], th: f[idx.th],
        dn: idx.dn.map((i) => f[i]),
        state: idx.state >= 0 ? f[idx.state] : -1,
        row: idx.row >= 0 ? f[idx.row] : 0,
      };
      if ([s.t, s.x, s.y, s.th, ...s.dn].every(Number.isFinite)) out.push(s);
    }
    if (!out.length) throw new Error('No readable rows in the file.');
    return out;
  }
  function toCSV(S) {
    let s = 'seq,t_ms,x_mm,y_mm,theta_rad,dn1,dn2,dn3,dn4,dn5,state,row\n';
    for (const r of S) s += `${r.seq},${r.t},${r.x.toFixed(2)},${r.y.toFixed(2)},${r.th.toFixed(5)},${r.dn.join(',')},${r.state},${r.row}\n`;
    return s;
  }
  function download(name, blob) {
    const a = document.createElement('a');
    a.href = URL.createObjectURL(blob); a.download = name;
    document.body.appendChild(a); a.click(); a.remove();
    setTimeout(() => URL.revokeObjectURL(a.href), 2000);
  }
  function stamp() {
    const d = new Date(), p = (n) => String(n).padStart(2, '0');
    return `${d.getFullYear()}${p(d.getMonth() + 1)}${p(d.getDate())}-${p(d.getHours())}${p(d.getMinutes())}`;
  }
  $('file-in').addEventListener('change', async (e) => {
    const f = e.target.files[0];
    if (!f) return;
    try {
      const S = parseCSV(await f.text());
      sets.file.samples = S; sets.file.name = f.name;
      $('view-file').hidden = false;
      stopReplay();
      setView('file');
      setMsg($('run-msg'), `Loaded ${S.length} samples from ${f.name}.`);
    } catch (err) {
      setMsg($('run-msg'), 'Could not read that file: ' + err.message, true);
    }
    e.target.value = '';
  });
  $('btn-csv').addEventListener('click', () => {
    const S = sets[view].samples;
    if (!S.length) { setMsg($('run-msg'), 'Nothing to download yet.', true); return; }
    const tag = view === 'live' ? 'run' + sets.live.run : view;
    download(`raster-${tag}-${stamp()}.csv`, new Blob([toCSV(S)], { type: 'text/csv' }));
  });
  $('btn-png').addEventListener('click', () => {
    if (!rec) { setMsg($('run-msg'), 'No image to save yet.', true); return; }
    const k = Math.max(1, Math.round(4 / rec.cell * 2));
    const c = document.createElement('canvas');
    c.width = rec.W * k; c.height = rec.H * k;
    const g = c.getContext('2d');
    g.fillStyle = '#c8ced8'; g.fillRect(0, 0, c.width, c.height);
    g.imageSmoothingEnabled = imageOpts().smooth;
    g.drawImage(off, 0, 0, c.width, c.height);
    c.toBlob((b) => b && download(`raster-${view}-${stamp()}.png`, b), 'image/png');
  });

  // ------------------------------------------------------------- demo robot
  // A small simulation of the same raster state machine, so the page shows
  // something useful without a robot. Truth differs from the model slightly.
  function makeTarget(cfg, o) {
    // The covered area: middle-sensor tracks from the first to the last row.
    const side = cfg.side < 0 ? -1 : 1;
    const lead = Number.isFinite(cfg.lead) ? cfg.lead : 40;
    const xa = lead - 10, xb = cfg.row_len + lead + 10;
    const yEdge = (cfg.rows - 1) * cfg.pitch * side;
    const reach = Math.max(...o.sensors.map(([, l]) => Math.abs(l)));
    const ya = Math.min(0, yEdge) - reach - 10, yb = Math.max(0, yEdge) + reach + 10;
    const w = Math.ceil(xb - xa), h = Math.ceil(yb - ya);
    const c = document.createElement('canvas');
    c.width = w; c.height = h;
    const g = c.getContext('2d');
    g.fillStyle = '#000';
    // A 2 x 2 sheet of shapes centred in the middle-sensor band:
    // square and circle on one row, triangle and hexagon on the other.
    const bandTop = h - (Math.max(0, yEdge) + 9 - ya), bandBot = h - (Math.min(0, yEdge) - 9 - ya);
    const cx = w / 2, cy = (bandTop + bandBot) / 2;
    const cellW = Math.min(w * 0.42, 60), cellH = Math.min((bandBot - bandTop) * 0.46, 60);
    const s = Math.max(10, Math.min(cellW, cellH) * 0.78);
    const at = (dx, dy) => [cx + dx * cellW * 0.55, cy + dy * cellH * 0.55];
    let [x, y] = at(-1, -1); g.fillRect(x - s / 2, y - s / 2, s, s);                         // square
    [x, y] = at(1, -1); g.beginPath(); g.arc(x, y, s * 0.45, 0, Math.PI * 2); g.fill();      // circle
    [x, y] = at(-1, 1); g.beginPath(); g.moveTo(x - s / 2, y + s * 0.43); g.lineTo(x + s / 2, y + s * 0.43);
    g.lineTo(x, y - s * 0.43); g.closePath(); g.fill();                                       // triangle
    [x, y] = at(1, 1); g.beginPath();
    for (let k = 0; k < 6; k++) { const a = Math.PI / 6 + k * Math.PI / 3; g.lineTo(x + s * 0.5 * Math.cos(a), y + s * 0.5 * Math.sin(a)); }
    g.closePath(); g.fill();                                                                  // hexagon
    const px = g.getImageData(0, 0, w, h).data;
    // canvas row 0 is the top of the picture = largest y
    return (x, y) => {
      const i = Math.floor(x - xa), j = Math.floor(yb - y);
      if (i < 0 || j < 0 || i >= w || j >= h) return 0;
      return px[(j * w + i) * 4 + 3] / 255;
    };
  }

  function simulate(cfg, o) {
    const target = makeTarget(cfg, o);
    const offs = sensorOffsets(o);
    const side = cfg.side < 0 ? -1 : 1;
    const rTrueL = 16.0, rTrueR = 16.06, sepTrue = cfg.sep * 1.005;  // the real robot is a bit different
    const rModelL = cfg.r_left, rModelR = cfg.r_right, sepModel = cfg.sep;
    const sensorOff = [640, 700, 560, 660, 600], sensorGain = [1480, 1300, 1650, 1240, 1420];
    const DEAD = 10, K = 0.18, LAG = 0.08, dt = 0.01;   // ~58 mm/s at PWM 30, like the real robot
    const wheel = (p) => Math.sign(p) * Math.max(0, Math.abs(p) - DEAD) * K;  // rad/s
    let seed = 7;
    const rnd = () => { seed = (seed * 16807) % 2147483647; return seed / 2147483647; };
    const gauss = () => { let a = 0; for (let i = 0; i < 6; i++) a += rnd(); return (a - 3) * 0.7; };
    const wrap = (a) => Math.atan2(Math.sin(a), Math.cos(a));

    let tx = 0, ty = 0, tth = 0, ex = 0, ey = 0, eth = 0, wl = 0, wr = 0;
    let mode = 1, next = 2, modeT = 0, settle = 1.0, row = 0, heading = 0, sx = 0, sy = 0;
    let dir = 1, rowY = 0;
    const SENSOR_LAG_STEPS = 3;                 // readings describe where the robot was ~30 ms ago
    // Each sensor sees a blurry spot (Gaussian, sigma ~3 mm), like the real reflectance sensors.
    const SPOT = [];
    { let t = 0;
      for (let a = -6; a <= 6; a += 2) for (let b = -6; b <= 6; b += 2) { const w = Math.exp(-(a * a + b * b) / 18); SPOT.push([a, b, w]); t += w; }
      SPOT.forEach((p) => (p[2] /= t)); }
    const hist = [];
    const L = cfg.row_len, M = cfg.margin;
    // Same row-line steering as the firmware (Controller_c::driveOnRowLine).
    const lineDrive = (lineH, d) => {
      const ramp = Math.min(1, modeT / 0.3);
      const base = cfg.scan_pwm * ramp * d;
      const c = Math.cos(lineH) >= 0 ? 1 : -1;
      const steer = clamp(Math.atan((rowY - ey) / 35), -0.45, 0.45);
      const maxc = 5 + 10 * ramp;
      const turn = clamp(60 * wrap(lineH + d * c * steer - eth), -maxc, maxc);
      return [base - turn, base + turn];
    };
    const out = [];
    const enter = (m) => { mode = m; modeT = 0; sx = ex; sy = ey; };
    const settleThen = (dur, m) => { next = m; settle = dur; enter(1); };
    const rowHeading = (r) => (r % 2 === 0 ? 0 : Math.PI);
    for (let step = 0, t = 0; step < 200000; step++, t += dt) {
      let pl = 0, pr = 0;
      const dist = Math.hypot(ex - sx, ey - sy);
      if (mode === 1) { if (modeT >= settle) enter(next); }
      else if (cfg.pattern === 0 && (mode === 2 || mode === 9)) {
        // Back and forth: no turns, alternate forwards / reverse.
        const last = row + 1 >= cfg.rows;
        const done = dir > 0 ? ex >= (last ? L : L + M) : ex <= (last ? 0 : -M);
        if (done) {
          if (last) mode = 6;
          else { row++; dir = -dir; rowY = side * row * cfg.pitch; settleThen(0.3, 9); }
        } else {
          mode = ex >= 0 && ex <= L ? 2 : 9;
          [pl, pr] = lineDrive(0, dir);
        }
      }
      else if (mode === 2 || mode === 8) {
        if (mode === 2 && dist >= cfg.row_len) mode = row + 1 >= cfg.rows ? 6 : 8;
        if (mode === 8 && dist >= cfg.row_len + 2 * cfg.lead) { heading = side * Math.PI / 2; settleThen(0.3, 3); }
        else if (mode !== 6) [pl, pr] = lineDrive(rowHeading(row), 1);
      } else if (mode === 3 || mode === 5) {
        const err = wrap(heading - eth);
        if (Math.abs(err) < 0.03 || modeT > 6) {
          if (mode === 3) settleThen(0.3, 4); else { row++; rowY = side * row * cfg.pitch; settleThen(0.3, 2); }
        } else {
          const mag = clamp(40 * Math.abs(err), cfg.turn_min, cfg.turn_pwm);
          const d = err > 0 ? 1 : -1; pl = -d * mag; pr = d * mag;
        }
      } else if (mode === 4) {
        const nextY = side * (row + 1) * cfg.pitch;
        if (side > 0 ? ey >= nextY : ey <= nextY) { heading = rowHeading(row + 1); settleThen(0.3, 5); }
        else {
          const f = cfg.scan_pwm * Math.min(1, modeT / 0.3);
          const turn = clamp(60 * wrap(heading - eth), -15, 15);
          pl = f - turn; pr = f + turn;
        }
      }
      if (mode === 6) break;

      // Motor lag, then true motion and the odometry estimate.
      wl += (wheel(pl) - wl) * (dt / LAG);
      wr += (wheel(pr) - wr) * (dt / LAG);
      const vL = wl * rTrueL, vR = wr * rTrueR;
      const v = (vL + vR) / 2, w = (vR - vL) / sepTrue;
      tx += v * Math.cos(tth) * dt; ty += v * Math.sin(tth) * dt; tth += w * dt;
      const eL = wl * rModelL * dt, eR = wr * rModelR * dt;
      const ed = (eL + eR) / 2;
      ex += ed * Math.cos(eth); ey += ed * Math.sin(eth); eth += (eR - eL) / sepModel;
      modeT += dt;
      hist.push([tx, ty, tth]);
      if (hist.length > SENSOR_LAG_STEPS + 1) hist.shift();

      // Same logging as the robot: none while paused, half rate while turning.
      if (mode !== 1 && step % 2 === 0 && ((mode !== 3 && mode !== 5 && mode !== 9) || step % 4 === 0)) {
        const [hx, hy, hth] = hist[0];
        const c = Math.cos(hth), s = Math.sin(hth);
        const dn = offs.map(([fx, ly], i) => {
          const X = hx + fx * c - ly * s, Y = hy + fx * s + ly * c;
          let d = 0;
          for (const [ax, ay, w] of SPOT) d += w * target(X + ax, Y + ay);
          return Math.round(sensorOff[i] + sensorGain[i] * d + 28 * gauss());
        });
        out.push({ seq: out.length, t: Math.round(t * 1000), x: ex, y: ey, th: eth, dn, state: mode, row });
      }
    }
    return out;
  }

  let replayTimer = null;
  function stopReplay() { if (replayTimer) { clearInterval(replayTimer); replayTimer = null; } }
  function runDemo(animate) {
    stopReplay();
    const d = sets.demo;
    const cfg = readCfg();
    d.full = simulate(cfg, imageOpts());
    d.run++;
    view = 'demo';
    if (!animate) { d.samples = d.full.slice(); render(true); return; }
    d.samples = [];
    const reduce = window.matchMedia('(prefers-reduced-motion: reduce)').matches;
    const perTick = reduce ? d.full.length : 8;   // about 4x real time
    replayTimer = setInterval(() => {
      d.samples = d.full.slice(0, Math.min(d.full.length, d.samples.length + perTick));
      if (d.samples.length >= d.full.length) stopReplay();
      render();
    }, 40);
  }
  $('btn-replay').addEventListener('click', () => runDemo(true));

  // Fit sensor delay and the DN1/DN5, DN2/DN4 positions to this scan by making
  // readings that land in the same place agree (DN3 is the fixed reference).
  $('btn-autoalign').addEventListener('click', () => {
    const S0 = sets[view].samples;
    const msg = $('align-msg');
    if (S0.filter((x) => x.state === SCAN_ROW).length < 200) { setMsg(msg, 'Scan something first.', true); return; }
    setMsg(msg, 'Calibrating…');
    setTimeout(() => {
      const o = imageOpts();
      // Long scans: calibrate on every k-th sample so this stays around a second.
      const S = S0.length > 5000 ? S0.filter((_, i) => i % Math.ceil(S0.length / 5000) === 0) : S0;
      const L = o.sensors.map((p) => p.slice());
      let delay = o.delay || 0;
      const base = { ...o, cell: 2, foot: 4, use: 'scan', useSensor: [true, true, true, true, true], deblur: 0 };
      const score = (lay, d) => { const r = reconstruct(S, { ...base, sensors: lay, delay: d }); return r ? r.disagreement : Infinity; };
      const before = score(L, delay);
      const search = (vals, apply) => {
        let best = null;
        for (const v of vals) { const lay = L.map((p) => p.slice()); apply(lay, v); const sc = score(lay, delay); if (!best || sc < best.sc) best = { v, sc }; }
        return best.v;
      };
      const range = (c, half, step) => { const a = []; for (let v = c - half; v <= c + half + 1e-9; v += step) a.push(Math.round(v * 100) / 100); return a; };
      const fitDelay = (c, half, step) => {
        let best = null;
        for (const d of range(c, half, step)) { const sc = score(L, d); if (!best || sc < best.sc) best = { d, sc }; }
        delay = best.d;
      };
      const setOuterL = (lay, v) => { lay[0][1] = v; lay[4][1] = -v; };
      const setOuterF = (lay, v) => { lay[0][0] = v; lay[4][0] = v; };
      const setInnerL = (lay, v) => { lay[1][1] = v; lay[3][1] = -v; };
      fitDelay(50, 90, 10);
      for (const [half, step] of [[5, 0.5], [1.5, 0.25]]) {
        let v = search(range((L[0][1] - L[4][1]) / 2, half, step), setOuterL); setOuterL(L, v);
        v = search(range((L[0][0] + L[4][0]) / 2, half + 2, step), setOuterF); setOuterF(L, v);
        v = search(range((L[1][1] - L[3][1]) / 2, Math.min(half, 3), step / 2), setInnerL); setInnerL(L, v);
        fitDelay(delay, 10, 5);
      }
      const after = score(L, delay);
      setLayout(L);
      $('opt-delay').value = String(delay);
      saveImageSettings(); updatePitchHint();
      const gain = before > 0 ? Math.round((1 - after / before) * 100) : 0;
      setMsg(msg, `Delay ${delay} ms · DN1/DN5 ${fmt(L[0][0], 1)} ahead, ±${fmt(L[0][1], 2)} · DN2/DN4 ±${fmt(L[1][1], 2)} mm. Rows disagree ${gain}% less. Saved in the table.`);
      render(true);
    }, 30);
  });

  // ------------------------------------------------------ template compare
  // Your printed picture next to the robot's scan. The template is placed in
  // the robot's frame (axle start = origin, rows along +x, next rows +y):
  // its bottom-left corner at (x0, y0) mm, printed width w mm, then stretched
  // by (sx, sy) when Auto-align fits the scale. Sheets made by make_sheet.py
  // carry their placement inside the PNG, so they line up without searching.
  const TPL_KEY = 'rasterbot-template-v1';
  const TPL_R = 2;                       // template samples per mm
  let tpl = null;
  let tplStats = null;

  function pngInfo(buf) {
    // Reads pHYs (pixel size) and tEXt 'rasterbot-template' (placement) chunks.
    const b = new Uint8Array(buf), dv = new DataView(buf);
    const out = {};
    if (b.length < 8 || b[0] !== 0x89 || b[1] !== 0x50) return out;
    let p = 8;
    while (p + 8 <= b.length) {
      const len = dv.getUint32(p), type = String.fromCharCode(b[p + 4], b[p + 5], b[p + 6], b[p + 7]);
      const d = p + 8;
      if (type === 'pHYs' && len >= 9 && b[d + 8] === 1) out.pxPerMm = dv.getUint32(d) / 1000;
      if (type === 'tEXt') {
        const txt = new TextDecoder('latin1').decode(b.subarray(d, d + len));
        const z = txt.indexOf('\0');
        if (z > 0 && txt.slice(0, z) === 'rasterbot-template') { try { out.place = JSON.parse(txt.slice(z + 1)); } catch (e) { /* ignore */ } }
      }
      if (type === 'IEND') break;
      p = d + len + 4;
    }
    return out;
  }
  function jpegInfo(buf) {
    const b = new Uint8Array(buf);
    // JFIF APP0: FF D8 FF E0 .. 'JFIF\0' ver(2) units(1) Xdensity(2)
    if (b[0] === 0xff && b[1] === 0xd8 && b[2] === 0xff && b[3] === 0xe0 && b[6] === 0x4a && b[7] === 0x46) {
      const units = b[13], xd = (b[14] << 8) | b[15];
      if (units === 1 && xd > 0) return { pxPerMm: xd / 25.4 };
      if (units === 2 && xd > 0) return { pxPerMm: xd / 10 };
    }
    return {};
  }

  function defaultTplPlace(wmm) {
    // make_sheet.py convention: picture's bottom-left 45 mm ahead of the start
    // axle, on the first row line.
    return { x0: 45, y0: 0, w: wmm };
  }

  async function loadTemplateFile(file) {
    const msg = $('tpl-msg');
    const name = file.name || 'template';
    if (/\.pdf$/i.test(name) || file.type === 'application/pdf') {
      setMsg(msg, 'PDFs can\'t be opened here (the robot\'s WiFi has no internet for a PDF reader). For a make_sheet.py sheet, load <name>_template.png from the same folder. For any other PDF: open it in Preview, File > Export, Format PNG, 150 pixels/inch, and load that PNG.', true);
      return;
    }
    const buf = await file.arrayBuffer();
    let info = {};
    if (/\.png$/i.test(name) || file.type === 'image/png') info = pngInfo(buf);
    else if (/\.jpe?g$/i.test(name) || file.type === 'image/jpeg') info = jpegInfo(buf);
    const url = await new Promise((res, rej) => {
      const fr = new FileReader();
      fr.onload = () => res(fr.result); fr.onerror = rej;
      fr.readAsDataURL(file);
    });
    const isSvg = /\.svg$/i.test(name) || file.type === 'image/svg+xml';
    await setTemplate({ name, url, info, isSvg });
  }

  function imageFromUrl(url) {
    return new Promise((res, rej) => { const im = new Image(); im.onload = () => res(im); im.onerror = rej; im.src = url; });
  }

  async function setTemplate(src, saved) {
    const msg = $('tpl-msg');
    let im;
    try { im = await imageFromUrl(src.url); } catch (e) { setMsg(msg, 'That file could not be opened as a picture.', true); return; }
    const nw = im.naturalWidth || im.width, nh = im.naturalHeight || im.height;
    if (!nw || !nh) { setMsg(msg, 'That picture has no size (an SVG needs a width and height).', true); return; }
    let place, how;
    if (saved) { place = saved; how = saved.how || 'Restored from this browser.'; }
    else if (src.info.place && src.info.place.w_mm) {
      const p = src.info.place;
      place = { x0: p.x0_mm, y0: p.y0_mm, w: p.w_mm, fromSheet: true, sizeKnown: true };
      how = `Sheet "${p.sheet || src.name}": placed exactly as printed. Press Auto-align to correct for where the robot really started.`;
    } else {
      let ppm = src.info.pxPerMm;
      if (src.isSvg) ppm = 96 / 25.4;
      const known = ppm && ppm > 1.9 && ppm < 200;
      const cfgLen = parseFloat($('cfg-row_len').value);
      const wmm = known ? nw / ppm : (Number.isFinite(cfgLen) ? Math.max(20, cfgLen - 8) : 150);
      place = defaultTplPlace(Math.round(wmm * 10) / 10);
      place.sizeKnown = !!known;
      how = (known ? `Size from the file: ${fmt(wmm, 1)} mm wide. ` : 'The file doesn\'t say its printed size: set the width you printed it at. ')
        + 'Press Auto-align to find where it sits under the scan.';
    }
    const wmm = place.w, hmm = wmm * nh / nw;
    // Darkness samples at TPL_R per mm (capped for very large pictures).
    const R = Math.min(TPL_R, 3000 / Math.max(wmm, hmm));
    const cw = Math.max(2, Math.round(wmm * R)), ch = Math.max(2, Math.round(hmm * R));
    const c = document.createElement('canvas');
    c.width = cw; c.height = ch;
    const g = c.getContext('2d');
    g.fillStyle = '#fff'; g.fillRect(0, 0, cw, ch);
    g.drawImage(im, 0, 0, cw, ch);
    const px = g.getImageData(0, 0, cw, ch).data;
    const dark = new Float32Array(cw * ch);
    for (let i = 0; i < cw * ch; i++) dark[i] = 1 - (0.299 * px[i * 4] + 0.587 * px[i * 4 + 1] + 0.114 * px[i * 4 + 2]) / 255;
    tpl = {
      name: src.name, url: src.url, info: src.info, isSvg: src.isSvg, nw, nh,
      w: wmm, h: hmm, x0: place.x0, y0: place.y0, sx: place.sx || 1, sy: place.sy || 1,
      fromSheet: !!place.fromSheet, sizeKnown: !!place.sizeKnown, R: cw / wmm, cw, ch, dark, blurred: null, blurSigma: -1,
    };
    $('tpl-name').textContent = src.name;
    ['tpl-w', 'tpl-x', 'tpl-y'].forEach((id) => ($(id).disabled = false));
    $('tpl-align').disabled = false; $('tpl-clear').disabled = false;
    writeTplFields();
    setMsg(msg, how);
    saveTemplate();
    render(true);
  }

  function writeTplFields() {
    $('tpl-w').value = String(Math.round(tpl.w * 10) / 10);
    $('tpl-x').value = String(Math.round(tpl.x0 * 10) / 10);
    $('tpl-y').value = String(Math.round(tpl.y0 * 10) / 10);
  }

  function saveTemplate() {
    try {
      if (!tpl) { localStorage.removeItem(TPL_KEY); return; }
      const v = { name: tpl.name, url: tpl.url, info: tpl.info, isSvg: tpl.isSvg,
        place: { x0: tpl.x0, y0: tpl.y0, w: tpl.w, sx: tpl.sx, sy: tpl.sy, fromSheet: tpl.fromSheet, sizeKnown: tpl.sizeKnown } };
      localStorage.setItem(TPL_KEY, JSON.stringify(v));
    } catch (e) { /* too big for storage, or storage unavailable: fine for this visit */ }
  }
  async function restoreTemplate() {
    try {
      const v = JSON.parse(localStorage.getItem(TPL_KEY) || 'null');
      if (v && v.url) await setTemplate(v, v.place);
    } catch (e) { /* ignore */ }
  }

  // Darkness of the template at a world point (NaN outside the picture).
  function tplArray(blurMm) {
    if (!(blurMm > 0)) return tpl.dark;
    if (tpl.blurSigma !== blurMm) {
      const k = gaussKernel(blurMm * tpl.R);
      tpl.blurred = convolve(tpl.dark, tpl.cw, tpl.ch, k);
      tpl.blurSigma = blurMm;
    }
    return tpl.blurred;
  }
  function tplSampler(arr, x0, y0, sx, sy) {
    const R = tpl.R, cw = tpl.cw, ch = tpl.ch, h = tpl.h;
    return (x, y) => {
      const u = Math.floor(((x - x0) / sx) * R), v = Math.floor((h - (y - y0) / sy) * R);
      return (u < 0 || v < 0 || u >= cw || v >= ch) ? NaN : arr[v * cw + u];
    };
  }

  // Compares the shown scan with the template, cell by cell.
  function compareTemplate(r, shown, sample) {
    const W = r.W, H = r.H, cell = r.cell;
    const diff = new Float32Array(W * H).fill(NaN);
    let n = 0, sAbs = 0, sa = 0, sb = 0, saa = 0, sbb = 0, sab = 0;
    for (let j = 0; j < H; j++) {
      const y = r.minY + (j + 0.5) * cell;
      for (let i = 0; i < W; i++) {
        const v = shown[j * W + i];
        if (Number.isNaN(v)) continue;
        const t = sample(r.minX + (i + 0.5) * cell, y);
        if (Number.isNaN(t)) continue;
        diff[j * W + i] = v - t;
        n++; sAbs += Math.abs(v - t);
        sa += v; sb += t; saa += v * v; sbb += t * t; sab += v * t;
      }
    }
    const cov = sab / n - (sa / n) * (sb / n);
    const va = saa / n - (sa / n) ** 2, vb = sbb / n - (sb / n) ** 2;
    return { n, mae: n ? sAbs / n : NaN, corr: n && va > 1e-9 && vb > 1e-9 ? cov / Math.sqrt(va * vb) : NaN, area: n * cell * cell, diff };
  }

  const hexRgb = (h) => {
    const m = /^#?([0-9a-f]{2})([0-9a-f]{2})([0-9a-f]{2})/i.exec(h || '');
    return m ? [parseInt(m[1], 16), parseInt(m[2], 16), parseInt(m[3], 16)] : [128, 128, 128];
  };

  function drawTemplate() {
    const show = recCache.show, r = rec;
    const canv = ['tpl-c-ref', 'tpl-c-scan', 'tpl-c-diff'].map($);
    const dpr = window.devicePixelRatio || 1;
    const cssW = Math.max(120, canv[0].parentElement.clientWidth);
    const aspect = r ? (r.H / r.W) : 0.7;
    const cssH = Math.max(80, Math.min(420, cssW * aspect));
    for (const c of canv) {
      c.style.height = cssH + 'px';
      c.width = Math.round(cssW * dpr); c.height = Math.round(cssH * dpr);
      const g = c.getContext('2d');
      g.setTransform(1, 0, 0, 1, 0, 0);
      g.fillStyle = hatchPattern(g, dpr); g.fillRect(0, 0, c.width, c.height);
    }
    const ro = (id, t) => ($(id).textContent = t);
    if (!r || !show) { tplStats = null; ['tpl-err', 'tpl-corr', 'tpl-area'].forEach((id) => ro(id, '–')); }
    // Robot scan: the same bitmap as the main image (tone applied).
    if (r && show) {
      const g = canv[1].getContext('2d');
      g.imageSmoothingEnabled = imageOpts().smooth;
      g.drawImage(off, 0, 0, canv[1].width, canv[1].height);
    }
    if (!tpl) { ro('tpl-sc', '–'); return; }
    ro('tpl-sc', `${fmt(tpl.sx, 3)} / ${fmt(tpl.sy, 3)}`);
    if (!r) return;
    const o = imageOpts();
    const arr = tplArray($('tpl-blur').checked ? o.blur : 0);
    const sample = tplSampler(arr, tpl.x0, tpl.y0, tpl.sx, tpl.sy);
    // Template over the scanned area, one sample per canvas pixel.
    {
      const c = canv[0], g = c.getContext('2d');
      const im = g.createImageData(c.width, c.height);
      const wx = r.W * r.cell, wy = r.H * r.cell;
      for (let py = 0; py < c.height; py++) {
        const y = r.minY + wy * (1 - (py + 0.5) / c.height);
        for (let pxl = 0; pxl < c.width; pxl++) {
          const t = sample(r.minX + wx * (pxl + 0.5) / c.width, y);
          if (Number.isNaN(t)) continue;
          const q = (py * c.width + pxl) * 4, gv = Math.round(255 * (1 - t));
          im.data[q] = gv; im.data[q + 1] = gv; im.data[q + 2] = gv; im.data[q + 3] = 255;
        }
      }
      g.putImageData(im, 0, 0);
    }
    const st = compareTemplate(r, show, sample);
    tplStats = st;
    ro('tpl-err', st.n ? fmt(st.mae * 100, 1) + '%' : '–');
    ro('tpl-corr', Number.isFinite(st.corr) ? fmt(st.corr, 3) : '–');
    ro('tpl-area', st.n ? `${fmt(st.area / 100, 0)} cm²` : 'no overlap');
    // Difference: extra ink (scan darker) and missed ink (template darker).
    {
      const more = hexRgb(css('--diff-more')), less = hexRgb(css('--diff-less'));
      const t = document.createElement('canvas');
      t.width = r.W; t.height = r.H;
      const g = t.getContext('2d'), im = g.createImageData(r.W, r.H);
      for (let j = 0; j < r.H; j++) {
        const row = r.H - 1 - j;
        for (let i = 0; i < r.W; i++) {
          const d = st.diff[j * r.W + i];
          if (Number.isNaN(d)) continue;
          const a = Math.min(1, Math.abs(d) * 1.4), col = d > 0 ? more : less;
          const q = (row * r.W + i) * 4;
          for (let k = 0; k < 3; k++) im.data[q + k] = Math.round(255 + (col[k] - 255) * a);
          im.data[q + 3] = 255;
        }
      }
      g.putImageData(im, 0, 0);
      const cg = canv[2].getContext('2d');
      cg.imageSmoothingEnabled = o.smooth;
      cg.drawImage(t, 0, 0, canv[2].width, canv[2].height);
    }
  }

  // Finds where the template sits under the scan (and its scale) by
  // maximising the correlation between the two.
  function autoAlignTemplate() {
    const msg = $('tpl-msg');
    const r = rec, show = recCache.show;
    if (!tpl || !r || !show) { setMsg(msg, 'Scan something first (or load a CSV), then align.', true); return; }
    const o = imageOpts();
    const arr = tplArray(Math.max(o.blur, 1.5));
    // Scan points on a ~2 mm grid (averaged), and on the full grid for the final step.
    const pts = (step) => {
      const f = Math.max(1, Math.round(step / r.cell));
      const X = [], Y = [], V = [];
      for (let j = 0; j + f <= r.H; j += f) for (let i = 0; i + f <= r.W; i += f) {
        let s = 0, c = 0;
        for (let b = 0; b < f; b++) for (let a = 0; a < f; a++) { const v = show[(j + b) * r.W + i + a]; if (!Number.isNaN(v)) { s += v; c++; } }
        if (c * 2 >= f * f) { X.push(r.minX + (i + f / 2) * r.cell); Y.push(r.minY + (j + f / 2) * r.cell); V.push(s / c); }
      }
      return { X: Float32Array.from(X), Y: Float32Array.from(Y), V: Float32Array.from(V) };
    };
    const coarse = pts(2), fine = pts(r.cell);
    if (coarse.V.length < 20) { setMsg(msg, 'Not enough of the scan to align with.', true); return; }
    const score = (P, x0, y0, sx, sy) => {
      const s = tplSampler(arr, x0, y0, sx, sy);
      let n = 0, sa = 0, sb = 0, saa = 0, sbb = 0, sab = 0;
      for (let k = 0; k < P.V.length; k++) {
        const t = s(P.X[k], P.Y[k]);
        if (Number.isNaN(t)) continue;
        const v = P.V[k];
        n++; sa += v; sb += t; saa += v * v; sbb += t * t; sab += v * t;
      }
      if (n < P.V.length * 0.4) return -2;
      const va = saa / n - (sa / n) ** 2, vb = sbb / n - (sb / n) ** 2;
      return va > 1e-9 && vb > 1e-9 ? (sab / n - (sa / n) * (sb / n)) / Math.sqrt(va * vb) : -2;
    };
    const before = { x0: tpl.x0, y0: tpl.y0, sx: tpl.sx, sy: tpl.sy };
    let best = { ...before, sc: score(coarse, tpl.x0, tpl.y0, tpl.sx, tpl.sy) };
    const tryAt = (P, x0, y0, sx, sy) => { const v = score(P, x0, y0, sx, sy); if (v > best.sc) best = { x0, y0, sx, sy, sc: v }; };
    // 1) Position: near the expected place for a known sheet, anywhere overlapping otherwise.
    const ax0 = r.minX, ax1 = r.minX + r.W * r.cell, ay0 = r.minY, ay1 = r.minY + r.H * r.cell;
    const tw = tpl.w * tpl.sx, th = tpl.h * tpl.sy;
    let xs, ys;
    if (tpl.fromSheet) { xs = [tpl.x0 - 20, tpl.x0 + 20]; ys = [tpl.y0 - 20, tpl.y0 + 20]; }
    else { xs = [ax0 - tw + 0.3 * (ax1 - ax0), ax1 - 0.3 * (ax1 - ax0)]; ys = [ay0 - th + 0.3 * (ay1 - ay0), ay1 - 0.3 * (ay1 - ay0)]; }
    for (let x0 = xs[0]; x0 <= xs[1]; x0 += 2) for (let y0 = ys[0]; y0 <= ys[1]; y0 += 2) tryAt(coarse, x0, y0, best.sx, best.sy);
    // 2) Scale (about the middle of the scan, so content there stays put).
    if ($('tpl-scale').checked) {
      const cx = (ax0 + ax1) / 2, cy = (ay0 + ay1) / 2;
      for (let pass = 0; pass < 2; pass++) {
        const b = { ...best };
        const ux = (cx - b.x0) / b.sx, uy = (cy - b.y0) / b.sy;
        const lim = tpl.sizeKnown ? 0.06 : 0.1;
        for (let sx = 1 - lim; sx <= 1 + lim + 1e-4; sx += 0.01) for (let sy = 1 - lim; sy <= 1 + lim + 1e-4; sy += 0.01) {
          for (let dx = -2; dx <= 2; dx += 2) for (let dy = -2; dy <= 2; dy += 2) {
            tryAt(coarse, cx - ux * sx + dx, cy - uy * sy + dy, sx, sy);
          }
        }
      }
    }
    // 3) Fine position (and scale) on the full grid.
    best.sc = score(fine, best.x0, best.y0, best.sx, best.sy);
    const b0 = { ...best };
    for (let dx = -1.5; dx <= 1.5; dx += 0.25) for (let dy = -1.5; dy <= 1.5; dy += 0.25) tryAt(fine, b0.x0 + dx, b0.y0 + dy, b0.sx, b0.sy);
    if ($('tpl-scale').checked) {
      const b1 = { ...best };
      for (let ds = -0.006; ds <= 0.0061; ds += 0.002) for (let dt = -0.006; dt <= 0.0061; dt += 0.002) tryAt(fine, b1.x0, b1.y0, b1.sx + ds, b1.sy + dt);
    }
    if (best.sc <= -1) { setMsg(msg, 'Could not find the template under the scan. Check the printed width.', true); return; }
    Object.assign(tpl, { x0: best.x0, y0: best.y0, sx: best.sx, sy: best.sy });
    writeTplFields(); saveTemplate();
    let text = `Aligned (match ${fmt(best.sc, 3)}): moved ${fmt(best.x0 - before.x0, 1)} mm along the rows, ${fmt(best.y0 - before.y0, 1)} mm across.`;
    if ($('tpl-scale').checked) {
      text += ` Scale ${fmt(best.sx, 3)} along the rows, ${fmt(best.sy, 3)} across.`;
      const rl = parseFloat($('cfg-r_left').value), rr = parseFloat($('cfg-r_right').value);
      if (!tpl.sizeKnown) {
        text += ' The printed width was a guess, so the scale mostly corrects that: type the real width and align again.';
      } else if (Math.abs(best.sx - 1) >= 0.005 && Number.isFinite(rl) && Number.isFinite(rr)) {
        const rNow = (rl + rr) / 2;
        text += ` The scan is ${fmt(Math.abs(best.sx - 1) * 100, 1)}% ${best.sx > 1 ? 'longer' : 'shorter'} than the print along the rows, so the odometry ${best.sx > 1 ? 'over' : 'under'}-counts distance: wheel radius ≈ ${fmt(rNow / best.sx, 2)} mm (now ${fmt(rNow, 2)}) if the print is true to size.`;
      }
    }
    setMsg(msg, text);
    render(true);
  }

  $('tpl-in').addEventListener('change', async (e) => {
    const f = e.target.files && e.target.files[0];
    e.target.value = '';
    if (f) await loadTemplateFile(f);
  });
  $('tpl-align').addEventListener('click', () => {
    setMsg($('tpl-msg'), 'Aligning…');
    setTimeout(autoAlignTemplate, 30);
  });
  $('tpl-clear').addEventListener('click', () => {
    tpl = null; saveTemplate();
    $('tpl-name').textContent = 'No template';
    ['tpl-w', 'tpl-x', 'tpl-y'].forEach((id) => { $(id).value = ''; $(id).disabled = true; });
    $('tpl-align').disabled = true; $('tpl-clear').disabled = true;
    setMsg($('tpl-msg'), 'Template removed.');
    render(true);
  });
  for (const id of ['tpl-w', 'tpl-x', 'tpl-y']) {
    $(id).addEventListener('change', async () => {
      if (!tpl) return;
      const v = parseFloat($(id).value);
      if (!Number.isFinite(v)) { writeTplFields(); return; }
      if (id === 'tpl-x') tpl.x0 = v;
      if (id === 'tpl-y') tpl.y0 = v;
      if (id === 'tpl-w' && v >= 5) {
        // A new printed size: rebuild the samples at that size, keep the position.
        await setTemplate({ name: tpl.name, url: tpl.url, info: tpl.info, isSvg: tpl.isSvg },
          { x0: tpl.x0, y0: tpl.y0, w: v, sx: 1, sy: 1, fromSheet: tpl.fromSheet, sizeKnown: true,
            how: `Printed width set to ${fmt(v, 1)} mm. Press Auto-align to fit it under the scan again.` });
        return;
      }
      saveTemplate(); render(true);
    });
  }
  $('tpl-blur').addEventListener('change', () => render(true));

  // ----------------------------------------------------------------- wiring
  for (const b of document.querySelectorAll('#view-seg button')) {
    b.addEventListener('click', () => { if (!b.disabled) setView(b.dataset.view); });
  }
  for (const id of STORE_IDS) {
    const onChange = () => { saveImageSettings(); updatePitchHint(); render(); };
    $(id).addEventListener('input', onChange);
    $(id).addEventListener('change', onChange);
  }
  $('btn-geo-reset').addEventListener('click', () => { setLayout(LAYOUT_3PI); saveImageSettings(); updatePitchHint(); render(true); });
  window.addEventListener('resize', () => render());
  try {
    new MutationObserver(() => render(true)).observe(document.documentElement, { attributes: true, attributeFilter: ['data-theme'] });
    window.matchMedia('(prefers-color-scheme: dark)').addEventListener('change', () => render(true));
  } catch (e) { /* older browsers */ }

  buildStatic();
  loadImageSettings();
  updatePitchHint();
  syncPatternFields();
  updateEstimate();
  runDemo(false);          // open with a finished demo scan so the page is never empty
  restoreTemplate();
  setConnChip();
  if (base !== null) { pollStatus(); pollData(); }
})();
</script>
</body>
</html>
)DASHBOARD";
