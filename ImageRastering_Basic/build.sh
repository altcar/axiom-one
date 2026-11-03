#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
board='esp32:esp32:m5stack_stamp_c3:CDCOnBoot=default'
sketch='firmware/ImageRastering'
arduino-cli compile --fqbn "$board" --build-path "$PWD/build" "$sketch"
if [[ $# -eq 1 ]]; then
  arduino-cli upload --fqbn "$board,UploadSpeed=115200" --port "$1" --input-dir "$PWD/build" "$sketch"
elif [[ $# -gt 1 ]]; then
  echo 'Usage: bash build.sh [serial-port]' >&2
  exit 2
fi
