//
//  JDCVideoRenderer.h
//  JDCFFPlayer
//
//  Created by maochengrui on 05/05/2017.
//  Copyright © 2017 jidong. All rights reserved.
//

#import <Foundation/Foundation.h>

@class JDCVideoFrame;

@protocol JDCVideoRenderer <NSObject>

@required
- (void)render:(JDCVideoFrame *)frame;


@end
