# VidSyncVST — Hardware-Accelerated Video Sync Plugin for Reason 13 (macOS)

**VidSyncVST** is a lightweight, hardware-accelerated VST3 plugin designed specifically for film, TV, commercial, and game composers working in **Reason 13** on Apple macOS (Apple Silicon M1/M2/M3/M4 & Intel).

Because Reason 13 lacks a built-in video sequencer track, VidSyncVST acts as your video monitor window. It synchronizes seamlessly to Reason's transport playhead using native **AVFoundation** hardware decoding, giving you fluid 60 FPS playback, instant frame-accurate timeline scrubbing, SMPTE timecode display, audio volume/muting, start offset cueing, and a pop-out detached window for multi-monitor scoring setups.

---

## Install on an Intel Mac (no Xcode)

GitHub Actions builds the universal `VidSyncVST.vst3` (`x86_64` + `arm64`). Download it and copy it into Reason's VST3 folder. Xcode and CMake are not required.

1. Download [VidSyncVST_Mac.dmg](https://github.com/traysamuels/ScreenScore/releases/download/vidsync-vst3/VidSyncVST_Mac.dmg) or [VidSyncVST.vst3.zip](https://github.com/traysamuels/ScreenScore/releases/download/vidsync-vst3/VidSyncVST.vst3.zip) from the **VidSyncVST macOS universal** release. The same files are the **VidSyncVST-macOS-Universal** artifact on each [macOS workflow run](https://github.com/traysamuels/ScreenScore/actions/workflows/build-dmg.yml).
2. Open the DMG and drag `VidSyncVST.vst3` onto **Drag to VST3 Folder**, or unzip the zip and copy the `VidSyncVST.vst3` folder to `/Library/Audio/Plug-Ins/VST3/`. That is the folder Reason 13 scans. macOS asks for an administrator password.
3. Clear quarantine so Reason can open the downloaded plug-in:

   ```bash
   xattr -dr com.apple.quarantine /Library/Audio/Plug-Ins/VST3/VidSyncVST.vst3
   lipo -info /Library/Audio/Plug-Ins/VST3/VidSyncVST.vst3/Contents/MacOS/VidSyncVST
   ```

   `lipo` must list `x86_64` for an Intel Mac.
4. In Reason, keep `/Library/Audio/Plug-Ins/VST3` enabled under **Settings > Folders**, tick **Show VST3 plugins**, then quit and relaunch Reason.
5. Open **Effects > VST3 Plugins > VidSync > VidSyncVST**, drag it into the rack, and click **Open Plugin Window**.

---

## Key Features vs. VidPlayVST

| Feature | VidSyncVST (Our Plugin) | VidPlayVST |
| :--- | :--- | :--- |
| **Engine** | Native Apple **AVFoundation** (Metal & VideoToolbox) | FFmpeg software library |
| **Mac Performance** | Near 0% CPU; GPU-accelerated 4K & 1080p | Relies on software CPU frame decoding |
| **Formats Supported** | MP4 (H.264, HEVC/H.265), MOV, Apple ProRes (422/4444), M4V | FFmpeg supported codecs |
| **Reason 13 Compatibility** | 64-bit VST3 universal binary (`x86_64` required for Intel) | VST2.4 & VST3 |
| **Scrubbing Sync** | Instant zero-tolerance frame seek when Reason stopped | Keyframe interval dependent |
| **Multi-Monitor Display** | Detachable / Floating Pop-Out Window with Fullscreen | Detached window |
| **Timecode Overlay** | Built-in SMPTE HUD (23.976, 24, 25, 29.97, 30, 60 fps) | SMPTE display |
| **Cue Offset** | Microsecond & frame-accurate start offset parameter | MIDI C-note cue / Offset setting |

---

## Project Structure

```text
VidSyncVST/
├── CMakeLists.txt              # Build configuration with JUCE FetchContent & AVFoundation
├── build_mac.sh                # 1-command build & install script
├── Source/
│   ├── PluginProcessor.h      # DAW transport tracking & APVTS parameter management
│   ├── PluginProcessor.cpp    # AudioPlayHead position querying & Reason state saving
│   ├── PluginEditor.h         # Plugin GUI layout and 60Hz sync timer
│   ├── PluginEditor.cpp       # Parameter attachments & UI bindings
│   ├── VideoPlayerComponent.h # High-level video container & playback state
│   ├── VideoPlayerComponent.cpp # File chooser, pop-out management & transport HUD
│   ├── MacVideoView.h         # Objective-C++ video bridge header
│   ├── MacVideoView.mm        # Native AVPlayer & AVPlayerLayer hardware rendering
│   ├── DetachedVideoWindow.h  # Secondary floating monitor window
│   ├── TimecodeOverlay.h      # SMPTE timecode formatting
│   └── TimecodeMath.h         # Frame-rate and drop-frame counting
├── tests/timecode_test.cpp    # SMPTE counter checks (no JUCE required)
└── README.md
```

---

## Prerequisites for Building on macOS

1. **macOS 10.15 (Catalina) or newer** (compatible through macOS Sonoma, Sequoia, and later).
2. **Xcode** or the Xcode Command Line Tools. Run `xcode-select --install` and wait until it finishes. `xcode-select -p` must print a developer directory before you configure. If `/usr/bin/cc` only reports "No developer tools were found", CMake will fail.
3. **CMake 3.22+** (install via `brew install cmake`).
4. **Git** (for downloading JUCE automatically).

---

## Optional local build

The download above is the install path when Xcode is not installed. To compile on a Mac that already has Xcode or the Command Line Tools, plus CMake 3.22+ and Git, run this from the `VidSyncVST` directory:

```bash
chmod +x build_mac.sh
./build_mac.sh
```

This script will:
1. Use the Xcode generator when full Xcode is selected, or Unix Makefiles when only the Command Line Tools are installed.
2. Delete a CMake cache left by a failed configure or a different generator. A checked-in Unix Makefiles cache used to make a later Xcode configure stop immediately.
3. Fetch **JUCE 8.0.4** via CMake.
4. Compile a universal binary (`arm64` and `x86_64`). If that compile cannot produce an `x86_64` slice, it rebuilds `x86_64` only. Reason 13 on an Intel Mac loads that slice. An arm64-only plug-in will not open there.
5. Install `VidSyncVST.vst3` into the folder Reason scans:
   `/Library/Audio/Plug-Ins/VST3/`

   Reason 13 does not scan `~/Library/Audio/Plug-Ins/VST3` unless you add that folder yourself. Writing to the system folder asks for an administrator password when needed. This project installs a VST3 bundle, which Reason 13 loads. It does not build a legacy `.vst` plug-in.

---

## How to Use in Reason 13

1. **Quit and relaunch Reason 13** after installing, so the VST3 scan runs.
2. **Insert the Plugin**:
   - In Reason's Browser, go to **Effects** > **VST3 Plugins** > **VidSync** > **VidSyncVST**.
   - Drag it into your Reason Rack.
3. **Open the Interface**:
   - Click **Open Plugin Window** on the Reason Rack device.
4. **Load Your Video**:
   - Click **"Open Video File..."** and select your scoring picture (`.mp4`, `.mov`, `.m4v`, or ProRes).
5. **Sync with Reason**:
   - Hit **Spacebar** in Reason: the video starts in sync.
   - Click anywhere in Reason's Sequencer ruler: the video immediately seeks to that exact frame.
   - Adjust **Cue Offset** if your film cue starts at Bar 5 or a specific timecode.
   - Click **"Pop Out Window"** to drag the video display to a second monitor and maximize it.
