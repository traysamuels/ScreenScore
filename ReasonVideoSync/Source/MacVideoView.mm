#include "MacVideoView.h"
#import <Cocoa/Cocoa.h>
#import <AVFoundation/AVFoundation.h>
#import <AVKit/AVKit.h>

@interface NativeVideoController : NSObject
@property (strong, nonatomic) AVPlayer* player;
@property (strong, nonatomic) AVPlayerLayer* playerLayer;
@property (strong, nonatomic) NSView* containerView;
@property (nonatomic) double videoDuration;
@property (nonatomic) BOOL isCurrentlyPlaying;
- (void)loadURL:(NSURL*)url;
- (void)seekTo:(double)seconds isPlaying:(BOOL)playing;
@end

@implementation NativeVideoController

- (instancetype)init {
    self = [super init];
    if (self) {
        _containerView = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 640, 360)];
        _containerView.wantsLayer = YES;
        _player = [[AVPlayer alloc] init];
        _player.actionAtItemEnd = AVPlayerActionAtItemEndNone;
        _playerLayer = [AVPlayerLayer playerLayerWithPlayer:_player];
        _playerLayer.videoGravity = AVLayerVideoGravityResizeAspect;
        _playerLayer.autoresizingMask = kCALayerWidthSizable | kCALayerHeightSizable;
        [_containerView.layer addSublayer:_playerLayer];
        _videoDuration = 0.0;
        _isCurrentlyPlaying = NO;
    }
    return self;
}

- (void)loadURL:(NSURL*)url {
    AVURLAsset* asset = [AVURLAsset URLAssetWithURL:url options:nil];
    AVPlayerItem* item = [AVPlayerItem playerItemWithAsset:asset];
    [self.player replaceCurrentItemWithPlayerItem:item];
    CMTime dur = asset.duration;
    self.videoDuration = CMTIME_IS_NUMERIC(dur) ? CMTimeGetSeconds(dur) : 0.0;
    [self.player setMuted:YES];
}

- (void)seekTo:(double)seconds isPlaying:(BOOL)playing {
    if (!self.player.currentItem) return;
    if (seconds < 0.0) seconds = 0.0;
    if (self.videoDuration > 0.0 && seconds > self.videoDuration)
        seconds = self.videoDuration;

    CMTime targetTime = CMTimeMakeWithSeconds(seconds, 600);
    [self.player seekToTime:targetTime toleranceBefore:kCMTimeZero toleranceAfter:kCMTimeZero];

    if (playing) {
        if (self.player.rate == 0.0)
            [self.player play];
    } else if (self.player.rate != 0.0) {
        [self.player pause];
    }
}
@end

MacVideoView::MacVideoView()
{
    NativeVideoController* controller = [[NativeVideoController alloc] init];
    nativePlayerController = (__bridge_retained void*)controller;
    setView((__bridge void*)controller.containerView);
}

MacVideoView::~MacVideoView()
{
    if (nativePlayerController != nullptr)
    {
        NativeVideoController* controller = (__bridge_transfer NativeVideoController*)nativePlayerController;
        [controller.player pause];
        nativePlayerController = nullptr;
    }
}

void MacVideoView::loadVideoFile(const juce::File& file)
{
    if (!nativePlayerController || !file.existsAsFile()) return;
    NativeVideoController* controller = (__bridge NativeVideoController*)nativePlayerController;
    NSString* path = [NSString stringWithUTF8String:file.getFullPathName().toRawUTF8()];
    NSURL* url = [NSURL fileURLWithPath:path];
    [controller loadURL:url];
}

void MacVideoView::setPosition(double seconds, bool isPlaying)
{
    if (!nativePlayerController) return;
    NativeVideoController* controller = (__bridge NativeVideoController*)nativePlayerController;
    [controller seekTo:seconds isPlaying:isPlaying];
}

double MacVideoView::getDuration() const
{
    if (!nativePlayerController) return 0.0;
    NativeVideoController* controller = (__bridge NativeVideoController*)nativePlayerController;
    return controller.videoDuration;
}
