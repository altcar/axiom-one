#include "WebDashboard.h"
#include "dashboard_html.h"

WebDashboard_c::WebDashboard_c(Robot_c &robot_in, Controller_c &controller_in, uint16_t port)
  : web(port), robot(robot_in), controller(controller_in) {
}

void WebDashboard_c::begin() {
  web.on("/",       HTTP_GET, [this]() { handleRoot(); });
  web.on("/status", HTTP_ANY, [this]() { handleStatus(); });
  web.on("/data",   HTTP_ANY, [this]() { handleData(); });
  web.on("/start",  HTTP_ANY, [this]() { handleStart(); });
  web.on("/stop",   HTTP_ANY, [this]() { handleStop(); });
  web.on("/clear",  HTTP_ANY, [this]() { handleClear(); });
  web.on("/config", HTTP_ANY, [this]() { handleConfig(); });
  web.on("/favicon.ico", HTTP_GET, [this]() { web.send(204); });
  web.onNotFound([this]() { handleNotFound(); });
  web.begin();
}

void WebDashboard_c::update() {
  web.handleClient();
}

void WebDashboard_c::addCommonHeaders() {
  // Lets a dashboard opened from a file on your laptop talk to the robot too.
  web.sendHeader("Access-Control-Allow-Origin", "*");
  web.sendHeader("Cache-Control", "no-store");
}

void WebDashboard_c::sendJson(int code, const String &json) {
  addCommonHeaders();
  web.send(code, "application/json", json);
}

void WebDashboard_c::handleRoot() {
  addCommonHeaders();
  web.send_P(200, "text/html; charset=utf-8", DASHBOARD_HTML, sizeof(DASHBOARD_HTML) - 1);
}

String WebDashboard_c::configJson() const {
  const RasterConfig_t &c = controller.config;
  char buf[400];
  snprintf(buf, sizeof(buf),
    "{\"pattern\":%u,\"margin\":%.1f,\"rows\":%u,\"row_len\":%.1f,\"pitch\":%.1f,\"scan_pwm\":%.1f,"
    "\"turn_pwm\":%.1f,\"turn_min\":%.1f,\"side\":%d,\"lead\":%.1f,"
    "\"r_left\":%.3f,\"r_right\":%.3f,\"sep\":%.3f}",
    (unsigned)c.pattern, c.margin_mm, c.rows, c.row_length_mm, c.row_pitch_mm, c.scan_pwm,
    c.turn_pwm, c.turn_min_pwm, c.side, c.sensor_lead_mm,
    c.wheel_radius_left_mm, c.wheel_radius_right_mm, c.wheel_separation_mm);
  return String(buf);
}

void WebDashboard_c::handleStatus() {
  const OdometryModel_c::Pose &p = controller.odometry.pose;
  char buf[512];
  snprintf(buf, sizeof(buf),
    "{\"run\":%lu,\"mode\":%u,\"mode_name\":\"%s\",\"row\":%u,"
    "\"n\":%lu,\"cap\":%lu,\"oldest\":%lu,\"dropped\":%lu,\"ms\":%lu,"
    "\"x\":%.1f,\"y\":%.1f,\"th\":%.4f,"
    "\"dn\":[%u,%u,%u,%u,%u],\"msg\":\"%s\",\"cfg\":",
    (unsigned long)controller.getRunId(),
    (unsigned)controller.getMode(),
    controller.getModeName(),
    (unsigned)controller.getRow(),
    (unsigned long)controller.scan_log.count(),
    (unsigned long)controller.scan_log.capacity(),
    (unsigned long)controller.scan_log.oldest(),
    (unsigned long)controller.scan_log.dropped(),
    (unsigned long)millis(),
    p.x, p.y, p.theta,
    robot.surface.reading[0], robot.surface.reading[1], robot.surface.reading[2],
    robot.surface.reading[3], robot.surface.reading[4],
    controller.getMessage());
  String json(buf);
  json += configJson();
  json += "}";
  sendJson(200, json);
}

void WebDashboard_c::handleData() {
  uint32_t n = controller.scan_log.count();
  uint32_t oldest = controller.scan_log.oldest();
  uint32_t from = 0;
  if (web.hasArg("from")) {
    long requested = web.arg("from").toInt();
    from = requested > 0 ? (uint32_t)requested : 0;
  }
  if (from > n) from = n;
  if (from < oldest) from = oldest;   // older samples have been overwritten
  uint32_t to = from + MAX_ROWS_PER_REQUEST;
  if (to > n) to = n;

  String out;
  out.reserve(96 + (to - from) * 72);
  char line[128];

  // Header line: lets the page notice when a new run has started.
  snprintf(line, sizeof(line), "#run=%lu,n=%lu,from=%lu,oldest=%lu,dropped=%lu\n",
           (unsigned long)controller.getRunId(), (unsigned long)n, (unsigned long)from,
           (unsigned long)oldest, (unsigned long)controller.scan_log.dropped());
  out += line;

  // seq,t_ms,x,y,theta,dn1,dn2,dn3,dn4,dn5,state,row
  for (uint32_t i = from; i < to; i++) {
    const ScanSample_t &s = controller.scan_log.get(i);
    snprintf(line, sizeof(line), "%lu,%lu,%.1f,%.1f,%.4f,%u,%u,%u,%u,%u,%u,%u\n",
             (unsigned long)i, (unsigned long)s.t_ms, s.x, s.y, s.theta,
             s.dn[0], s.dn[1], s.dn[2], s.dn[3], s.dn[4], s.state, s.row);
    out += line;
  }

  controller.scan_log.markSent(to);
  addCommonHeaders();
  web.send(200, "text/plain", out);
}

void WebDashboard_c::handleStart() {
  controller.requestStart();
  sendJson(200, "{\"ok\":true}");
}

void WebDashboard_c::handleStop() {
  controller.requestStop();
  sendJson(200, "{\"ok\":true}");
}

void WebDashboard_c::handleClear() {
  bool ok = controller.clearLog();
  sendJson(ok ? 200 : 409, ok ? "{\"ok\":true}" : "{\"ok\":false,\"error\":\"Stop the scan first\"}");
}

void WebDashboard_c::handleConfig() {
  RasterConfig_t c = controller.config;
  bool any = false;

  if (web.hasArg("pattern"))  { c.pattern = (web.arg("pattern").toInt() == PATTERN_UTURN) ? PATTERN_UTURN : PATTERN_BACK_AND_FORTH; any = true; }
  if (web.hasArg("margin"))   { c.margin_mm = web.arg("margin").toFloat(); any = true; }
  if (web.hasArg("rows"))     { c.rows = (uint8_t)constrain(web.arg("rows").toInt(), 0L, 255L); any = true; }
  if (web.hasArg("row_len"))  { c.row_length_mm = web.arg("row_len").toFloat(); any = true; }
  if (web.hasArg("pitch"))    { c.row_pitch_mm = web.arg("pitch").toFloat(); any = true; }
  if (web.hasArg("scan_pwm")) { c.scan_pwm = web.arg("scan_pwm").toFloat(); any = true; }
  if (web.hasArg("turn_pwm")) { c.turn_pwm = web.arg("turn_pwm").toFloat(); any = true; }
  if (web.hasArg("turn_min")) { c.turn_min_pwm = web.arg("turn_min").toFloat(); any = true; }
  if (web.hasArg("side"))     { c.side = (web.arg("side").toInt() < 0) ? -1 : 1; any = true; }
  if (web.hasArg("lead"))     { c.sensor_lead_mm = web.arg("lead").toFloat(); any = true; }
  if (web.hasArg("r_left"))   { c.wheel_radius_left_mm = web.arg("r_left").toFloat(); any = true; }
  if (web.hasArg("r_right"))  { c.wheel_radius_right_mm = web.arg("r_right").toFloat(); any = true; }
  if (web.hasArg("sep"))      { c.wheel_separation_mm = web.arg("sep").toFloat(); any = true; }

  if (any && !controller.setConfig(c)) {
    String err = "{\"ok\":false,\"error\":\"";
    err += controller.isRunning() ? "Stop the scan before changing settings"
                                  : "A value is out of range";
    err += "\",\"cfg\":" + configJson() + "}";
    sendJson(409, err);
    return;
  }
  sendJson(200, "{\"ok\":true,\"cfg\":" + configJson() + "}");
}

void WebDashboard_c::handleNotFound() {
  addCommonHeaders();
  web.send(404, "text/plain", "Not found");
}
