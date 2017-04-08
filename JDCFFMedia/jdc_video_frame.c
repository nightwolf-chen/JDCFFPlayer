//
//  jdc_video_frame.c
//  JDCFFPlayer
//
//  Created by ChenJidong on 17/4/8.
//  Copyright © 2017年 jidong. All rights reserved.
//

#include "jdc_video_frame.h"

JDCVideoFrame * jdc_video_Frame_alloc()
{
    return av_mallocz(sizeof(JDCVideoFrame));
}

void jdc_video_Frame_free(JDCVideoFrame *frame)
{
    if (frame->avFrame) {
        av_frame_unref(frame->avFrame);
        av_frame_free(&frame->avFrame);
    }
    
    av_free(frame);
}
