#!/usr/bin/env bash
# Build VidSyncVST.vst3 and install it where Reason 13 scans for VST3 plug-ins.
set -euo pipefail

if [[ "$(uname -s)" != "Darwin" ]]; then
    echo "ERROR: VidSyncVST is a macOS audio plug-in and must be built on macOS."
    exit 1
fi

if ! xcode-select -p >/dev/null 2>&1; then
    echo "ERROR: Xcode or the Xcode Command Line Tools are not installed."
    echo "A /usr/bin/cc stub that only opens the installer cannot compile this project."
    echo "Install the tools, wait until they finish, then run this script again:"
    echo "  xcode-select --install"
    exit 1
fi

if ! command -v cmake >/dev/null 2>&1; then
    echo "ERROR: CMake 3.22 or newer is required. Install it with: brew install cmake"
    exit 1
fi

echo "=== Building VidSyncVST for Reason 13 (macOS VST3) ==="

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${PROJECT_DIR}/build"
# Reason 13 scans this system folder for VST3. It does not scan
# ~/Library/Audio/Plug-Ins/VST3 unless that folder is added manually.
SYSTEM_VST3_DIR="/Library/Audio/Plug-Ins/VST3"
USER_VST3_DIR="${HOME}/Library/Audio/Plug-Ins/VST3"

# A failed configure (or the Unix Makefiles cache that used to be checked in)
# makes `cmake -G Xcode` refuse to run. An incomplete build directory has no
# generated Xcode project, so start that directory over.
if [[ -d "${BUILD_DIR}" && ! -d "${BUILD_DIR}/VidSyncVST.xcodeproj" ]]; then
    echo "Removing incomplete CMake build directory: ${BUILD_DIR}"
    rm -rf "${BUILD_DIR}"
fi

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

BINARY="$(find "${BUILT_VST3}/Contents/MacOS" -maxdepth 1 -type f -print -quit || true)"
if [[ -n "${BINARY}" ]] && command -v lipo >/dev/null 2>&1; then
    echo "Built architectures: $(lipo -archs "${BINARY}")"
fi

install_bundle() {
    local dest_dir="$1"
    local use_sudo="$2"
    if [[ "${use_sudo}" == "1" ]]; then
        sudo mkdir -p "${dest_dir}"
        sudo rm -rf "${dest_dir}/VidSyncVST.vst3"
        sudo ditto "${BUILT_VST3}" "${dest_dir}/VidSyncVST.vst3"
    else
        mkdir -p "${dest_dir}"
        rm -rf "${dest_dir}/VidSyncVST.vst3"
        ditto "${BUILT_VST3}" "${dest_dir}/VidSyncVST.vst3"
    fi
}

installed_dir=""
if [[ -w "${SYSTEM_VST3_DIR}" ]] || { [[ ! -e "${SYSTEM_VST3_DIR}" ]] && [[ -w "/Library/Audio/Plug-Ins" ]]; }; then
    install_bundle "${SYSTEM_VST3_DIR}" 0
    installed_dir="${SYSTEM_VST3_DIR}"
elif sudo -n true >/dev/null 2>&1; then
    install_bundle "${SYSTEM_VST3_DIR}" 1
    installed_dir="${SYSTEM_VST3_DIR}"
else
    echo "Reason 13 scans ${SYSTEM_VST3_DIR}. Installing there needs an administrator password."
    if sudo -v; then
        install_bundle "${SYSTEM_VST3_DIR}" 1
        installed_dir="${SYSTEM_VST3_DIR}"
    fi
fi

if [[ -z "${installed_dir}" ]]; then
    install_bundle "${USER_VST3_DIR}" 0
    echo "ERROR: Could not install into ${SYSTEM_VST3_DIR}."
    echo "The plug-in was copied to ${USER_VST3_DIR}/VidSyncVST.vst3 instead."
    echo "Reason will not load it from that folder until you add the folder in"
    echo "Reason > Settings > Folders, or re-run this script and approve the password prompt."
    exit 1
fi

if [[ -d "${USER_VST3_DIR}/VidSyncVST.vst3" ]]; then
    rm -rf "${USER_VST3_DIR}/VidSyncVST.vst3"
    echo "Removed the old copy in ${USER_VST3_DIR}. Reason does not scan that folder."
fi

echo "Installed: ${installed_dir}/VidSyncVST.vst3"
echo "Quit and relaunch Reason 13 so it scans VST3 plug-ins."
