//
//  JDCMediaPlayer.m
//  JDCFFPlayer
//
//  Created by ChenJidong on 17/5/6.
//  Copyright © 2017年 jidong. All rights reserved.
//

#import "JDCMediaPlayer.h"
#import "jdc_media_player.h"
#import "JDCAVFrame.h"

@interface JDCMediaPlayer(){
    JDCMediaContext *_mediaCtx;
}

@property (nonatomic,strong) NSMutableArray *renderers;

@end

@implementation JDCMediaPlayer

void display(void *target , JDCMediaContext *mCtx , AVFrame *frame){
    JDCMediaPlayer *player = (__bridge JDCMediaPlayer *)target;
    [player p_displayFrame:frame context:mCtx];
}

- (void)p_displayFrame:(AVFrame *)frame context:(JDCMediaContext *)mCtx{
    JDCAVFrame *jdAvFrame = [JDCAVFrame new];
    jdAvFrame.avFrame = frame;
    jdAvFrame.frameWidth = frame->width;
    jdAvFrame.frameHeight = frame->height;
    jdAvFrame.mCtx = mCtx;
    for(id<JDCVideoRenderer> render in self.renderers){
        dispatch_async(dispatch_get_main_queue(),^{
            [render render:jdAvFrame];
        });
    }
}

+ (void)initialize
{
    //        NSString *path = [[NSBundle mainBundle] pathForResource:@"test" ofType:@"MOV"];
    
    jdc_media_init();
    jdc_sdl_init();
}

- (instancetype)initWithURL:(NSURL *)url
{
    if (self = [super init]) {
        JDCMediaContext *mCtx = jdc_media_open_input([url.absoluteString UTF8String], NULL);
        _mediaCtx = mCtx;
        jdc_media_set_display_callback((__bridge void *)(self),mCtx, display);
    }
    
    return self;
}

-  (NSMutableArray *)renderers
{
    if (!_renderers) {
        _renderers = [NSMutableArray array];
    }
    
    return _renderers;
}

- (void)addRenderer:(id<JDCVideoRenderer>)renderer
{
    [self.renderers addObject:renderer];
}

- (void)start
{
    if (_mediaCtx == NULL) {
        return ;
    }
    
    jdc_media_start(_mediaCtx);
}

- (void)pause
{
    
}

- (void)stop
{
    
}

- (void)shutdown
{
    
}

- (void)resume
{
    
}


@end
