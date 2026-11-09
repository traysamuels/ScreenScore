#include "MacVideoView.h"

#include <cmath>

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
    bool hasRequestedSeek { false };
    double lastSeekTarget { 0.0 };
    double lastSeekWallSeconds { -1.0 };

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

            CMTime dur = asset.duration;
            if (CMTIME_IS_NUMERIC(dur))
                durationSeconds = CMTimeGetSeconds(dur);
            else
                durationSeconds = 0.0;

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
            hasRequestedSeek = false;
            return true;
        }
    }

    void sync(double dawSeconds, bool isPlaying, double offsetSeconds) {
        @autoreleasepool {
            if (!loaded || player == nil) return;

            double targetTime = dawSeconds - offsetSeconds;
            if (targetTime < 0.0) targetTime = 0.0;
            if (std::isfinite(durationSeconds) && durationSeconds > 0.0 && targetTime > durationSeconds)
                targetTime = durationSeconds;

            const double now = CFAbsoluteTimeGetCurrent();
            double currentVideoTime = CMTimeGetSeconds(player.currentTime);
            const bool currentValid = std::isfinite(currentVideoTime);
            const double drift = currentValid ? std::abs(currentVideoTime - targetTime) : 1.0e9;
            CMTime cmTarget = CMTimeMakeWithSeconds(targetTime, 60000);

            // The UI calls this at 60 Hz. Seeking on every call, including when
            // the target has not moved, cancels AVPlayer and the picture never
            // settles or plays. Seek only when transport state or the requested
            // time actually changes, and rate-limit corrections while rolling.
            if (isPlaying) {
                const bool cooldownElapsed = (now - lastSeekWallSeconds) > 0.30;
                if ((!wasHostPlaying || drift > 0.15) && cooldownElapsed) {
                    CMTime tolerance = CMTimeMakeWithSeconds(0.08, 600);
                    [player seekToTime:cmTarget toleranceBefore:tolerance toleranceAfter:tolerance];
                    lastSeekWallSeconds = now;
                    lastSeekTarget = targetTime;
                    hasRequestedSeek = true;
                }
                if (player.rate < 0.05f)
                    [player play];
                wasHostPlaying = true;
            } else {
                if (wasHostPlaying || player.rate > 0.05f)
                    [player pause];
                wasHostPlaying = false;

                const bool targetMoved = !hasRequestedSeek || std::abs(targetTime - lastSeekTarget) > 0.02;
                if (targetMoved) {
                    [player seekToTime:cmTarget toleranceBefore:kCMTimeZero toleranceAfter:kCMTimeZero];
                    lastSeekTarget = targetTime;
                    lastSeekWallSeconds = now;
                    hasRequestedSeek = true;
                }
            }
        }
    }

    void seek(double targetSeconds) {
        @autoreleasepool {
            if (!loaded || player == nil) return;
            if (targetSeconds < 0.0) targetSeconds = 0.0;
            CMTime cmTarget = CMTimeMakeWithSeconds(targetSeconds, 60000);
            [player seekToTime:cmTarget toleranceBefore:kCMTimeZero toleranceAfter:kCMTimeZero];
            lastSeekTarget = targetSeconds;
            lastSeekWallSeconds = CFAbsoluteTimeGetCurrent();
            hasRequestedSeek = true;
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
