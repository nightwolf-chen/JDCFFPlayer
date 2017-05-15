//
//  jdc_media_player.h
//  JDCFFPlayer
//
//  Created by maochengrui on 01/04/2017.
//  Copyright © 2017 jidong. All rights reserved.
//

#ifndef jdc_media_player_h
#define jdc_media_player_h

#include "avformat.h"
#include "avcodec.h"
#include "swscale.h"
#include "avutil.h"
#include "imgutils.h"
#include "jdc_sdl.h"
#include "jdc_video_frame.h"


struct JDCMediaContext {
    
    AVFormatContext *fmtCtx;
    
    AVCodec *codecVideo;
    AVCodecContext *codecCtxVideo;
    AVStream *videoStream;
    int videoStreamIdx;
    
    AVCodec *codecAudio;
    AVCodecContext *codecCtxAudio;
    AVStream *audioStream;
    int audioStreamIdx;
    
    JDCSDLContext *sdlCtx;
    
    SDL_Thread *parse_tid;
    SDL_Thread *video_tid;
    SDL_Thread *main_tid;
    
    struct SwsContext *swsCtx;
    JDCSDLPacketQueue *audioQueue;
    JDCSDLPacketQueue *videoQueue;
    
    JDCSDLPacketQueue *videoFrameQueue;
    
    char filename[1024];
    
    int quit;
    
    double videoClock;
    double frame_last_pts;
    double frame_last_delay;
    double frame_timer;
    
    double audio_clock;
    int audio_buf_index;
    int audio_buf_size;
    
    void (*display_callback)(void *,struct JDCMediaContext *,AVFrame *);
    void *display_target;
};

typedef struct JDCMediaContext JDCMediaContext;

typedef struct JDCError {
}JDCError;


int jdc_media_init();

JDCMediaContext *jdc_media_open_input(const char *url,JDCError **error);

int jdc_media_play(JDCMediaContext *mCtx);

void jdc_media_set_display_callback(void *target,
                                    JDCMediaContext *mCtx,
                                    void (*callback)(void *target,
                                                     JDCMediaContext *mCtx ,
                                                     AVFrame *frame));

void jdc_media_start(JDCMediaContext *mCtx);

#endif /* jdc_media_player_h */
