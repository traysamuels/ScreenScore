# ScreenScore (VidSyncVST)

VidSyncVST is a macOS VST3 that plays picture in sync with the Reason 13 transport. Reason 13 on an Intel Mac loads the `x86_64` slice. GitHub builds the universal plug-in (`x86_64` and `arm64`), so the Intel Mac does not need Xcode or CMake.

## Install on an Intel Mac

Reason 13 requires macOS 10.15 (Catalina) or later.

1. Download the universal plug-in from the GitHub Release **VidSyncVST macOS universal**:
   - [VidSyncVST_Mac.dmg](https://github.com/traysamuels/ScreenScore/releases/download/vidsync-vst3/VidSyncVST_Mac.dmg)
   - or [VidSyncVST.vst3.zip](https://github.com/traysamuels/ScreenScore/releases/download/vidsync-vst3/VidSyncVST.vst3.zip)

   The same files are attached to every [macOS build](https://github.com/traysamuels/ScreenScore/actions/workflows/build-dmg.yml) as the **VidSyncVST-macOS-Universal** artifact. On a pull request, open the **build** check and download that artifact.

2. Install it into the folder Reason scans, `/Library/Audio/Plug-Ins/VST3/`.
   - **DMG:** open `VidSyncVST_Mac.dmg` and drag `VidSyncVST.vst3` onto **Drag to VST3 Folder**. macOS asks for an administrator password.
   - **Zip:** unzip `VidSyncVST.vst3.zip`. Copy the `VidSyncVST.vst3` folder into `/Library/Audio/Plug-Ins/VST3/`. Create that folder first if it is missing:

     ```bash
     sudo mkdir -p /Library/Audio/Plug-Ins/VST3
     sudo ditto VidSyncVST.vst3 /Library/Audio/Plug-Ins/VST3/VidSyncVST.vst3
     ```

3. Clear the download quarantine flag, or Reason can refuse to open the plug-in:

   ```bash
   xattr -dr com.apple.quarantine /Library/Audio/Plug-Ins/VST3/VidSyncVST.vst3
   ```

4. Confirm the Intel slice is present:

   ```bash
   lipo -info /Library/Audio/Plug-Ins/VST3/VidSyncVST.vst3/Contents/MacOS/VidSyncVST
   ```

   The line must include `x86_64`. `arm64` is the Apple Silicon slice in the same universal binary.

## Load it in Reason 13

1. Open **Settings > Folders** and keep `/Library/Audio/Plug-Ins/VST3` enabled.
2. Open the browser menu and tick **Show VST3 plugins**.
3. Quit Reason completely and launch it again. Reason scans plug-ins at startup.
4. Go to **Effects > VST3 Plugins > VidSync > VidSyncVST**, drag it into the rack, and click **Open Plugin Window**.
5. Click **Open Video File...**, choose an `.mp4` or `.mov`, and press play. The picture follows the Reason transport.

`~/Library/Audio/Plug-Ins/VST3` is not on Reason's default scan list. Use the system folder above.

Feature list and the file map are in [VidSyncVST/README.md](VidSyncVST/README.md).

## Optional: build on a Mac that has Xcode

The GitHub build is the supported install path. To compile locally instead, install Xcode or the Command Line Tools and CMake 3.22+, then run `VidSyncVST/build_mac.sh`. That script installs the same universal VST3 into `/Library/Audio/Plug-Ins/VST3/`.
