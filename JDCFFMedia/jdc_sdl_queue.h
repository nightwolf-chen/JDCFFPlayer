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

typedef struct JDCSDLPacketQueue {
    AVPacketList *first_pk;
    AVPacketList *last_pk;
    int nb_packets;
    int size;
    SDL_mutex *mutex;
    SDL_cond *cond;
    
    int quit;
}JDCSDLPacketQueue;

JDCSDLPacketQueue *jdc_packet_queue_alloc();

void jdc_packet_queue_init(JDCSDLPacketQueue *queue);

int jdc_packet_queue_push(JDCSDLPacketQueue *queue , AVPacket *packet);

AVPacket *jdc_packet_queue_front(JDCSDLPacketQueue *queue);

AVPacket *jdc_packet_queue_pop(JDCSDLPacketQueue *queue);

int jdc_packet_queue_get_packet(JDCSDLPacketQueue *queue , AVPacket *pkg , int blockThread);


    

#endif /* jdc_sdl_queue_h */
