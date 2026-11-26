#!/usr/bin/env python3
"""Merge Processing .pde tabs into one .java file, adding a headless batch driver.

Processing's PDE preprocessor does little more than hoist imports and wrap the
tabs in a class extending PApplet; that is reproduced here so the twin can be
compiled and run without the Processing IDE.
"""
import re, sys, pathlib
def preprocess(code):
    """Mimic Processing's PDE preprocessor where javac needs it: float literals."""
    out = []
    i, n, state = 0, len(code), None
    while i < n:
        c = code[i]
        if state is None:
            if code.startswith('//', i): state = 'line'; out.append('//'); i += 2; continue
            if code.startswith('/*', i): state = 'block'; out.append('/*'); i += 2; continue
            if c == '"': state = 'str'; out.append(c); i += 1; continue
            if c == "'": state = 'char'; out.append(c); i += 1; continue
            m = re.match(r'(?<![\w.])(\d+\.\d*|\.\d+|\d+[eE][-+]?\d+)([eE][-+]?\d+)?(?![\w.])', code[i:])
            if m:
                tok = m.group(0); out.append(tok + 'f'); i += len(tok); continue
            out.append(c); i += 1
        elif state == 'line':
            out.append(c)
            if c == '\n': state = None
            i += 1
        elif state == 'block':
            if code.startswith('*/', i): out.append('*/'); i += 2; state = None; continue
            out.append(c); i += 1
        else:
            out.append(c)
            if c == '\\':
                if i + 1 < n: out.append(code[i + 1])
                i += 2; continue
            if (state == 'str' and c == '"') or (state == 'char' and c == "'"): state = None
            i += 1
    return ''.join(out)


src = pathlib.Path(sys.argv[1])
out = pathlib.Path(sys.argv[2])

order = ["DigitalTwin.pde"] + sorted(
    p.name for p in src.glob("*.pde") if p.name != "DigitalTwin.pde"
)

imports, body = [], []
for name in order:
    text = (src / name).read_text()
    lines = []
    for line in text.splitlines():
        if re.match(r"^\s*import\s", line):
            imports.append(line.strip())
        else:
            lines.append(line)
    body.append(f"\n// ===== {name} =====\n" + "\n".join(lines))

# Headless batch driver: only active when -Dbatch.tag is set. GUI path untouched.
batch = r'''
// ===== Batch.pde (added for offline analysis) =====
boolean batchMode = false;
boolean batchStarted = false;
boolean batchWasPending = false;
int batchTargetRuns = 1;
int batchDone = 0;
String batchTag = "";
String batchModel = "calibrated";

void batchInit() {
  String tag = System.getProperty("batch.tag");
  if (tag == null) return;
  batchMode = true;
  batchTag = tag;
  batchModel = System.getProperty("batch.model", "calibrated");
  batchTargetRuns = Integer.parseInt(System.getProperty("batch.runs", "1"));
  println("BATCH start tag=" + batchTag + " model=" + batchModel + " runs=" + batchTargetRuns);
  try {
    java.lang.reflect.Field f = PApplet.class.getDeclaredField("sketchPath");
    f.setAccessible(true);
    f.set(this, new File(System.getProperty("batch.out", System.getProperty("user.dir"))).getAbsolutePath());
  } catch (Exception e) { println("BATCH sketchPath: " + e); }
  println("BATCH dir=" + sketchPath(""));
}

void applyBatchOverrides() {
  ModelParameters_c m = (batchModel.equals("nominal")) ? simulator.nominal : simulator.calibrated;
  simulator.activeModel = m;
  RasterConfig_c c = simulator.controller.config;
  c.pattern = PATTERN_BACK_AND_FORTH;
  String s;
  if ((s = System.getProperty("batch.pattern")) != null) c.pattern = (byte) Integer.parseInt(s);
  if ((s = System.getProperty("batch.pitch"))   != null) c.row_pitch_mm = Float.parseFloat(s);
  if ((s = System.getProperty("batch.rows"))    != null) c.rows = Integer.parseInt(s);
  if ((s = System.getProperty("batch.rowlen"))  != null) c.row_length_mm = Float.parseFloat(s);
  if ((s = System.getProperty("batch.pwm"))     != null) c.scan_pwm = Float.parseFloat(s);
  if ((s = System.getProperty("batch.sigma"))   != null) m.sensorFootprintSigmaMm = Float.parseFloat(s);
  if ((s = System.getProperty("batch.noise"))   != null) m.sensorNoiseSd = Float.parseFloat(s);
  if ((s = System.getProperty("batch.delay"))   != null) m.sensorDelayMs = Integer.parseInt(s);
  if ((s = System.getProperty("batch.sdxy"))    != null) m.startPositionSdMm = Float.parseFloat(s);
  if ((s = System.getProperty("batch.sdth"))    != null) m.startHeadingSdDeg = Float.parseFloat(s);
  if ((s = System.getProperty("batch.trueSep")) != null) m.wheelSeparationMm = Float.parseFloat(s);
  if ((s = System.getProperty("batch.beliefSep")) != null) c.wheel_separation_mm = Float.parseFloat(s);
  if ((s = System.getProperty("batch.radius"))  != null) {
    m.wheelRadiusLeftMm = Float.parseFloat(s); m.wheelRadiusRightMm = Float.parseFloat(s);
  }
  if ((s = System.getProperty("batch.efficiency")) != null) m.rotationEfficiency = Float.parseFloat(s);
}

void batchTick() {
  if (!batchStarted) {
    batchStarted = true;
    simulator.fastForward = true;
    applyBatchOverrides();
    simulator.beginSimRun();
    batchWasPending = true;
    return;
  }
  if (batchWasPending && !simulator.simRunPending) {
    batchDone++;
    if (batchDone >= batchTargetRuns) {
      println("BATCH DONE " + batchTag + " runs=" + batchDone);
      exit();
      return;
    }
    applyBatchOverrides();
    simulator.beginSimRun();
  }
  batchWasPending = simulator.simRunPending;
}
'''

defaults = ["import java.util.*;", "import java.io.*;", "import java.math.*;",
            "import processing.core.*;", "import processing.data.*;", "import processing.opengl.*;",
            "import processing.event.*;"]
header = "\n".join(defaults + sorted(set(imports)))
merged = batch + "\n" + "\n".join(body)
merged = preprocess(merged)

for _m in ("settings()", "setup()", "draw()", "keyPressed()", "mousePressed()",
           "mouseDragged()", "mouseReleased()"):
    merged = merged.replace("\nvoid " + _m, "\npublic void " + _m)

merged = re.sub(r"(?<![\w(])color\s+(?=[A-Za-z_]\w*\s*[,;)])", "int ", merged)

# Hoist setup()/draw() patches for the driver.
merged = merged.replace(
    "  simulator = new Simulator(this, SURFACE_OFFSET_X_MM, SURFACE_OFFSET_Y_MM);\n}",
    "  simulator = new Simulator(this, SURFACE_OFFSET_X_MM, SURFACE_OFFSET_Y_MM);\n  batchInit();\n}")
merged = merged.replace(
    "  simulator.update();\n  simulator.draw();",
    "  simulator.update();\n  if (batchMode) { batchTick(); return; }\n  simulator.draw();")

java = f"""// Generated from the ImageRaster DigitalTwin tabs. Do not edit.
{header}
public class DigitalTwin extends PApplet {{{merged}

  public static void main(String[] args) {{ PApplet.main(DigitalTwin.class, args); }}
}}
"""
out.write_text(java)
print("wrote", out, len(java.splitlines()), "lines")
