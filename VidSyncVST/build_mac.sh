#!/usr/bin/env bash
set -euo pipefail

echo "=== Building VidSyncVST for Reason 13 (macOS VST3) ==="

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${PROJECT_DIR}/build"
USER_VST3_DIR="${HOME}/Library/Audio/Plug-Ins/VST3"

cmake -S "${PROJECT_DIR}" -B "${BUILD_DIR}" -G Xcode \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET="10.15"

cmake --build "${BUILD_DIR}" --config Release --target VidSyncVST_VST3 --parallel

BUILT_VST3="$(find "${BUILD_DIR}" -type d -name "VidSyncVST.vst3" -print -quit)"
if [[ -z "${BUILT_VST3}" ]]; then
    echo "ERROR: VidSyncVST.vst3 was not produced."
    exit 1
fi

mkdir -p "${USER_VST3_DIR}"
rm -rf "${USER_VST3_DIR}/VidSyncVST.vst3"
ditto "${BUILT_VST3}" "${USER_VST3_DIR}/VidSyncVST.vst3"

echo "Installed: ${USER_VST3_DIR}/VidSyncVST.vst3"
echo "In Reason 13, rescan VST3 plug-ins if it is not immediately visible."
