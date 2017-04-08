//
//  jdc_sdl.c
//  JDCFFPlayer
//
//  Created by ChenJidong on 17/4/2.
//  Copyright © 2017年 jidong. All rights reserved.
//

#include "jdc_sdl.h"
#include "jdc_media_player.h"
#include "opt.h"

#include <assert.h>

#define SDL_AUDIO_BUFFER_SIZE 1024

int jdc_sdl_init(){
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        return -1;
    }
    return 0;
}

int AudioResampling(AVCodecContext * audio_dec_ctx,
                    AVFrame * pAudioDecodeFrame,
                    int out_sample_fmt,
                    int out_channels,
                    int out_sample_rate,uint8_t *audio_chunk)
{
    SwrContext * swr_ctx = NULL;
    int data_size = 0;
    int ret = 0;
    int64_t src_ch_layout = audio_dec_ctx->channel_layout;
    int64_t dst_ch_layout = AV_CH_LAYOUT_STEREO;
    int dst_nb_channels = 0;
    int dst_linesize = 0;
    int src_nb_samples = 0;
    int dst_nb_samples = 0;
    int max_dst_nb_samples = 0;
    uint8_t **dst_data = NULL;
    int resampled_data_size = 0;
    
    swr_ctx = swr_alloc();
    if (!swr_ctx)
    {
        printf("swr_alloc error \n");
        return -1;
    }
    
    src_ch_layout = (audio_dec_ctx->channels ==
                     av_get_channel_layout_nb_channels(audio_dec_ctx->channel_layout)) ?
    audio_dec_ctx->channel_layout :
    av_get_default_channel_layout(audio_dec_ctx->channels);
    
    if (out_channels == 1)
    {
        dst_ch_layout = AV_CH_LAYOUT_MONO;
        //printf("dst_ch_layout: AV_CH_LAYOUT_MONO\n");
    }
    else if (out_channels == 2)
    {
        dst_ch_layout = AV_CH_LAYOUT_STEREO;
        //printf("dst_ch_layout: AV_CH_LAYOUT_STEREO\n");
    }
    else
    {
        dst_ch_layout = AV_CH_LAYOUT_SURROUND;
        //printf("dst_ch_layout: AV_CH_LAYOUT_SURROUND\n");
    }
    
    if (src_ch_layout <= 0)
    {
        printf("src_ch_layout error \n");
        return -1;
    }
    
    src_nb_samples = pAudioDecodeFrame->nb_samples;
    if (src_nb_samples <= 0)
    {
        printf("src_nb_samples error \n");
        return -1;
    }
    
    av_opt_set_int(swr_ctx, "in_channel_layout", src_ch_layout, 0);
    av_opt_set_int(swr_ctx, "in_sample_rate", audio_dec_ctx->sample_rate, 0);
    av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", audio_dec_ctx->sample_fmt, 0);
    
    av_opt_set_int(swr_ctx, "out_channel_layout", dst_ch_layout, 0);
    av_opt_set_int(swr_ctx, "out_sample_rate", out_sample_rate, 0);
    av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", (enum AVSampleFormat)out_sample_fmt, 0);
    
    if ((ret = swr_init(swr_ctx)) < 0) {
        printf("Failed to initialize the resampling context\n");
        return -1;
    }
    
    max_dst_nb_samples = dst_nb_samples = av_rescale_rnd(src_nb_samples,
                                                         out_sample_rate, audio_dec_ctx->sample_rate, AV_ROUND_UP);
    if (max_dst_nb_samples <= 0)
    {
        printf("av_rescale_rnd error \n");
        return -1;
    }
    
    dst_nb_channels = av_get_channel_layout_nb_channels(dst_ch_layout);
    ret = av_samples_alloc_array_and_samples(&dst_data, &dst_linesize, dst_nb_channels,
                                             dst_nb_samples, (enum AVSampleFormat)out_sample_fmt, 0);
    if (ret < 0)
    {
        printf("av_samples_alloc_array_and_samples error \n");
        return -1;
    }
    
    
    dst_nb_samples = av_rescale_rnd(swr_get_delay(swr_ctx, audio_dec_ctx->sample_rate) +
                                    src_nb_samples, out_sample_rate, audio_dec_ctx->sample_rate, AV_ROUND_UP);
    if (dst_nb_samples <= 0)
    {
        printf("av_rescale_rnd error \n");
        return -1;
    }
    if (dst_nb_samples > max_dst_nb_samples)
    {
        av_free(dst_data[0]);
        ret = av_samples_alloc(dst_data, &dst_linesize, dst_nb_channels,
                               dst_nb_samples, (enum AVSampleFormat)out_sample_fmt, 1);
        max_dst_nb_samples = dst_nb_samples;
    }
    
    if (swr_ctx)
    {
        ret = swr_convert(swr_ctx, dst_data, dst_nb_samples,
                          (const uint8_t **)pAudioDecodeFrame->data, pAudioDecodeFrame->nb_samples);
        if (ret < 0)
        {
            printf("swr_convert error \n");
            return -1;
        }
        
        resampled_data_size = av_samples_get_buffer_size(&dst_linesize, dst_nb_channels,
                                                         ret, (enum AVSampleFormat)out_sample_fmt, 1);
        if (resampled_data_size < 0)
        {
            printf("av_samples_get_buffer_size error \n");
            return -1;
        }
    }
    else
    {
        printf("swr_ctx null error \n");
        return -1;
    }
    
    if (!audio_chunk){
        audio_chunk = (uint8_t *)av_malloc(resampled_data_size*sizeof(uint8_t));
    }
    
    memcpy(audio_chunk, dst_data[0], resampled_data_size);  
    
    if (dst_data)  
    {  
        av_freep(&dst_data[0]);  
    }  
    av_freep(&dst_data);  
    dst_data = NULL;  
    
    if (swr_ctx)  
    {  
        swr_free(&swr_ctx);  
    }  
    return resampled_data_size;  
}

int jdc_sdl_audio_decode_frame(AVCodecContext *aCodecCtx,
                       uint8_t *audio_buf,
                       int buf_size,
                       JDCMediaContext *mCtx,double *pts_ptr)
{
    AVPacket *pkt = NULL;
    static AVFrame frame;
    
    int len1, data_size = 0;
    double pts;
    
    while(1){
        
        if (pkt != NULL && pkt->data != NULL) {
            
            if  (avcodec_send_packet(aCodecCtx, pkt) < 0) {
                av_packet_unref(pkt);
                av_packet_free(&pkt);
                return -1;
            }
            
            data_size = 0;
            while(avcodec_receive_frame(aCodecCtx, &frame) >= 0){
                
                len1 = frame.linesize[0];
                if(len1 < 0) {
                    /* if error, skip frame */
                    break;
                }
                
                int fData_size = 0;
                fData_size = av_samples_get_buffer_size(NULL,
                                                       aCodecCtx->channels,
                                                       frame.nb_samples,
                                                       aCodecCtx->sample_fmt,
                                                       1);
                assert(fData_size <= buf_size);
                memcpy(audio_buf+data_size, frame.data[0], fData_size);
                fData_size = AudioResampling(aCodecCtx,
                                             &frame,
                                             AV_SAMPLE_FMT_S16,
                                             2,
                                             44100,audio_buf+data_size);
                data_size += fData_size;
            }
            
            if (data_size > 0) {
                av_packet_unref(pkt);
                av_packet_free(&pkt);
                return data_size;
            }
            
            av_packet_unref(pkt);
            av_packet_free(&pkt);
            
        }
        
        pts = mCtx->audio_clock;
        *pts_ptr = pts;
        double n = 2 * mCtx->codecCtxAudio->channels;
        mCtx->audio_clock += (double)data_size /
        (double)(n * mCtx->codecCtxAudio->sample_rate);
        
        if(mCtx->quit) {
            return -1;
        }
        
        if(jdc_packet_queue_get_packet(mCtx->audioQueue, (void **)&pkt, 1)< 0) {
            return -1;
        }
        
        /* if update, update the audio clock w/pts */
        if(pkt->pts != AV_NOPTS_VALUE) {
            mCtx->audio_clock = av_q2d(mCtx->audioStream->time_base)*pkt->pts;
        }
    }
    
    return -1;
}

void jdc_sdl_audio_callback(void *userdata, Uint8 * stream,int len)
{
    JDCMediaContext *mCtx = (JDCMediaContext *)userdata;
    AVCodecContext *codecCtx = mCtx->codecCtxAudio;
    int len1,audio_size;
    static uint8_t audio_buf[192000 * 4 / 2];
    double pts;
    
    while(len > 0) {
        if(mCtx->audio_buf_index >= mCtx->audio_buf_size) {
            /* We have already sent all our data; get more */
            audio_size = jdc_sdl_audio_decode_frame(codecCtx,
                                            audio_buf,
                                            sizeof(audio_buf),
                                            mCtx,&pts);
            if(audio_size < 0) {
                /* If error, output silence */
                mCtx->audio_buf_size = 1024;
                memset(audio_buf, 0,mCtx->audio_buf_size);
            } else {
                mCtx-> audio_buf_size = audio_size;
            }
            mCtx->audio_buf_index = 0;
        }
        
        len1 = mCtx->audio_buf_size - mCtx->audio_buf_index;
        
        if(len1 > len) len1 = len;
        
        //SDL_memset(stream, 0, len);
        
        memcpy(stream, (uint8_t *)audio_buf + mCtx->audio_buf_index, len1);
        
        //SDL_MixAudio(stream, (uint8_t *)audio_buf + audio_buf_index , len1, 100);
        
        len -= len1;
        stream += len1;
        mCtx->audio_buf_index += len1;
    }
    
}

JDCSDLContext *jdc_sdl_create_context(struct JDCMediaContext *mCtx)
{
    JDCSDLContext *sdlCtx = (JDCSDLContext *)av_mallocz(sizeof(JDCSDLContext));
    SDL_Window *window = NULL;
    
    window = SDL_CreateWindow("video",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              mCtx->codecCtxVideo->width,
                              mCtx->codecCtxVideo->height,
                              SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL |SDL_WINDOW_BORDERLESS);
    if (window == NULL) {
        av_free(sdlCtx);
        return NULL;
    }
    
    sdlCtx->window = window;
    
    SDL_Renderer *pRenderer = SDL_CreateRenderer(sdlCtx->window, -1, 0);
    if (pRenderer == NULL) {
        av_free(sdlCtx);
        return NULL;
    }
    
    sdlCtx->renderer = pRenderer;
    
    SDL_Texture *pTexture = SDL_CreateTexture(pRenderer,
                                              SDL_PIXELFORMAT_IYUV,
                                              SDL_TEXTUREACCESS_STREAMING,
                                              mCtx->codecCtxVideo->width,
                                              mCtx->codecCtxVideo->height);
    if (pTexture == NULL) {
        av_free(sdlCtx);
        return NULL;
    }
    
    SDL_SetTextureBlendMode(pTexture,SDL_BLENDMODE_BLEND);
    
    sdlCtx->texture = pTexture;
    
    enum AVPixelFormat pixFmt = AV_PIX_FMT_YUV420P;
    struct SwsContext *swsCtx = sws_getContext(mCtx->codecCtxVideo->width,
                                               mCtx->codecCtxVideo->height,
                                               mCtx->codecCtxVideo->pix_fmt,
                                               mCtx->codecCtxVideo->width,
                                               mCtx->codecCtxVideo->height,
                                               pixFmt,
                                               SWS_BICUBIC,
                                               NULL,
                                               NULL,
                                               NULL);
    
    if (swsCtx == NULL) {
        av_free(sdlCtx);
        return NULL;
    }
    
   // sdlCtx->swsCtx = swsCtx;
    
    
    AVFrame *pFrameYUV = NULL;
    pFrameYUV = av_frame_alloc();
    if (pFrameYUV == NULL) {
        av_free(sdlCtx);
        av_frame_free(&pFrameYUV);
        return NULL;
    }
    
    uint8_t *buffer = NULL;
    int numBytes = av_image_get_buffer_size(pixFmt,
                                            mCtx->codecCtxVideo->width,
                                            mCtx->codecCtxVideo->height,
                                            1);
    buffer = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
    
    int fCode = av_image_fill_arrays(pFrameYUV->data,
                         pFrameYUV->linesize,
                         buffer,
                         pixFmt,
                         mCtx->codecCtxVideo->width,
                         mCtx->codecCtxVideo->height,
                         1);
    
    if (fCode < 0) {
        av_free(sdlCtx);
        av_frame_free(&pFrameYUV);
        return NULL;
    }
    sdlCtx->frame = pFrameYUV;
    
    
    return sdlCtx;
}


SDL_Window *jdc_sdl_create_window(int width , int height)
{
    SDL_Window *window = NULL;
    
    window = SDL_CreateWindow("video",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              width,
                              height,
                              SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL |SDL_WINDOW_SHOWN);
    
    
    return window;
}

int jdc_sdl_present_frame(JDCSDLContext *sdlCtx , AVFrame *pFrame){
    
    
    AVFrame *pFrameYUV = sdlCtx->frame;
  
    
    struct SwsContext *swsCtx = NULL;
    
    sws_scale(swsCtx,
              (uint8_t  const * const *)pFrame->data,
              pFrame->linesize,
              0,
              pFrame->height,
              pFrameYUV->data,
              pFrameYUV->linesize);
    
    SDL_Rect sdlRect;
    sdlRect.x = 0;
    sdlRect.y = 0;
    sdlRect.w = pFrame->width;
    sdlRect.h = pFrame->height;
    
    SDL_UpdateYUVTexture(sdlCtx->texture, &sdlRect,
                         pFrameYUV->data[0], pFrameYUV->linesize[0],
                         pFrameYUV->data[1], pFrameYUV->linesize[1],
                         pFrameYUV->data[2], pFrameYUV->linesize[2]);
    
    SDL_RenderClear( sdlCtx->renderer );
    SDL_RenderCopy( sdlCtx->renderer, sdlCtx->texture,NULL, &sdlRect );
    SDL_RenderPresent( sdlCtx->renderer );
    
    SDL_Delay(1000);
    
    
    return 0;
}

int jdc_sdl_play_audio(struct JDCMediaContext *mCtx , JDCSDLContext *sdlCtx)
{
    
    SDL_AudioSpec wantedSpec;
    SDL_AudioSpec obtainedSpec;
    
    wantedSpec.freq = mCtx->audioStream->codecpar->sample_rate;
    wantedSpec.format = AUDIO_S16;
    wantedSpec.channels = mCtx->codecCtxAudio->channels;
    wantedSpec.silence = 0;
    wantedSpec.samples = SDL_AUDIO_BUFFER_SIZE;
    wantedSpec.callback = jdc_sdl_audio_callback;
    wantedSpec.userdata = mCtx;
    
    if(SDL_OpenAudio(&wantedSpec, &obtainedSpec) < 0) {
        av_free(sdlCtx);
        fprintf(stderr, "SDL_OpenAudio: %s\n", SDL_GetError());
        return -1;
    }
    SDL_PauseAudio(0);
    
    return 0;
}
