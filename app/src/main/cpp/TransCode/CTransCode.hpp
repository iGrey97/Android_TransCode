//
//  CTransCode.hpp
//  iTransCode
//
//  Created by LYH on 2018/8/13.
//  Copyright © 2018年 LYH. All rights reserved.
//

#ifndef CTransCode_hpp
#define CTransCode_hpp

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
//#include "libavdevice/avdevice.h"
#include "libavfilter/avfilter.h"
#include "libavutil/error.h"
#include "libavutil/mathematics.h"
#include "libavutil/time.h"
#include "libavutil/fifo.h"
#include "libavutil/audio_fifo.h"
#include "inttypes.h"
#include "stdint.h"


#include "CMux.hpp"
#include "CDemux.hpp"
class CTransCode{
  
public:
    CTransCode(CDemux *Demux,CMux *Mux, int sws_flags = SWS_BICUBIC);
    ~CTransCode(){}
    int Trans();
    
    void yuv_conversion(AVFrame * pinframe,AVFrame * poutframe);
    
    
    SwrContext * init_pcm_resample(AVFrame *in_frame, AVFrame *out_frame);
    int pcm_resample(SwrContext * pSwrCtx,AVFrame *in_frame, AVFrame *out_frame);
  
    void  flush_encoder(int stream_index);
    
private:
    CDemux *Demux;
    CMux *Mux;
    AVFrame *pinframe ;
    AVFrame * pout_video_frame ;
    AVFrame * pout_audio_frame ;
    SwrContext * swr_ctx ;
    int sws_flags ;
    struct SwsContext * img_convert_ctx_video ;
    int dst_nb_samples ;
    int resampled_data_size ;
   

    uint8_t * pOutput_buf ;
    
    AVStream * o_video_st;
    AVStream * o_audio_st;
    
    int i_video_stream_idx;
    int i_audio_stream_idx;
    AVFormatContext *i_fmt_ctx;
    
    int o_video_stream_idx;
    int o_audio_stream_idx;
    AVFormatContext *o_fmt_ctx;
    
    int des_Width;
    int des_Height;
    int des_bit_rate;
    double des_frameRate;
    AVPixelFormat des_Video_pixelfroma;
    AVCodecID video_codecID;
    
    
    AVSampleFormat des_BitsPerSample ;
    int des_Frequency;
    int des_channelCount;
    AVCodecID audio_codecID;
    
    
};
};
#endif /* CTransCode_hpp */
