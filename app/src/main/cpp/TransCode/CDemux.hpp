//
//  CDemux.hpp
//  iTransCode
//
//  Created by LYH on 2018/8/8.
//  Copyright © 2018年 LYH. All rights reserved.
//

#ifndef CDemux_hpp
#define CDemux_hpp


#include "LogHelper.h"
#include <iostream>
#include <string>
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

class CDemux{
public:
    CDemux(void (*c_sendInfo)(int what, string info),const char* i_Filename);
    ~CDemux();
    int openDecode(int stream_idx);
    int closeDecode(int stream_idx);
    int decode(AVMediaType type,AVFrame * frame,AVPacket &packet);

    bool isFail(){ return this->_isFail;}
    AVFormatContext *get_i_fmt_ctx(){return i_fmt_ctx;}
    int get_video_stream_idx(){return video_stream_idx;}
    int get_audio_stream_idx(){return audio_stream_idx;}
    AVCodecContext *get_i_video_codec_ctx(){ return i_video_codec_ctx;}
    AVCodecContext *get_i_audio_codec_ctx(){ return i_audio_codec_ctx;}
    
    int get_i_Width(){return  i_video_st->codecpar->width;}
    int get_i_Height(){return i_video_st->codecpar->height;}

    //audio
    int get_i_ChannelCount(){return i_audio_st->codecpar->channels;}
    int get_i_Frequency(){return  i_audio_st->codecpar->sample_rate;}


private:
    bool _isFail;
    void (*c_sendInfo)(int what, string info);//传给java信息

    AVFormatContext* i_fmt_ctx;
    AVCodecContext *i_video_codec_ctx;
    AVCodecContext *i_audio_codec_ctx;
    
    AVStream * i_video_st ;
    AVStream * i_audio_st ;
    int video_stream_idx ;
    int audio_stream_idx ;
    AVCodec *i_audio_codec ;
    AVCodec *i_video_codec ;
    char szError[256];
   
};


#endif /* CDemux_hpp */
