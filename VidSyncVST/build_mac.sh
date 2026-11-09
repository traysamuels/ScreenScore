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

# Full Xcode provides the Xcode generator. Command Line Tools alone can still
# compile with Unix Makefiles, which is what many Intel machines have selected.
if xcodebuild -version >/dev/null 2>&1; then
    GENERATOR="Xcode"
else
    GENERATOR="Unix Makefiles"
    echo "Full Xcode is not selected. Using the command line compiler instead."
    echo "If Xcode.app is installed, point xcode-select at it for the Xcode generator:"
    echo "  sudo xcode-select -s /Applications/Xcode.app/Contents/Developer"
fi

configure_and_build() {
    local archs="$1"
    local force_clean="$2"

    if [[ "${force_clean}" == "1" ]]; then
        rm -rf "${BUILD_DIR}"
    elif [[ -f "${BUILD_DIR}/CMakeCache.txt" ]]; then
        local cache_gen cache_home cache_archs
        cache_gen="$(sed -n 's/^CMAKE_GENERATOR:INTERNAL=//p' "${BUILD_DIR}/CMakeCache.txt")"
        cache_home="$(sed -n 's/^CMAKE_HOME_DIRECTORY:INTERNAL=//p' "${BUILD_DIR}/CMakeCache.txt")"
        cache_archs="$(sed -n 's/^CMAKE_OSX_ARCHITECTURES:[^=]*=//p' "${BUILD_DIR}/CMakeCache.txt")"
        if [[ "${cache_gen}" != "${GENERATOR}" || "${cache_home}" != "${PROJECT_DIR}" || "${cache_archs}" != "${archs}" ]]; then
            echo "Removing incompatible CMake cache in ${BUILD_DIR}"
            rm -rf "${BUILD_DIR}"
        fi
    elif [[ -d "${BUILD_DIR}" ]]; then
        rm -rf "${BUILD_DIR}"
    fi

    cmake -S "${PROJECT_DIR}" -B "${BUILD_DIR}" -G "${GENERATOR}" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_OSX_ARCHITECTURES="${archs}" \
        -DCMAKE_OSX_DEPLOYMENT_TARGET="10.15" || return 1

    cmake --build "${BUILD_DIR}" --config Release --target VidSyncVST_VST3 --parallel || return 1
}

find_built_binary() {
    BUILT_VST3="$(find "${BUILD_DIR}" -type d -name "VidSyncVST.vst3" -print -quit)"
    BINARY=""
    if [[ -n "${BUILT_VST3}" ]]; then
        BINARY="$(find "${BUILT_VST3}/Contents/MacOS" -maxdepth 1 -type f -print -quit || true)"
    fi
}

has_intel_slice() {
    [[ -n "${BINARY}" ]] && lipo "${BINARY}" -verify_arch x86_64 >/dev/null 2>&1
}

if ! configure_and_build "arm64;x86_64" 0; then
    echo "Universal build failed. Building x86_64 only so Reason 13 can open the plug-in on this Intel Mac."
    configure_and_build "x86_64" 1 || exit 1
fi

find_built_binary
if ! has_intel_slice; then
    echo "This binary has no x86_64 slice. Reason 13 on an Intel Mac cannot load an arm64-only plug-in."
    echo "Rebuilding x86_64."
    configure_and_build "x86_64" 1 || exit 1
    find_built_binary
fi

if [[ -z "${BUILT_VST3}" || -z "${BINARY}" ]]; then
    echo "ERROR: VidSyncVST.vst3 was not produced."
    exit 1
fi

if ! has_intel_slice; then
    echo "ERROR: ${BINARY} has no x86_64 slice, so Reason 13 on an Intel Mac will not open it."
    exit 1
fi

echo "Built architectures: $(lipo -archs "${BINARY}")"

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
echo "Quit and relaunch Reason 13. On an Intel Mac, confirm the binary includes x86_64:"
echo "  lipo -info \"${installed_dir}/VidSyncVST.vst3/Contents/MacOS/VidSyncVST\""
