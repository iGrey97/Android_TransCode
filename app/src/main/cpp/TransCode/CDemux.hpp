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
    CDemux(const char* i_Filename);
    int openDecode(int stream_idx);
    int closeDecode(int stream_idx);
    int decode(AVMediaType type,AVFrame * frame,AVPacket &packet);
   
    AVFormatContext *get_i_fmt_ctx(){return i_fmt_ctx;}
    int get_video_stream_idx(){return video_stream_idx;}
    int get_audio_stream_idx(){return audio_stream_idx;}
    
private:
    

    AVFormatContext* i_fmt_ctx;
    AVStream * i_video_st ;
    AVStream * i_audio_st ;
    int video_stream_idx ;
    int audio_stream_idx ;
    AVCodec *audio_codec ;
    AVCodec *video_codec ;
    char szError[256];
   
};


#endif /* CDemux_hpp */