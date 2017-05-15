//
//  JDCVideoRenderer.h
//  JDCFFPlayer
//
//  Created by maochengrui on 05/05/2017.
//  Copyright © 2017 jidong. All rights reserved.
//

#import <Foundation/Foundation.h>

@class JDCAVFrame;

@protocol JDCVideoRenderer <NSObject>

@required
- (void)render:(JDCAVFrame *)frame;


@end
