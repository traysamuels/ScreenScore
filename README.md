# ScreenScore (VidSyncVST)

VidSyncVST is a macOS VST3 that plays picture in sync with the Reason 13 transport. It builds and runs on macOS only. Reason 13 on an Intel Mac loads the `x86_64` slice of the plug-in.

## Build and install on an Intel Mac

Reason 13 requires macOS 10.15 (Catalina) or later.

1. Install **Xcode** or the Xcode Command Line Tools, and wait until the install finishes. `xcode-select -p` must print a developer directory. A `/usr/bin/cc` that only says "No developer tools were found" cannot configure this project. The build script uses the Xcode generator when full Xcode is selected, and Unix Makefiles when only the Command Line Tools are installed.
2. Install **CMake 3.22+** (`brew install cmake`).
3. From this repository:

```bash
cd VidSyncVST
chmod +x build_mac.sh
./build_mac.sh
```

The script builds `VidSyncVST.vst3` with an `x86_64` slice (and `arm64` as well, when the compiler can). It installs the bundle to:

`/Library/Audio/Plug-Ins/VST3/`

That is the VST3 folder Reason 13 scans. `~/Library/Audio/Plug-Ins/VST3` is not on the default scan list. Installing into the system folder asks for an administrator password when the folder is not writable.

If the Mac cannot compile `arm64`, the script rebuilds `x86_64` only. That slice is the one Intel Reason opens. The script will not install a binary that lacks `x86_64`.

## Load it in Reason 13

1. If you copied the plug-in from a downloaded DMG or zip, clear quarantine:

   ```bash
   xattr -dr com.apple.quarantine /Library/Audio/Plug-Ins/VST3/VidSyncVST.vst3
   ```

2. In Reason, open **Settings > Folders** and keep `/Library/Audio/Plug-Ins/VST3` enabled.
3. Open the browser menu and tick **Show VST3 plugins**.
4. Quit Reason and launch it again. The VST3 scan runs at startup.
5. Go to **Effects > VST3 Plugins > VidSync > VidSyncVST**, drag it into the rack, and click **Open Plugin Window**.

VidSyncVST is a VST3 bundle. Reason 13 loads it from the VST3 folder above.

Feature list and the file map are in [VidSyncVST/README.md](VidSyncVST/README.md).

## Verify

- The script prints `Installed: /Library/Audio/Plug-Ins/VST3/VidSyncVST.vst3`.
- `Built architectures` includes `x86_64`. On a machine that can cross-compile, it also includes `arm64`.
- `lipo -info /Library/Audio/Plug-Ins/VST3/VidSyncVST.vst3/Contents/MacOS/VidSyncVST` lists `x86_64`.
- After relaunching Reason on the Intel Mac, the plug-in window opens from the Effects palette. Load an `.mp4` or `.mov` and press play. The picture should follow the transport.
