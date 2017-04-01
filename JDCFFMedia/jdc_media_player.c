//
//  jdc_media_player.c
//  JDCFFPlayer
//
//  Created by maochengrui on 01/04/2017.
//  Copyright © 2017 jidong. All rights reserved.
//

#include "jdc_media_player.h"

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>

int jdc_media_init()
{
    av_register_all();
    return 0;
}


/**
 打开一个url的多媒体文件

 @param context 上下文
 @param url 文件的url
 @return 0为正常，-1打开失败
 */
int jdc_media_open_input(AVFormatContext **context, const char *url)
{
    if (avformat_open_input(context, url, NULL, NULL) != 0) {
        return -1;
    }
    
    if (avformat_find_stream_info(*context, NULL) < 0) {
        return -1;
    }
    
#if DEBUG
    av_dump_format(*context, 0, url, 0);
#endif
    
    int videoStream = -1;
    AVCodecContext *pCodecCtx = NULL;
    for(int i = 0 ; i < (*context)->nb_streams ; i++){
        if((*context)->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
            videoStream = i;
            break;
        }
    }
    
    if (videoStream == -1) {
        return -1;
    }
    
    enum AVCodecID code_id = (*context)->streams[videoStream]->codecpar->codec_id;
    AVCodec *pCodec = NULL;
    pCodec = avcodec_find_decoder(code_id);
    if (pCodec == NULL) {
        fprintf(stderr, "Unsuppored codec!");
        return -1;
    }
    
    pCodecCtx = avcodec_alloc_context3(pCodec);
    if (avcodec_parameters_to_context(pCodecCtx, (*context)->streams[videoStream]->codecpar) < 0) {
        fprintf(stderr, "avcodec_paramters copy failed");
        return -1;
    }
    
    
    return 0;
}

int jdc_media_get_frame(AVCodecContext *pCodecCtx , AVFormatContext *pFmtCtx)
{
    AVFrame *pFrame = NULL;
    pFrame = av_frame_alloc();
    if (pFrame == NULL) {
        return -1;
    }
    
    AVFrame *pFrameRGB = NULL;
    pFrameRGB = av_frame_alloc();
    if (pFrameRGB == NULL) {
        return -1;
    }
    
    uint8_t *buffer = NULL;
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, context->width, context->height, 0);
    buffer = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
    
    av_image_fill_arrays(<#uint8_t **dst_data#>, <#int *dst_linesize#>, <#const uint8_t *src#>, <#enum AVPixelFormat pix_fmt#>, <#int width#>, <#int height#>, <#int align#>)
    
}
