//
//  CDemux.cpp
//  iTransCode
//
//  Created by LYH on 2018/8/8.
//  Copyright © 2018年 LYH. All rights reserved.
//

#include "CDemux.hpp"


CDemux::CDemux(void (*c_sendInfo)(int what, string info),const char* i_Filename)
:c_sendInfo(c_sendInfo)
{
//    throw CMyException("123");
    _isFail= false;
    av_register_all();
    i_fmt_ctx= nullptr;

    i_video_st = nullptr;
    i_audio_st = nullptr;;
    video_stream_idx =-1;
    audio_stream_idx =-1;
    audio_codec = nullptr;
    video_codec = nullptr;


    LOGE("123");
    int nRet = avformat_open_input(&i_fmt_ctx, i_Filename,NULL, NULL);
    LOGD("##input nRet:%d\n", nRet);
    if (nRet != 0)
    {
//        printf(szError);
        av_strerror(nRet, szError, 256);
        printf("\n");
        printf("Call avformat_open_input function failed!\n");
        LOGE("open input %s:\n", szError);
        if(this->c_sendInfo!= nullptr){
            this->c_sendInfo(2,szError);//调试去看的话不懂为啥strTemp传过去非法的
        }
        _isFail=true;
        return ;
    }
    if (avformat_find_stream_info(i_fmt_ctx,NULL) < 0)
    {
        printf("Call av_find_stream_info function failed!\n");
        LOGE("Call av_find_stream_info function failed!\n");
        if(this->c_sendInfo!= nullptr){
            this->c_sendInfo(2,szError);//调试去看的话不懂为啥strTemp传过去非法的
        }
        _isFail=true;
        return ;
    }
    //输出视频信息
    av_dump_format(i_fmt_ctx, -1, i_Filename, 0);
    
    //添加音频信息到输出context
    for (int i = 0; i < i_fmt_ctx->nb_streams; i++){
        
        if (i_fmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
            video_stream_idx = i;
            i_video_st=i_fmt_ctx->streams[i] ;
            
        }else if (i_fmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO){
            audio_stream_idx = i;
           i_audio_st=i_fmt_ctx->streams[i];
        }
        
    }
    
}


int CDemux::openDecode(int stream_idx) {
    AVCodec *pcodec = NULL;
    AVCodecContext *CodecCtx = NULL;
    

    CodecCtx = i_fmt_ctx->streams[stream_idx]->codec;
    pcodec = avcodec_find_decoder(CodecCtx->codec_id);
    if (!pcodec)
    {
        return -1;
    }
    

    //打开解码器
    int nRet = avcodec_open2(CodecCtx, pcodec, NULL);
    if (nRet < 0)
    {
        printf("Could not open decoder\n");
        LOGE("Could not open decoder\n");
        if(this->c_sendInfo!= nullptr){
            this->c_sendInfo(2,"Could not open decoder\n");//调试去看的话不懂为啥strTemp传过去非法的
        }
        _isFail=true;
        return -1;
    }
    return 1;
}

int CDemux:: closeDecode(int stream_idx)
{
    AVCodecContext *CodecCtx = NULL;
    
    if (stream_idx == audio_stream_idx)
    {
        CodecCtx = i_audio_st->codec;
    }
    else if (stream_idx == video_stream_idx)
    {
        CodecCtx = i_video_st->codec;
    }
    avcodec_close(CodecCtx);
    return 1;
}


int CDemux::decode(AVMediaType type,AVFrame * frame,AVPacket &packet)
{
    AVCodecContext *CodeCtx = NULL;
    int frameFinished = 0 ;
    
    if (type == AVMEDIA_TYPE_AUDIO)
    {
        CodeCtx = i_fmt_ctx->streams[audio_stream_idx]->codec;
        avcodec_decode_audio4(CodeCtx,frame,&frameFinished,&packet);
        if(frameFinished)
        {
            return 0;
        }
    }
    else if (type == AVMEDIA_TYPE_VIDEO)
    {
        CodeCtx = i_fmt_ctx->streams[video_stream_idx]->codec;
        avcodec_decode_video2(CodeCtx,frame,&frameFinished,&packet);
        if(frameFinished)
        {
            return 0;
        }
    }
    return 1;
}
