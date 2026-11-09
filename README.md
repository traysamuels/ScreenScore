# ScreenScore (VidSyncVST)

VidSyncVST is a macOS VST3 that plays picture in sync with the Reason 13 transport. It is an Apple AVFoundation plug-in, so it builds and runs on macOS only.

## Build and install

1. Install **Xcode** or the Xcode Command Line Tools, and wait until the install finishes. `xcode-select -p` must print a developer directory. A `/usr/bin/cc` that only says "No developer tools were found" cannot configure this project.
2. Install **CMake 3.22+** (`brew install cmake`).
3. From this repository:

```bash
cd VidSyncVST
chmod +x build_mac.sh
./build_mac.sh
```

The script builds a universal (`arm64` + `x86_64`) `VidSyncVST.vst3` and installs it to:

`/Library/Audio/Plug-Ins/VST3/`

That system folder is the VST3 location Reason 13 scans. The user folder `~/Library/Audio/Plug-Ins/VST3` is not on Reason's default scan list. Installing into the system folder asks for an administrator password when the folder is not writable.

Then quit and relaunch Reason. The plug-in is under **Effects > VST3 Plugins > VidSync > VidSyncVST**.

Feature list, controls, and the file map are in [VidSyncVST/README.md](VidSyncVST/README.md).

## Verify

- The script prints `Installed: /Library/Audio/Plug-Ins/VST3/VidSyncVST.vst3` and `Built architectures: arm64 x86_64`.
- `lipo -info /Library/Audio/Plug-Ins/VST3/VidSyncVST.vst3/Contents/MacOS/VidSyncVST` lists `arm64` and `x86_64`.
- After relaunching Reason, open the plug-in, load an `.mp4` or `.mov`, and press play. The picture should follow the transport instead of sitting on a frozen frame.
