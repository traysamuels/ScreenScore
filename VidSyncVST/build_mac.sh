#!/usr/bin/env bash
set -e

echo "=== Building VidSyncVST for macOS (Reason 13 Compatible) ==="

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${PROJECT_DIR}/build"

mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

echo "Step 1: Running CMake configure with Xcode generator..."
cmake .. -G Xcode     -DCMAKE_BUILD_TYPE=Release     -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"     -DCMAKE_OSX_DEPLOYMENT_TARGET="10.15"

echo "Step 2: Building VST3 and AU targets..."
cmake --build . --config Release --target VidSyncVST_VST3 VidSyncVST_AU

echo "Step 3: Installing VST3 to Reason 13 plugin directory..."
USER_VST3_DIR="${HOME}/Library/Audio/Plug-Ins/VST3"
mkdir -p "${USER_VST3_DIR}"

# Locate compiled .vst3 bundle
BUILT_VST3=$(find . -name "VidSyncVST.vst3" -type d | head -n 1)

if [ -n "${BUILT_VST3}" ]; then
    echo "Copying ${BUILT_VST3} to ${USER_VST3_DIR}..."
    cp -R "${BUILT_VST3}" "${USER_VST3_DIR}/"
    echo "✓ Success! VidSyncVST.vst3 installed to ${USER_VST3_DIR}"
    echo "Launch Reason 13, open the Device Browser, and look for 'VidSync' under VST3 Instruments / Effects!"
else
    echo "Error: Could not locate VidSyncVST.vst3 artifact."
    exit 1
fi
