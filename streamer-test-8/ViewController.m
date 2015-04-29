//
//  ViewController.m
//  streamer-test-8
//
//  Created by Valerii Lider on 4/29/15.
//  Copyright (c) 2015 Valerii Lider. All rights reserved.
//

#import "ViewController.h"
#import "Live555Wrapper.h"

#include "Global.h"

@import AVFoundation;

@interface ViewController ()
@property (nonatomic, weak) IBOutlet UIButton *startButton;
@property (nonatomic, strong) Live555Wrapper *live555Wrapper;
@property (nonatomic, strong) AVAudioEngine *audioEngine;
@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (IBAction)onStartButton:(id)sender {
    
    ((UIButton *)sender).enabled = NO;
    
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        
        [[Live555Wrapper sharedInstance] publish];
        
        self.audioEngine = [[AVAudioEngine alloc] init];
        
        AVAudioInputNode *inputNode = [self.audioEngine inputNode];
        AVAudioFormat *format = [inputNode inputFormatForBus:0];
        [inputNode installTapOnBus:0 bufferSize:8192 format:format block:^(AVAudioPCMBuffer *buffer, AVAudioTime *when) {
            
            UInt32 frames = 0;
            for (UInt32 i = 0; i < buffer.audioBufferList->mNumberBuffers; i++) {
                Float32 *data = buffer.audioBufferList->mBuffers[i].mData;
                frames = buffer.audioBufferList->mBuffers[i].mDataByteSize / sizeof(Float32);
                
                appendBytesIntoGlobalCircularBuffer((void *)data, (int32_t)buffer.audioBufferList->mBuffers[i].mDataByteSize);
            }
            /// notifies StreamDeviceSource that new data available
            [[Live555Wrapper sharedInstance] newFramesAvailable];
        }];
        
        AVAudioMixerNode *mixerNode = [self.audioEngine mainMixerNode];
        [self.audioEngine connect:inputNode to:mixerNode format:[inputNode inputFormatForBus:0]];
        
//        AVAudioOutputNode *outputNode = [self.audioEngine outputNode];
//        [self.audioEngine connect:mixerNode to:outputNode format:[outputNode inputFormatForBus:0]];
        
        NSError *error = nil;
        [self.audioEngine startAndReturnError:&error];
        if (error) {
            NSLog(@"error: %@", error);
            return;
        }
    });
}

@end
