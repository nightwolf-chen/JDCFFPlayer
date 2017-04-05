//
//  jdc_media_player.c
//  JDCFFPlayer
//
//  Created by maochengrui on 01/04/2017.
//  Copyright © 2017 jidong. All rights reserved.
//

#include "jdc_media_player.h"


int jdc_media_init()
{
    av_register_all();
    return 0;
}


JDCMediaContext *jdc_media_open_input(const char *url,JDCError **error)
{
    JDCMediaContext *mCtx = (JDCMediaContext *)av_mallocz(sizeof(JDCMediaContext));
    AVFormatContext *pFmtCtx = avformat_alloc_context();
    if (avformat_open_input(&pFmtCtx, url, NULL, NULL) != 0) {
        av_free(mCtx);
        return NULL;
    }
    mCtx->fmtCtx = pFmtCtx;
    
    if (avformat_find_stream_info(pFmtCtx, NULL) < 0) {
        av_free(mCtx);
        return NULL;
    }
    
    
    for(int i = 0 ; i < pFmtCtx->nb_streams ; i++){
        if(pFmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO &&
           mCtx->videoStream == NULL){
            mCtx->videoStream = pFmtCtx->streams[i];
        }
        
        if(pFmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO &&
           mCtx->audioStream == NULL){
            mCtx->audioStream = pFmtCtx->streams[i];
        }
    }
    
    if (mCtx->videoStream == NULL) {
        av_free(mCtx);
        return NULL;
    }
    
    enum AVCodecID code_id = mCtx->videoStream->codecpar->codec_id;
    AVCodec *pCodec = NULL;
    pCodec = avcodec_find_decoder(code_id);
    if (pCodec == NULL) {
        fprintf(stderr, "Unsuppored codec!");
        av_free(mCtx);
        return NULL;
    }
    
    mCtx->codecVideo = pCodec;
    
    mCtx->codecAudio = avcodec_find_decoder(mCtx->audioStream->codecpar->codec_id);
    if (mCtx->audioStream == NULL) {
        fprintf(stderr, "Unsuppored audio codec!");
        av_free(mCtx);
        return NULL;
    }
    
    mCtx->codecCtxVideo = avcodec_alloc_context3(mCtx->codecVideo);
    if (avcodec_parameters_to_context(mCtx->codecCtxVideo, mCtx->videoStream->codecpar) < 0) {
        fprintf(stderr, "avcodec_paramters copy failed");
        av_free(mCtx);
        return NULL;
    }
    AVDictionary *dic = NULL;
    avcodec_open2(mCtx->codecCtxVideo, pCodec, &dic);
    
    mCtx->codecCtxAudio = avcodec_alloc_context3(mCtx->codecAudio);
    if (avcodec_parameters_to_context(mCtx->codecCtxAudio, mCtx->audioStream->codecpar)) {
        fprintf(stderr, "avcodec_paramters copy failed");
        av_free(mCtx);
        return NULL;
    }
    avcodec_open2(mCtx->codecCtxAudio, mCtx->codecAudio, &dic);
    
    mCtx->sldCtx = jdc_sdl_create_context(mCtx);
    
    
    return mCtx;
}


int jdc_media_play(JDCMediaContext *mCtx , JDCSDLContext *sdlCtx)

{
    AVFrame *pFrame = NULL;
    pFrame = av_frame_alloc();
    if (pFrame == NULL) {
        return -1;
    }
    
    jdc_sdl_play_audio(mCtx, sdlCtx);
    
    int numBytes;
    numBytes = av_image_get_buffer_size(AV_PIX_FMT_YUV420P,
                                  mCtx->codecCtxVideo->width,
                                  mCtx->codecCtxVideo->height,
                                        1);
    AVPacket packet;
    av_new_packet(&packet, numBytes);
    
    while(av_read_frame(mCtx->fmtCtx, &packet) >= 0){
        if (packet.stream_index == mCtx->videoStream->index) {
            continue;
            int r = avcodec_send_packet(mCtx->codecCtxVideo, &packet);
            if (r != 0) {
                av_packet_unref(&packet);
                //return -1;
                continue;
            }
            r = avcodec_receive_frame(mCtx->codecCtxVideo, pFrame);
            if (r != 0) {
                av_packet_unref(&packet);
                //return -1;
                continue;
            }else{
                jdc_sdl_present_frame(sdlCtx, pFrame);
            }
        }else if(packet.stream_index == mCtx->audioStream->index){
            jdc_packet_queue_push(sdlCtx->audioQueue, &packet);
        }
        
       // av_packet_unref(&packet);
    }
    
    
    return 0;
    
}
