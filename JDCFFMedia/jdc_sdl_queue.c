//
//  jdc_sdl_queue.c
//  JDCFFPlayer
//
//  Created by ChenJidong on 17/4/4.
//  Copyright © 2017年 jidong. All rights reserved.
//

#include "jdc_sdl_queue.h"

struct JDCSDLPacketQueue {
    void *first_pk;
    void *last_pk;
    int size;
    SDL_mutex *mutex;
    SDL_cond *cond;
    int quit;
};

typedef struct JDCQueueNode {
    struct JDCQueueNode *next;
    void *data;
}JDCQueueNode;

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

int jdc_packet_queue_push(JDCSDLPacketQueue *queue , void *packet)
{
    JDCQueueNode *listNode = (JDCQueueNode *)malloc(sizeof(JDCQueueNode));
    
    if (!listNode) {
        return -1;
    }
    
    listNode->data = packet;
    listNode->next = NULL;
    
    SDL_LockMutex(queue->mutex);
    
    if (queue->first_pk == NULL) {
        queue->first_pk = listNode;
    }else{
        ((JDCQueueNode *)queue->last_pk)->next = listNode;
    }
    
    queue->last_pk = listNode;
    queue->size ++;
    
    
    SDL_CondSignal(queue->cond);
    SDL_UnlockMutex(queue->mutex);
    
    return 0;
}

int jdc_packet_queue_size(JDCSDLPacketQueue *queue)
{
    return queue->size;
}

void *jdc_packet_queue_front(JDCSDLPacketQueue *queue)
{
    SDL_LockMutex(queue->mutex);
    
    AVPacket *pkt = NULL;
    if (queue->first_pk) {
        return ((JDCQueueNode *)queue->first_pk)->data;
    }
    
    SDL_UnlockMutex(queue->mutex);
    
    return pkt;
}

void *jdc_packet_queue_pop(JDCSDLPacketQueue *queue)
{
    void *data = NULL;
    SDL_LockMutex(queue->mutex);
    
    if (queue->first_pk) {
        JDCQueueNode *firstPkl = queue->first_pk;
        data = firstPkl->data;
        queue->first_pk = firstPkl->next;
        queue->size--;
        
        free(firstPkl);
    }
    
    SDL_UnlockMutex(queue->mutex);
    
    return data;
}

int jdc_packet_queue_get_packet(JDCSDLPacketQueue *queue , void **pkg , int block)
{
    int ret;
    
    SDL_LockMutex(queue->mutex);
    
    while (1) {
        if (queue->quit) {
            ret = -1;
            break;
        }
        
        if (queue->first_pk) {
            *pkg = jdc_packet_queue_pop(queue);
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
