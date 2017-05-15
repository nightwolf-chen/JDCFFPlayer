//
//  JDCAVFrame.h
//  JDCFFPlayer
//
//  Created by ChenJidong on 17/5/6.
//  Copyright © 2017年 jidong. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "jdc_media_player.h"

@interface JDCAVFrame : NSObject

@property (nonatomic,assign) AVFrame *avFrame;

@property (nonatomic,assign) float frameWidth;
@property (nonatomic,assign) float frameHeight;
@property (nonatomic,assign) JDCMediaContext *mCtx;

@end
