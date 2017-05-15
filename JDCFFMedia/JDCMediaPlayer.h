//
//  JDCMediaPlayer.h
//  JDCFFPlayer
//
//  Created by ChenJidong on 17/5/6.
//  Copyright © 2017年 jidong. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "JDCVideoRenderer.h"

@interface JDCMediaPlayer : NSObject

- (instancetype)initWithURL:(NSURL *)url;

- (void)addRenderer:(id<JDCVideoRenderer>)renderer;

- (void)start;
- (void)pause;
- (void)stop;
- (void)shutdown;
- (void)resume;

@end
