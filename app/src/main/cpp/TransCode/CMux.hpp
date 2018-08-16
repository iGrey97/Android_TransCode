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
#include <stdio.h>
#include <iostream>
using namespace std;


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
    CMux(CDemux *Demux,const char* o_Filename,
         //video param
         int des_Width = 480,
         int des_Height = 200,
         double des_FrameRate = 23,  //帧率
         AVCodecID des_Video_codecID = AV_CODEC_ID_H264,
         AVPixelFormat des_Video_pixelfromat = AV_PIX_FMT_YUV420P,
         int des_bit_rate = 261371,
         int des_gop_size = 12,
         int des_max_b_frame = 2,
         int des_thread_count = 2,

         //audio param
         uint64_t  des_Layout=0,
         int des_ChannelCount = 2,      //声道
         int des_Frequency = 44100,     //采样率
         AVCodecID des_Audio_codecID = AV_CODEC_ID_AAC,//AV_CODEC_ID_AC3
         AVSampleFormat des_BitsPerSample=AV_SAMPLE_FMT_S16P
    );
    ~CMux();
    int audio_support(enum AVCodecID codecId,int *channel,uint64_t * layout,int *samplePerSec,AVSampleFormat * sample_fmt);
    int video_support(enum AVCodecID codecId,AVPixelFormat * video_pixelfromat);

    AVStream *add_out_stream(AVFormatContext* i_fmt_ctx,AVMediaType codec_type_t,AVCodec **codec);
    int openCode(int stream_idx);
    int closeCode(int stream_idx);
    void write_frame(AVMediaType type,AVPacket &pkt);

    int code_and_write(AVMediaType type,AVFrame * frame);



    AVFormatContext* get_o_fmt_ctx(){return o_fmt_ctx;}

    int get_i_video_stream_idx(){return i_video_stream_idx;}
    int get_i_audio_stream_idx(){return i_audio_stream_idx;}
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
private:
    char szError[256];
    CDemux *Demux;
    const char *o_Filename;


    AVFormatContext* o_fmt_ctx ;
    int o_video_stream_idx ;
    int o_audio_stream_idx ;

    AVFormatContext* i_fmt_ctx;
    int i_video_stream_idx;
    int i_audio_stream_idx;

    AVStream * o_video_st ;
    AVStream * o_audio_st ;

    AVCodec *audio_codec ;
    AVCodec *video_codec ;

    AVBitStreamFilterContext * vbsf_aac_adtstoasc ;


    AVBSFContext *audio_bsf_ctx;
    const AVBitStreamFilter *audio_filter;

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



    //mp3
    //AVCodecID audio_codecID = AV_CODEC_ID_MP3;
    //int audio_frame_size  = 1152;


   AVAudioFifo * audiofifo ;

    
};

#endif /* CMux_hpp */
