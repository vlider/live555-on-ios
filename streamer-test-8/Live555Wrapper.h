//
//  Live555Wrapper.h
//  streamer-test-8
//
//  Created by Valerii Lider on 4/29/15.
//  Copyright (c) 2015 Valerii Lider. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface Live555Wrapper : NSObject

+ (instancetype)sharedInstance;
- (void)publish;
- (void)newFramesAvailable;
@end
