//
//  main.m
//  JDCFFPlayer
//
//  Created by maochengrui on 01/04/2017.
//  Copyright © 2017 jidong. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "AppDelegate.h"

#import "jdc_media_player.h"

int main(int argc, char * argv[]) {
    @autoreleasepool {
        //return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
        NSString *path = [[NSBundle mainBundle] pathForResource:@"war3end" ofType:@"mp4"];
//        NSString *path = [[NSBundle mainBundle] pathForResource:@"test" ofType:@"MOV"];
        
        jdc_media_init();
        jdc_sdl_init();
        JDCMediaContext *mCtx = jdc_media_open_input([path UTF8String], NULL);
        jdc_media_play(mCtx);
        
        return 0;
    }
}
