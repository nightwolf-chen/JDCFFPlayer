//
//  jdc_sdl.h
//  JDCFFPlayer
//
//  Created by ChenJidong on 17/4/2.
//  Copyright © 2017年 jidong. All rights reserved.
//

#ifndef jdc_sdl_h
#define jdc_sdl_h

#include "SDL.h"
#include "avformat.h"
#include "avcodec.h"
#include "swscale.h"
#include "avutil.h"
#include "imgutils.h"
#include "jdc_sdl_queue.h"
#include "swresample.h"

struct JDCMediaContext;
//typedef struct JDCMediaContext JDCMediaContext;

typedef struct JDCSDLContext{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    AVFrame *frame;
    struct SwsContext *swsCtx;
    
    JDCSDLPacketQueue *audioQueue;
    JDCSDLPacketQueue *videoQueue;
}JDCSDLContext;

int jdc_sdl_init();
int jdc_sdl_present_frame(JDCSDLContext *sdlCtx, AVFrame *pFrame);

JDCSDLContext *jdc_sdl_create_context(struct JDCMediaContext *mCtx);

int jdc_sdl_play_audio(struct JDCMediaContext *mCtx , JDCSDLContext *sdlCtx);


#endif /* jdc_sdl_h */
