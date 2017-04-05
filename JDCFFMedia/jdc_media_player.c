//
//  jdc_media_player.c
//  JDCFFPlayer
//
//  Created by maochengrui on 01/04/2017.
//  Copyright © 2017 jidong. All rights reserved.
//

#include "jdc_media_player.h"

#define FF_QUIT_EVENT 10086
#define FF_REFRESH_EVENT 10087

int jdc_media_init()
{
    av_register_all();
    return 0;
}

int jdc_media_video_thread(void *data)
{
    JDCMediaContext *mCtx = data;
    AVPacket *packet;
    AVFrame *pFrame = av_frame_alloc();
    mCtx->videoFrameQueue = jdc_packet_queue_alloc();
    jdc_packet_queue_init(mCtx->videoFrameQueue);
    
    while(1){
        
        if(jdc_packet_queue_get_packet(mCtx->videoQueue, (void **)&packet, 1) < 0) {
            break;
        }
        
         int r = avcodec_send_packet(mCtx->codecCtxVideo, packet);
         if (r != 0) {
             av_packet_unref(packet);
             continue;
         }
        
         r = avcodec_receive_frame(mCtx->codecCtxVideo, pFrame);
         if (r != 0) {
             av_packet_unref(packet);
             continue;
         }
        
        jdc_packet_queue_push(mCtx->videoFrameQueue, pFrame);
    }
    
    av_frame_free(&pFrame);
    
    return 0;
}

int jdc_media_open_stream(JDCMediaContext *mCtx , int sIdx){
    
    AVFormatContext *pFormatCtx = mCtx->fmtCtx;
    AVCodecContext *codecCtx = NULL;
    AVCodec *codec = NULL;
    
    if (sIdx < 0 || sIdx >= pFormatCtx->nb_streams) {
        return -1;
    }
    
    AVStream *stream = pFormatCtx->streams[sIdx];
    codec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!codec) {
        fprintf(stderr, "Unsupported codec!\n");
        return -1;
    }
    
    codecCtx = avcodec_alloc_context3(codec);
    if(avcodec_parameters_to_context(codecCtx, stream->codecpar) < 0){
        fprintf(stderr, "avcodec parameters to context failed!\n");
        return -1;
    }
    
    if (codecCtx->codec_type == AVMEDIA_TYPE_AUDIO) {
        SDL_AudioSpec wanted_spec;
        SDL_AudioSpec spec;
        wanted_spec.freq = codecCtx->sample_rate;
        wanted_spec.format = AUDIO_S16SYS;
        wanted_spec.channels = codecCtx->channels;
        wanted_spec.silence = 0;
        wanted_spec.samples = 1024;
        wanted_spec.callback = jdc_sdl_audio_callback;
        wanted_spec.userdata = mCtx;
        
        if(SDL_OpenAudio(&wanted_spec, &spec) < 0) {
            fprintf(stderr, "SDL_OpenAudio: %s\n", SDL_GetError());
            return -1;
        }
    }
    
    if(avcodec_open2(codecCtx, codec, NULL) < 0) {
        fprintf(stderr, "Unsupported codec!\n");
        return -1;
    }
    
    switch(codecCtx->codec_type) {
        case AVMEDIA_TYPE_AUDIO:
            mCtx->audioStreamIdx = sIdx;
            mCtx->audioStream = stream;
            mCtx->codecAudio = codec;
            mCtx->codecCtxAudio = codecCtx;
            mCtx->audioQueue = jdc_packet_queue_alloc();
            jdc_packet_queue_init(mCtx->audioQueue);
            SDL_PauseAudio(0);
            break;
        case AVMEDIA_TYPE_VIDEO:
            mCtx->videoStreamIdx = sIdx;
            mCtx->videoStream = stream;
            mCtx->codecVideo = codec;
            mCtx->codecCtxVideo = codecCtx;
            mCtx->videoQueue = jdc_packet_queue_alloc();
            mCtx->video_tid = SDL_CreateThread(jdc_media_video_thread,
                                               "video thread",
                                               mCtx);
            jdc_packet_queue_init(mCtx->videoQueue);
            mCtx->swsCtx = sws_getContext(mCtx->codecCtxVideo->width,
                                           mCtx->codecCtxVideo->height,
                                           mCtx->codecCtxVideo->pix_fmt,
                                           mCtx->codecCtxVideo->width,
                                           mCtx->codecCtxVideo->height,
                                           AV_PIX_FMT_YUV420P,
                                           SWS_BILINEAR,
                                           NULL,
                                           NULL,
                                           NULL);
            break;
        default:
            break;
    }
    
    return 0;
}

int jdc_media_decode_thread(void *userData)
{
    JDCMediaContext *mCtx = userData;
    
    
    AVFrame *pFrame = NULL;
    pFrame = av_frame_alloc();
    
    if (pFrame == NULL) {
        return -1;
    }
    
    int numBytes;
    numBytes = av_image_get_buffer_size(AV_PIX_FMT_YUV420P,
                                        mCtx->codecCtxVideo->width,
                                        mCtx->codecCtxVideo->height,
                                        1);
    AVPacket packet;
    av_new_packet(&packet, numBytes);
    
    while(av_read_frame(mCtx->fmtCtx, &packet) >= 0){
        if (packet.stream_index == mCtx->videoStream->index) {
            jdc_packet_queue_push(mCtx->videoQueue, &packet);
        }else if(packet.stream_index == mCtx->audioStream->index){
            jdc_packet_queue_push(mCtx->audioQueue, &packet);
        }
    }
    
    return 0;
}



JDCMediaContext *jdc_media_open_input(const char *url,JDCError **error)
{
    JDCMediaContext *mCtx = (JDCMediaContext *)av_mallocz(sizeof(JDCMediaContext));
    AVFormatContext *pFmtCtx = avformat_alloc_context();
    
    strcpy(mCtx->filename, url);
    
    if (avformat_open_input(&pFmtCtx, url, NULL, NULL) != 0) {
        av_free(mCtx);
        return NULL;
    }
    
    mCtx->fmtCtx = pFmtCtx;
    
    if (avformat_find_stream_info(pFmtCtx, NULL) < 0) {
        av_free(mCtx);
        return NULL;
    }
    
    // Dump information about file onto standard error
    av_dump_format(pFmtCtx, 0, mCtx->filename, 0);

    
    for(int i = 0 ; i < pFmtCtx->nb_streams ; i++){
        if(pFmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO &&
           mCtx->videoStream == NULL){
            mCtx->videoStreamIdx = i;
        }
        
        if(pFmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO &&
           mCtx->audioStream == NULL){
            mCtx->audioStreamIdx = i;
        }
    }
    
    
    return mCtx;
}
static Uint32 sdl_refresh_timer_cb(Uint32 interval, void *opaque) {
    SDL_Event event;
    event.type = FF_REFRESH_EVENT;
    event.user.data1 = opaque;
    SDL_PushEvent(&event);
    return 0; /* 0 means stop timer */
}

int schedule_refresh(JDCMediaContext *mCtx, int n)
{
    return SDL_AddTimer(n, sdl_refresh_timer_cb, mCtx);
}

int video_display(JDCMediaContext *mCtx , void *data) {
    
    AVFrame *pFrameYUV = av_frame_alloc();
    AVFrame *pFrame = data;
    JDCSDLContext *sdlCtx = mCtx->sdlCtx;
    
    struct SwsContext *swsCtx = mCtx->swsCtx;
    
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
    
    SDL_RenderClear(sdlCtx->renderer );
    SDL_RenderCopy( sdlCtx->renderer, sdlCtx->texture,NULL, &sdlRect );
    SDL_RenderPresent( sdlCtx->renderer );
    
    
    
    return 0;
}

void video_refresh_timer(void *userdata) {
    
    JDCMediaContext *mCtx = (JDCMediaContext *)userdata;
    
    if(mCtx->videoStream) {
        if(jdc_packet_queue_size(mCtx->videoFrameQueue) == 0) {
            schedule_refresh(mCtx, 1);
        } else {
            /* Now, normally here goes a ton of code
             about timing, etc. we're just going to
             guess at a delay for now. You can
             increase and decrease this value and hard code
             the timing - but I don't suggest that ;)
             We'll learn how to do it for real later.
             */
            schedule_refresh(mCtx, 40);
            
            /* show the picture! */
            void *videoFrameData = NULL;
            jdc_packet_queue_get_packet(mCtx->videoFrameQueue, &videoFrameData, 1);
            video_display(mCtx,videoFrameData);
        }
    } else {
        schedule_refresh(mCtx, 100);
    }
}

int jdc_media_play(JDCMediaContext *mCtx)
{
    jdc_media_open_stream(mCtx, mCtx->audioStreamIdx);
    jdc_media_open_stream(mCtx, mCtx->videoStreamIdx);
    mCtx->sdlCtx = jdc_sdl_create_context(mCtx);
    
    SDL_Event event;
    schedule_refresh(mCtx, 40);
    
    mCtx->parse_tid = SDL_CreateThread(jdc_media_decode_thread, "decode thread",mCtx);
    
    
    if(!mCtx->parse_tid) {
        return -1;
    }
    
    
    while(1){
        
        SDL_WaitEvent(&event);
        switch(event.type) {
            case FF_QUIT_EVENT:
            case SDL_QUIT:
                mCtx->quit = 1;
                SDL_Quit();
                return 0;
                break;
            case FF_REFRESH_EVENT:
                video_refresh_timer(event.user.data1);
                break;
            default:
                break;
        }
    }
    return 0;
}
