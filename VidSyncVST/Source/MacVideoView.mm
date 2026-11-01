#include "MacVideoView.h"

#if JUCE_MAC
#import <AVFoundation/AVFoundation.h>
#import <AVKit/AVKit.h>
#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>

@interface VidSyncNativeView : NSView
@property (nonatomic, strong) AVPlayerLayer* playerLayer;
@end

@implementation VidSyncNativeView
- (instancetype)initWithFrame:(NSRect)frameRect {
    self = [super initWithFrame:frameRect];
    if (self) {
        self.wantsLayer = YES;
        self.layer.backgroundColor = [[NSColor blackColor] CGColor];
        _playerLayer = [AVPlayerLayer playerLayerWithPlayer:nil];
        _playerLayer.videoGravity = AVLayerVideoGravityResizeAspect;
        _playerLayer.frame = self.bounds;
        _playerLayer.autoresizingMask = kCALayerWidthSizable | kCALayerHeightSizable;
        [self.layer addSublayer:_playerLayer];
    }
    return self;
}

- (void)layout {
    [super layout];
    [CATransaction begin];
    [CATransaction setDisableActions:YES];
    self.playerLayer.frame = self.bounds;
    [CATransaction commit];
}
@end

struct MacVideoBackend::Impl {
    VidSyncNativeView* nativeView { nil };
    AVPlayer* player { nil };
    AVPlayerItem* playerItem { nil };
    double durationSeconds { 0.0 };
    int videoWidth { 0 };
    int videoHeight { 0 };
    bool loaded { false };
    bool wasHostPlaying { false };

    Impl() {
        @autoreleasepool {
            nativeView = [[VidSyncNativeView alloc] initWithFrame:NSMakeRect(0, 0, 640, 360)];
            player = [[AVPlayer alloc] init];
            player.actionAtItemEnd = AVPlayerActionAtItemEndPause;
            nativeView.playerLayer.player = player;
        }
    }

    ~Impl() {
        @autoreleasepool {
            [player pause];
            nativeView.playerLayer.player = nil;
            player = nil;
            playerItem = nil;
            nativeView = nil;
        }
    }

    bool loadFile(const juce::File& file) {
        @autoreleasepool {
            if (!file.existsAsFile())
                return false;

            NSString* path = [NSString stringWithUTF8String:file.getFullPathName().toRawUTF8()];
            NSURL* url = [NSURL fileURLWithPath:path];
            AVURLAsset* asset = [AVURLAsset URLAssetWithURL:url options:nil];

            playerItem = [AVPlayerItem playerItemWithAsset:asset];
            [player replaceCurrentItemWithPlayerItem:playerItem];

            // Extract duration
            CMTime dur = asset.duration;
            durationSeconds = CMTimeGetSeconds(dur);

            // Extract track dimensions
            NSArray<AVAssetTrack*>* tracks = [asset tracksWithMediaType:AVMediaTypeVideo];
            if (tracks.count > 0) {
                CGSize size = tracks.firstObject.naturalSize;
                videoWidth = (int)size.width;
                videoHeight = (int)size.height;
            } else {
                videoWidth = 1920;
                videoHeight = 1080;
            }

            loaded = true;
            return true;
        }
    }

    void sync(double dawSeconds, bool isPlaying, double offsetSeconds) {
        @autoreleasepool {
            if (!loaded || player == nil) return;

            double targetTime = dawSeconds - offsetSeconds;
            if (targetTime < 0.0) targetTime = 0.0;
            if (durationSeconds > 0.0 && targetTime > durationSeconds) targetTime = durationSeconds;

            CMTime cmTarget = CMTimeMakeWithSeconds(targetTime, 60000);

            if (isPlaying) {
                // When DAW transport is playing:
                double currentVideoTime = CMTimeGetSeconds(player.currentTime);
                double drift = std::abs(currentVideoTime - targetTime);

                // If transport just started or jumped (drift > 40ms / 1 frame)
                if (!wasHostPlaying || drift > 0.04) {
                    [player seekToTime:cmTarget
                       toleranceBefore:kCMTimeZero
                        toleranceAfter:kCMTimeZero
                     completionHandler:^(BOOL) {
                         [player play];
                         [player setRate:1.0f];
                     }];
                } else if (player.rate == 0.0f) {
                    [player play];
                    [player setRate:1.0f];
                }
                wasHostPlaying = true;
            } else {
                // When DAW transport is stopped (user scrubbing or clicking Reason's timeline):
                if (wasHostPlaying) {
                    [player pause];
                    wasHostPlaying = false;
                }

                // Instant frame-accurate seek so composer sees the current picture
                [player seekToTime:cmTarget
                   toleranceBefore:kCMTimeZero
                    toleranceAfter:kCMTimeZero];
            }
        }
    }

    void seek(double targetSeconds) {
        @autoreleasepool {
            if (!loaded || player == nil) return;
            if (targetSeconds < 0.0) targetSeconds = 0.0;
            CMTime cmTarget = CMTimeMakeWithSeconds(targetSeconds, 60000);
            [player seekToTime:cmTarget toleranceBefore:kCMTimeZero toleranceAfter:kCMTimeZero];
        }
    }

    void setVolume(float volume, bool muted) {
        @autoreleasepool {
            if (player != nil) {
                player.muted = muted;
                player.volume = volume;
            }
        }
    }
};

MacVideoBackend::MacVideoBackend() : pImpl(std::make_unique<Impl>()) {}
MacVideoBackend::~MacVideoBackend() = default;

bool MacVideoBackend::loadFile(const juce::File& videoFile) {
    return pImpl->loadFile(videoFile);
}

void MacVideoBackend::sync(double dawSeconds, bool isPlaying, double offsetSeconds) {
    pImpl->sync(dawSeconds, isPlaying, offsetSeconds);
}

void MacVideoBackend::seek(double targetSeconds) {
    pImpl->seek(targetSeconds);
}

void MacVideoBackend::setVolume(float volume, bool muted) {
    pImpl->setVolume(volume, muted);
}

double MacVideoBackend::getDuration() const { return pImpl->durationSeconds; }
juce::Point<int> MacVideoBackend::getNaturalSize() const { return { pImpl->videoWidth, pImpl->videoHeight }; }
void* MacVideoBackend::getNativeView() const { return (__bridge void*)pImpl->nativeView; }
bool MacVideoBackend::isLoaded() const { return pImpl->loaded; }

#else
// Fallback stub for non-Mac compilation checks
struct MacVideoBackend::Impl {};
MacVideoBackend::MacVideoBackend() {}
MacVideoBackend::~MacVideoBackend() {}
bool MacVideoBackend::loadFile(const juce::File&) { return false; }
void MacVideoBackend::sync(double, bool, double) {}
void MacVideoBackend::seek(double) {}
void MacVideoBackend::setVolume(float, bool) {}
double MacVideoBackend::getDuration() const { return 0.0; }
juce::Point<int> MacVideoBackend::getNaturalSize() const { return { 1920, 1080 }; }
void* MacVideoBackend::getNativeView() const { return nullptr; }
bool MacVideoBackend::isLoaded() const { return false; }
#endif
