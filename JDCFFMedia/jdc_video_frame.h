//
//  jdc_video_frame.h
//  JDCFFPlayer
//
//  Created by ChenJidong on 17/4/8.
//  Copyright © 2017年 jidong. All rights reserved.
//

#ifndef jdc_video_frame_h
#define jdc_video_frame_h

#include "avformat.h"
#include "avcodec.h"
#include "swscale.h"
#include "avutil.h"
#include "imgutils.h"

struct JDCVideoFrame {
    AVFrame *avFrame;
    double pts;
};

typedef struct JDCVideoFrame JDCVideoFrame;

JDCVideoFrame * jdc_video_Frame_alloc();
void jdc_video_Frame_free(JDCVideoFrame *frame);


#endif /* jdc_video_frame_h */
