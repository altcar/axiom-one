#ifndef WEB_DASHBOARD_H
#define WEB_DASHBOARD_H

#include <Arduino.h>
#include <WebServer.h>
#include "Robot.h"
#include "Controller.h"

/**
 * @brief Serves the raster dashboard web page and its data from the StampC3.
 *
 * Connect a laptop/phone to the robot's WiFi (set in the .ino), then open
 *   http://192.168.4.1:8080
 *
 * Port 8080 is used so the template's TCP telemetry on port 80 (used by the
 * Processing Digital Twin) keeps working unchanged.
 *
 * Endpoints (all GET, all allow cross-origin requests):
 *   /          the dashboard page (dashboard_html.h)
 *   /status    JSON: mode, row, pose, live DN1..DN5, log size, settings
 *   /data      CSV rows of the scan log: /data?from=N returns samples N..N+249
 *              (the log is a ring buffer; the page downloads while the robot scans)
 *   /start     start a scan (same as pressing the StampC3 button when idle)
 *   /stop      stop the robot
 *   /clear     empty the scan log (only when not running)
 *   /config    read settings, or change them: /config?rows=6&row_len=180...
 */
class WebDashboard_c {
  public:
    WebDashboard_c(Robot_c &robot_in, Controller_c &controller_in, uint16_t port = 8080);

    /** @brief Registers the endpoints and starts listening. Call after WiFi starts. */
    void begin();

    /** @brief Answers any waiting browser request. Call every loop(). */
    void update();

  private:
    WebServer web;
    Robot_c &robot;
    Controller_c &controller;

    static constexpr uint32_t MAX_ROWS_PER_REQUEST = 250;

    void handleRoot();
    void handleStatus();
    void handleData();
    void handleStart();
    void handleStop();
    void handleClear();
    void handleConfig();
    void handleNotFound();

    void addCommonHeaders();
    void sendJson(int code, const String &json);
    String configJson() const;
};

#endif
