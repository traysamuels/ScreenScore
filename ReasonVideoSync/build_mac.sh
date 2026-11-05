#!/bin/bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$ROOT/build"
DEST="$HOME/Library/Audio/Plug-Ins/VST3"

if [[ "$(uname -s)" != "Darwin" ]]; then
  echo "This plugin must be built on macOS (it uses AVFoundation)."
  exit 1
fi

command -v cmake >/dev/null 2>&1 || {
  echo "CMake is required. Install it with: brew install cmake"
  exit 1
}

cmake -S "$ROOT" -B "$BUILD_DIR"   -DCMAKE_BUILD_TYPE=Release   -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"   -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15

cmake --build "$BUILD_DIR" --config Release --target ReasonVideoSync_VST3 --parallel

PLUGIN="$(find "$BUILD_DIR" -type d -name 'ReasonVideoSync.vst3' -print -quit)"
if [[ -z "$PLUGIN" ]]; then
  echo "Could not locate the built ReasonVideoSync.vst3 bundle."
  exit 1
fi

mkdir -p "$DEST"
rm -rf "$DEST/ReasonVideoSync.vst3"
ditto "$PLUGIN" "$DEST/ReasonVideoSync.vst3"
echo "Installed to: $DEST/ReasonVideoSync.vst3"
