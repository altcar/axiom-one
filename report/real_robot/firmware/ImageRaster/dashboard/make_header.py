"""Builds ../dashboard_html.h from dashboard.html so the StampC3 can serve it.

Run this after editing dashboard.html, then re-upload the sketch:
    python3 dashboard/make_header.py
"""
from pathlib import Path

here = Path(__file__).resolve().parent
src = (here / "dashboard.html").read_text(encoding="utf-8")

# dashboard.html starts with <title>, <meta> and <style>; put those in <head>.
cut = src.index("</style>") + len("</style>")
head, body = src[:cut], src[cut:]
page = (
    "<!doctype html>\n<html lang=\"en\">\n<head>\n<meta charset=\"utf-8\">\n"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1, viewport-fit=cover\">\n"
    + head
    + "\n</head>\n<body>\n"
    + body.strip()
    + "\n</body>\n</html>\n"
)

DELIM = "DASHBOARD"
if f"){DELIM}\"" in page:
    raise SystemExit("dashboard.html contains the raw-string delimiter; change DELIM")

header = (
    "// GENERATED FILE - do not edit by hand.\n"
    "// Source: dashboard/dashboard.html. After editing it, run:\n"
    "//     python3 dashboard/make_header.py\n"
    "#pragma once\n"
    "#include <Arduino.h>\n\n"
    f"const char DASHBOARD_HTML[] PROGMEM = R\"{DELIM}({page}){DELIM}\";\n"
)
out = here.parent / "dashboard_html.h"
out.write_text(header, encoding="utf-8")
print(f"Wrote {out.name}: {len(page.encode('utf-8')):,} bytes of HTML")
