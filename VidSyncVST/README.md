# VidSyncVST — Hardware-Accelerated Video Sync Plugin for Reason 13 (macOS)

**VidSyncVST** is a lightweight, hardware-accelerated VST3 plugin designed specifically for film, TV, commercial, and game composers working in **Reason 13** on Apple macOS (Apple Silicon M1/M2/M3/M4 & Intel).

Because Reason 13 lacks a built-in video sequencer track, VidSyncVST acts as your video monitor window. It synchronizes seamlessly to Reason's transport playhead using native **AVFoundation** hardware decoding, giving you fluid 60 FPS playback, instant frame-accurate timeline scrubbing, SMPTE timecode display, audio volume/muting, start offset cueing, and a pop-out detached window for multi-monitor scoring setups.

---

## Key Features vs. VidPlayVST

| Feature | VidSyncVST (Our Plugin) | VidPlayVST |
| :--- | :--- | :--- |
| **Engine** | Native Apple **AVFoundation** (Metal & VideoToolbox) | FFmpeg software library |
| **Mac Performance** | Near 0% CPU; GPU-accelerated 4K & 1080p | Relies on software CPU frame decoding |
| **Formats Supported** | MP4 (H.264, HEVC/H.265), MOV, Apple ProRes (422/4444), M4V | FFmpeg supported codecs |
| **Reason 13 Compatibility** | Native 64-bit VST3 (and AU) Universal Binary | VST2.4 & VST3 |
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

## 1-Command Build & Installation

Open Terminal, navigate to the `VidSyncVST` directory, and run:

```bash
chmod +x build_mac.sh
./build_mac.sh
```

This script will:
1. Delete an incomplete `build/` directory left by a failed configure. A checked-in Unix Makefiles cache used to make `cmake -G Xcode` stop immediately.
2. Fetch **JUCE 8.0.4** via CMake.
3. Compile a Universal 2 binary (`arm64` for Apple Silicon + `x86_64` for Intel).
4. Install `VidSyncVST.vst3` into the folder Reason actually scans:
   `/Library/Audio/Plug-Ins/VST3/`

   Reason 13 does **not** scan `~/Library/Audio/Plug-Ins/VST3` unless you add that folder yourself. Writing to the system folder asks for an administrator password when needed.

---

## How to Use in Reason 13

1. **Quit and relaunch Reason 13**:
   - Reason scans `/Library/Audio/Plug-Ins/VST3` at launch.
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
