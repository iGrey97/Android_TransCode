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
    i_audio_codec = nullptr;
    i_video_codec = nullptr;
    i_video_codec_ctx=nullptr;
    i_audio_codec_ctx=nullptr;


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
    
    AVCodec *pcodec = nullptr;
    AVCodecContext *CodecCtx = nullptr;
    AVCodecParameters *codecpar=nullptr;
    //添加音频信息到输出context
    for (int i = 0; i < i_fmt_ctx->nb_streams; i++){
        
        if (i_fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
            video_stream_idx = i;
            i_video_st=i_fmt_ctx->streams[i] ;

            //获取输入codecCtx
            codecpar=i_fmt_ctx->streams[i]->codecpar;
            pcodec = avcodec_find_decoder(codecpar->codec_id);
            if (!pcodec)
            {
                printf("Cannot find_decoder!\n");
                LOGE("Cannot find_decoder!\n");
                if(this->c_sendInfo!= nullptr){
                    this->c_sendInfo(2,"Cannot find_decoder!\n");//调试去看的话不懂为啥strTemp传过去非法的
                }
                _isFail=true;
                return ;
            }
            
            /* Allocate a new decoding context. */
            CodecCtx = avcodec_alloc_context3(pcodec);
            if (!CodecCtx ) {
                fprintf(stderr, "Could not allocate a video decoding context\n");
                LOGE("Could not allocate a video decoding context\n");
                if(this->c_sendInfo!= nullptr){
                    this->c_sendInfo(2,"Cannot find_decoder!\n");//调试去看的话不懂为啥strTemp传过去非法的
                }
                _isFail=true;
                return ;
            }
             CodecCtx->framerate = av_guess_frame_rate(i_fmt_ctx, i_fmt_ctx->streams[i], NULL);
            /* Initialize the stream parameters with demuxer information. */
            int error=avcodec_parameters_to_context( CodecCtx, i_fmt_ctx->streams[i]->codecpar);
            if (error < 0) {
                LOGE("Could not parameters_to_context\n");
                if(this->c_sendInfo!= nullptr){
                    this->c_sendInfo(2,"Cannot parameters_to_context!\n");//调试去看的话不懂为啥strTemp传过去非法的
                }
                _isFail=true;
                return ;
            }
            i_video_codec=pcodec;
            i_video_codec_ctx=CodecCtx;
            /**********获取输入codecCtx_end****************/
            
            
        }else if (i_fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){
            audio_stream_idx = i;
           i_audio_st=i_fmt_ctx->streams[i];
            
            //获取输入codecCtx
            codecpar=i_fmt_ctx->streams[i]->codecpar;
            pcodec = avcodec_find_decoder(codecpar->codec_id);
            if (!pcodec)
            {
                printf("Cannot find_decoder!\n");
                LOGE("Cannot find_decoder!\n");
                if(this->c_sendInfo!= nullptr){
                    this->c_sendInfo(2,"Cannot find_decoder!\n");//调试去看的话不懂为啥strTemp传过去非法的
                }
                _isFail=true;
                return ;
            }
            
            /* Allocate a new decoding context. */
            CodecCtx = avcodec_alloc_context3(pcodec);
            if (!CodecCtx ) {
                fprintf(stderr, "Could not allocate a audio decoding context\n");
                LOGE("Could not allocate a video decoding context\n");
                if(this->c_sendInfo!= nullptr){
                    this->c_sendInfo(2,"Cannot find_decoder!\n");//调试去看的话不懂为啥strTemp传过去非法的
                }
                _isFail=true;
                return ;
            }
            
            /* Initialize the stream parameters with demuxer information. */
            int error=avcodec_parameters_to_context( CodecCtx, i_fmt_ctx->streams[i]->codecpar);
            if (error < 0) {
                LOGE("Could not parameters_to_context\n");
                if(this->c_sendInfo!= nullptr){
                    this->c_sendInfo(2,"Cannot parameters_to_context!\n");//调试去看的话不懂为啥strTemp传过去非法的
                }
                _isFail=true;
                return ;
            }
            i_audio_codec=pcodec;
            i_audio_codec_ctx=CodecCtx;
            /**********获取输入codecCtx_end****************/
        }
        
    }
  

    
}

CDemux::~CDemux(){
     avformat_close_input(&i_fmt_ctx);
     avcodec_free_context(&i_video_codec_ctx);
     avcodec_free_context(&i_audio_codec_ctx);
}

int CDemux::openDecode(int stream_idx) {
    AVCodec *pcodec = nullptr;
    AVCodecContext *CodecCtx = nullptr;

    
    
    if(stream_idx==audio_stream_idx){
        CodecCtx=i_audio_codec_ctx;
        pcodec=i_audio_codec;
    }else{
        CodecCtx=i_video_codec_ctx;
        pcodec=i_video_codec;
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
        CodecCtx = i_audio_codec_ctx;
    }
    else if (stream_idx == video_stream_idx)
    {
        CodecCtx = i_video_codec_ctx;
    }
    avcodec_close(CodecCtx);
    return 1;
}


int CDemux::decode(AVMediaType type,AVFrame * frame,AVPacket &packet)
{
    int error=0;
    AVCodecContext *CodeCtx = NULL;
    if (type == AVMEDIA_TYPE_AUDIO)
    {
        CodeCtx = i_audio_codec_ctx;
    }
    else if (type == AVMEDIA_TYPE_VIDEO)
    {
        CodeCtx = i_video_codec_ctx;
    }
    error=avcodec_send_packet(CodeCtx, &packet);
    if (error < 0) {
        fprintf(stderr, "Could not send packet for decoding (error '%s')\n",
                av_err2str(error));
        return error;
    }
    /* Receive one frame from the decoder. */
    error = avcodec_receive_frame(CodeCtx, frame);
    /* If the decoder asks for more data to be able to decode a frame,
     * return indicating that no data is present. */
    if (error == AVERROR(EAGAIN)) {
      //AVERROR(EAGAIN):   output is not available in this state - user must try to send new input
   
        /* If the end of the input file is reached, stop decoding. */
    } else if (error == AVERROR_EOF) {
        //有的输入流最后没有一帧data=null
        //AVERROR_EOF:       the decoder has been fully flushed, and there will be no more output frames

    } else if (error < 0) {
        fprintf(stderr, "Could not decode frame (error '%s')\n",av_err2str(error));
        
        /* Default case: Return decoded data. */
    } else {
        //error就等于0
        //解码成功
//        enum AVPixelFormat;
        
   
        frame->pts=frame->best_effort_timestamp;//mac上pts自动设置了，android没有
    }
    return error;
   
}
