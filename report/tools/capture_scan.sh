#!/usr/bin/env bash
# Start a scan on the robot and pull the log to CSV, no browser needed.
#   ./tools/capture_scan.sh [output.csv]
# Env: ROBOT (default 192.168.4.1), TIMEOUT (default 240 s)
set -euo pipefail
ROBOT=${ROBOT:-192.168.4.1}
OUT=${1:-data/scan_$(date +%Y%m%d_%H%M%S).csv}
TIMEOUT=${TIMEOUT:-240}
mkdir -p "$(dirname "$OUT")"

echo "checking $ROBOT"
curl -fsS --max-time 5 "http://$ROBOT:8080/status" >/dev/null || {
  echo "cannot reach the robot. Join its WiFi first." >&2; exit 1; }

curl -fsS "http://$ROBOT:8080/clear" >/dev/null
curl -fsS "http://$ROBOT:8080/start" >/dev/null
echo "scanning, writing $OUT"

: > "$OUT"
last=0; idle=0; t0=$(date +%s)
while :; do
  body=$(curl -fsS --max-time 5 "http://$ROBOT:8080/data?from=$last" || true)
  if [ -n "$body" ]; then
    n=$(printf '%s\n' "$body" | head -1 | sed -n 's/.*n=\([0-9]*\).*/\1/p')
    printf '%s\n' "$body" | grep -v '^#' | grep -v '^seq,' >> "$OUT" || true
    if [ -n "${n:-}" ] && [ "$n" -gt "$last" ]; then last=$n; idle=0; else idle=$((idle+1)); fi
  else
    idle=$((idle+1))
  fi
  sig=$(curl -fsS --max-time 5 "http://$ROBOT:8080/status" | tr -d '\n' || true)
  if [ "$idle" -ge 6 ] && ! printf '%s' "$sig" | grep -q '"signal":1'; then break; fi
  if [ $(( $(date +%s) - t0 )) -gt "$TIMEOUT" ]; then echo "timeout" >&2; break; fi
  sleep 0.5
done
echo "wrote $(wc -l < "$OUT") rows to $OUT"
