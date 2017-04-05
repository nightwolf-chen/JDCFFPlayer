//
//  jdc_sdl_queue.h
//  JDCFFPlayer
//
//  Created by ChenJidong on 17/4/4.
//  Copyright © 2017年 jidong. All rights reserved.
//

#ifndef jdc_sdl_queue_h
#define jdc_sdl_queue_h

#include "avformat.h"
#include "avcodec.h"
#include "swscale.h"
#include "avutil.h"
#include "imgutils.h"
#include "SDL.h"

struct JDCSDLPacketQueue;

typedef struct JDCSDLPacketQueue JDCSDLPacketQueue;


JDCSDLPacketQueue *jdc_packet_queue_alloc();

void jdc_packet_queue_init(JDCSDLPacketQueue *queue);

int jdc_packet_queue_size(JDCSDLPacketQueue *queue);

int jdc_packet_queue_push(JDCSDLPacketQueue *queue , void *data);

void *jdc_packet_queue_front(JDCSDLPacketQueue *queue);

void *jdc_packet_queue_pop(JDCSDLPacketQueue *queue);

int jdc_packet_queue_get_packet(JDCSDLPacketQueue *queue , void **data , int blockThread);


    

#endif /* jdc_sdl_queue_h */
