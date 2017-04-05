//
//  jdc_sdl_queue.c
//  JDCFFPlayer
//
//  Created by ChenJidong on 17/4/4.
//  Copyright © 2017年 jidong. All rights reserved.
//

#include "jdc_sdl_queue.h"

JDCSDLPacketQueue *jdc_packet_queue_alloc()
{
    return (JDCSDLPacketQueue *)av_mallocz(sizeof(JDCSDLPacketQueue));
}

void jdc_packet_queue_init(JDCSDLPacketQueue *queue)
{
    memset(queue, 0, sizeof(JDCSDLPacketQueue));
    queue->mutex = SDL_CreateMutex();
    queue->cond = SDL_CreateCond();
}

int jdc_packet_queue_push(JDCSDLPacketQueue *queue , AVPacket *packet)
{
    AVPacketList *listNode = (AVPacketList *)malloc(sizeof(AVPacketList));
    
    if (!listNode) {
        return -1;
    }
    
    listNode->pkt = *packet;
    listNode->next = NULL;
    
    SDL_LockMutex(queue->mutex);
    
    if (queue->first_pk == NULL) {
        queue->first_pk = listNode;
    }else{
        queue->last_pk->next = listNode;
    }
    
    queue->last_pk = listNode;
    queue->nb_packets++;
    queue->size += listNode->pkt.size;
    
    
    SDL_CondSignal(queue->cond);
    SDL_UnlockMutex(queue->mutex);
    
    return 0;
}

AVPacket *jdc_packet_queue_front(JDCSDLPacketQueue *queue)
{
    SDL_LockMutex(queue->mutex);
    
    AVPacket *pkt = NULL;
    if (queue->first_pk) {
        pkt = &(queue->first_pk->pkt);
    }
    
    SDL_UnlockMutex(queue->mutex);
    
    return pkt;
}

AVPacket *jdc_packet_queue_pop(JDCSDLPacketQueue *queue)
{
    AVPacket *pkt = NULL;
    SDL_LockMutex(queue->mutex);
    
    if (queue->first_pk) {
        AVPacketList *firstPkl = queue->first_pk;
        pkt = &(queue->first_pk->pkt);
        queue->first_pk = queue->first_pk->next;
        queue->nb_packets--;
        queue->size -= pkt->size;
        
        free(firstPkl);
    }
    
    SDL_UnlockMutex(queue->mutex);
    
    return pkt;
}

int jdc_packet_queue_get_packet(JDCSDLPacketQueue *queue , AVPacket *pkg , int block)
{
    int ret;
    
    SDL_LockMutex(queue->mutex);
    
    while (1) {
        if (queue->quit) {
            ret = -1;
            break;
        }
        
        if (queue->first_pk) {
            *pkg = *(jdc_packet_queue_pop(queue));
            ret = 1;
            break;
        }else if(block){
            SDL_CondWait(queue->cond, queue->mutex);
        }else{
            ret = 0;
            break;
        }
    }
    
    SDL_UnlockMutex(queue->mutex);
    
    return ret;
}
