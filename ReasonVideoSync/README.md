# ReasonVideoSync — macOS VST3

A macOS VST3 video player intended to follow the host transport in Reason 13.

## Build in GitHub Actions

Open the **Actions** tab and run **Build ReasonVideoSync VST3**, or push a change under `ReasonVideoSync/`. The workflow builds a universal macOS VST3 for Apple Silicon and Intel and uploads it as a downloadable artifact.

## Install locally

1. Download the `ReasonVideoSync-macOS-Universal` artifact from the successful Actions run.
2. Unzip the downloaded artifact.
3. Copy `ReasonVideoSync.vst3` into `~/Library/Audio/Plug-Ins/VST3/`.
4. Remove quarantine from this local development build if macOS blocks it:

   ```bash
   xattr -dr com.apple.quarantine "$HOME/Library/Audio/Plug-Ins/VST3/ReasonVideoSync.vst3"
   ```

5. Restart Reason 13 and rescan VST3 plug-ins.

## Signing

The workflow creates an ad-hoc signed local development bundle. It does not require a paid Apple Developer Program membership. It is not Developer ID signed or notarized and is not suitable for frictionless public distribution.

## Scope and limitations

- VST3 only; no VST2 or Standalone target.
- Requires macOS and Apple's AVFoundation/AVKit frameworks.
- Transport synchronization and video playback must still be validated inside Reason 13 on the target Mac.
