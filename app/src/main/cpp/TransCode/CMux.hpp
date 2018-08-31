//
//  CMux.hpp
//  iTransCode
//
//  Created by LYH on 2018/8/12.
//  Copyright © 2018年 LYH. All rights reserved.
//

#ifndef CMux_hpp
#define CMux_hpp

#include "LogHelper.h"

#include "CDemux.hpp"
#include "CSemNamed.hpp"
#include <stdio.h>
#include <string>
#include <iostream>
using namespace std;

//#define STREAM_TB

extern "C"
{
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavutil/avutil.h"
#include "libavutil/mathematics.h"
#include "libswresample/swresample.h"
#include "libavutil/opt.h"
#include "libavutil/channel_layout.h"
#include "libavutil/samplefmt.h"
#include "libavfilter/avfilter.h"
#include "libavutil/error.h"
#include "libavutil/mathematics.h"
#include "libavutil/time.h"
#include "libavutil/fifo.h"
#include "libavutil/audio_fifo.h"
#include "inttypes.h"
#include "stdint.h"

};



class CMux{
public:
    CMux(void (*c_sendInfo)(int what, string info),CDemux *Demux,const char* o_Filename,
         //video param
         int des_Width = 480,
         int des_Height = 480,
         int des_ChannelCount = 2,      //声道
         int des_Frequency = 48000,     //采样率

         double des_FrameRate = 23,  //帧率
         AVCodecID des_Video_codecID = AV_CODEC_ID_H264,
         AVPixelFormat des_Video_pixelfromat = AV_PIX_FMT_YUV420P,
         int des_bit_rate =3210886,// 3210886,//261371,
         int des_gop_size = 12,
         int des_max_b_frame = 2,
         int des_thread_count = 2,

            //audio param
         uint64_t  des_Layout=0,

         AVCodecID des_Audio_codecID = AV_CODEC_ID_AAC,//AV_CODEC_ID_AC3
         AVSampleFormat des_BitsPerSample=AV_SAMPLE_FMT_FLTP//AV_SAMPLE_FMT_S16P
    );
    ~CMux();
    bool isFail(){ return this->_isFail;}
    int audio_support(enum AVCodecID codecId,int *channel,uint64_t * layout,int *samplePerSec,AVSampleFormat * sample_fmt);
    int video_support(enum AVCodecID codecId,AVPixelFormat * video_pixelfromat);

    AVStream *add_out_stream(AVFormatContext* i_fmt_ctx,AVMediaType codec_type_t,AVCodec **codec);
    int openCode(int stream_idx);
    int closeCode(int stream_idx);
    void write_frame(AVMediaType type,AVPacket &pkt);
    int encode(AVMediaType type,AVFrame * frame,AVPacket *outPkt);
    int code_and_write(AVMediaType type,AVFrame * frame);



    AVFormatContext* get_o_fmt_ctx(){return o_fmt_ctx;}


    int get_o_video_stream_idx(){return o_video_stream_idx;}
    int get_o_audio_stream_idx(){return o_audio_stream_idx;}
    AVStream * get_o_video_st(){return o_video_st;}
    AVStream * get_o_audio_st(){return o_audio_st;}

    //video
    AVPixelFormat get_des_Video_pixelfromat(){return des_Video_pixelfromat;}
    int get_des_Width(){return  des_Width;}
    int get_des_Height(){return des_Height;}
    int get_des_bit_rate(){return des_bit_rate;}
    double get_des_FrameRate(){return  des_FrameRate; }
    AVCodecID get_des_Video_codecID(){return des_Video_codecID;}

    //audio
    int get_des_ChannelCount(){return des_ChannelCount;}
    int get_des_Frequency(){return des_Frequency;}
    AVSampleFormat get_des_BitsPerSample(){return des_BitsPerSample;}
    AVCodecID get_des_Audio_codecID(){return des_Audio_codecID;}
    AVAudioFifo *& get_audiofifo(){return audiofifo;}
    AVCodecContext *get_o_video_codec_ctx(){ return o_video_codec_ctx;}
    AVCodecContext *get_o_audio_codec_ctx(){ return o_audio_codec_ctx;}
    
    bool get_video_directWrite(){return video_directWrite;}
    bool get_audio_directWrite(){return audio_directWrite;}
    
    CSemNamed *get_audioSemEmpty(){ return audioSemEmpty;}
    CSemNamed * get_audioSemFull(){return audioSemFull;}
    CSemNamed * get_audioMtx(){return audioMtx;}

   

private:
    bool _isFail;;

    void (*c_sendInfo)(int what, string info);
    char szError[256];
    CDemux *Demux;
    const char *o_Filename;


    AVFormatContext* o_fmt_ctx ;
    AVStream * o_video_st ;
    AVStream * o_audio_st ;
    AVCodecContext *o_video_codec_ctx;
    AVCodecContext *o_audio_codec_ctx;
    int o_video_stream_idx ;
    int o_audio_stream_idx ;


    AVFormatContext* i_fmt_ctx;
    AVCodecContext *i_video_codec_ctx;
    AVCodecContext *i_audio_codec_ctx;
    int i_video_stream_idx;
    int i_audio_stream_idx;



    AVCodec *audio_codec ;
    AVCodec *video_codec ;

    

    //video param
    int des_Width ;
    int des_Height ;
    double des_FrameRate ;  //帧率
    AVCodecID des_Video_codecID ;
    AVPixelFormat des_Video_pixelfromat ;
    //int bit_rate = 400000;
    int des_bit_rate ;
    int des_gop_size ;
    int des_max_b_frame ;
    int des_thread_count ;



    //audio param
    uint64_t  des_Layout;
    int des_ChannelCount ;      //声道
    int des_Frequency ;     //采样率
    //    44100
    //AVSampleFormat_t m_dwBitsPerSample = AV_SAMPLE_FMT_S16_t;    //样本
    AVSampleFormat des_BitsPerSample ;


    //aac
    AVCodecID des_Audio_codecID ;//AV_CODEC_ID_AC3
    //AVCodecID audio_codecID = AV_CODEC_ID_FIRST_AUDIO;






    AVAudioFifo * audiofifo ;

    
    bool video_directWrite=false;
    bool audio_directWrite=false;
    
//    mutex audioMtx;                 //互斥量
//    condition_variable audioCondVar;//条件变量
    CSemNamed *audioSemEmpty;
    CSemNamed *audioSemFull;
    CSemNamed *audioMtx;
    
};

#endif /* CMux_hpp */
