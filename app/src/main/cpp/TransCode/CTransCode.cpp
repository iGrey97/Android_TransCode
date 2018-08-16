//
//  CTransCode.cpp
//  iTransCode
//
//  Created by LYH on 2018/8/13.
//  Copyright © 2018年 LYH. All rights reserved.
//

#include "CTransCode.hpp"




CTransCode::CTransCode(CDemux *Demux,CMux *Mux,int sws_flags)
:Demux(Demux),Mux(Mux),sws_flags(sws_flags){
    pinframe = NULL;
    pout_video_frame = NULL;
    pout_audio_frame = NULL;
    swr_ctx = NULL;
    img_convert_ctx_video = NULL;
    
    o_video_st=Mux->get_o_video_st();
    o_audio_st=Mux->get_o_audio_st();
    
    i_video_stream_idx=Demux->get_video_stream_idx();
    i_audio_stream_idx=Demux->get_audio_stream_idx();
    i_fmt_ctx=Demux->get_i_fmt_ctx();
    o_fmt_ctx=Mux->get_o_fmt_ctx();
    o_video_stream_idx=Mux->get_o_video_stream_idx();
    o_audio_stream_idx=Mux->get_o_audio_stream_idx();

    des_Width=Mux->get_des_Width();
    des_Height=Mux->get_des_Height();
    des_bit_rate=Mux->get_des_bit_rate();
    des_frameRate=Mux->get_des_FrameRate();
    video_codecID=Mux->get_des_Video_codecID();
    des_Video_pixelfroma=Mux->get_des_Video_pixelfromat();

    des_BitsPerSample = Mux->get_des_BitsPerSample();
    des_Frequency =Mux->get_des_Frequency();
    des_channelCount =Mux->get_des_ChannelCount();
    audio_codecID=Mux->get_des_Audio_codecID();
    
//    int dst_nb_samples = -1;
//    int resampled_data_size = 0;
    //分配一个AVFrame并设置默认值
    pinframe = av_frame_alloc();
    if (pinframe == NULL)
    {
        printf("avcodec_alloc_frame pinframe error\n");
        return ;
    }
    pinframe->pts = 0;
    //video
    pout_video_frame = av_frame_alloc();
    if (pout_video_frame == NULL)
    {
        printf("avcodec_alloc_frame pout_video_frame error\n");
        return ;
    }
    pout_video_frame->pts = 0;
    
    //audio
    pout_audio_frame = av_frame_alloc();
    if (pout_audio_frame == NULL)
    {
        printf("avcodec_alloc_frame pout_audio_frame error\n");
        return ;
    }
    pout_audio_frame->pts = 0;
  
    int Out_size = avpicture_get_size(Mux->get_des_Video_pixelfromat(),Mux->get_des_Width(),Mux->get_des_Height() );
    pOutput_buf =( uint8_t *)malloc(Out_size * 3 * sizeof(char)); //最大分配的空间，能满足yuv的各种格式
    
    avpicture_fill((AVPicture *)pout_video_frame, (unsigned char *)pOutput_buf, Mux->get_des_Video_pixelfromat(),Mux->get_des_Width(),Mux->get_des_Height()); //内存关联、会给pout_video_frame->data分配空间
    
   
    av_frame_get_buffer(pout_audio_frame, 0);
}

int CTransCode::Trans() { 
    //开始解包
    int nRet=0;
    AVPacket pkt;
    while (1)
    {
        av_init_packet(&pkt);
        if (av_read_frame(i_fmt_ctx, &pkt) < 0)
        {
            break;
        }
        //视频
        if(pkt.stream_index == i_video_stream_idx )
        {
            //如果是视频需要编解码
            if(des_bit_rate != i_fmt_ctx->streams[i_video_stream_idx]->codec->bit_rate ||
               des_Width  != i_fmt_ctx->streams[i_video_stream_idx]->codec->width ||
               des_Height != i_fmt_ctx->streams[i_video_stream_idx]->codec->height ||
               video_codecID != i_fmt_ctx->streams[i_video_stream_idx]->codec->codec_id ||
               des_frameRate != av_q2d(i_fmt_ctx->streams[i_video_stream_idx]->r_frame_rate))
            {
                nRet = Demux->decode(AVMEDIA_TYPE_VIDEO, pinframe, pkt);
                if (nRet == 0)
                {
                    yuv_conversion(pinframe,pout_video_frame);
                    pout_video_frame->pts = pinframe->best_effort_timestamp;

                    nRet = Mux->code_and_write(AVMEDIA_TYPE_VIDEO, pout_video_frame);

                }
            }
            else
            {
                pkt.pos = -1;
                pkt.stream_index=o_video_stream_idx;//这样设置一下就可以直接写了，不用再设置pts、dts
                nRet = av_interleaved_write_frame(o_fmt_ctx, &pkt);
                cout<<"video"<<endl;
             
            }
        }
        //音频
        else if (pkt.stream_index == i_audio_stream_idx)
        {
            
            //如果是音频需要编解码
            if(audio_codecID != i_fmt_ctx->streams[i_audio_stream_idx]->codec->codec_id  ||
               des_BitsPerSample != i_fmt_ctx->streams[i_audio_stream_idx]->codec->sample_fmt||
               des_Frequency !=i_fmt_ctx->streams[i_audio_stream_idx]->codec->sample_rate||
               des_channelCount != i_fmt_ctx->streams[i_audio_stream_idx]->codec->channels)
            {
                nRet = Demux->decode(AVMEDIA_TYPE_AUDIO, pinframe, pkt);
                if (nRet == 0)
                {
                    //如果进和出的的声道，样本，采样率不同,需要重采样
                    if(i_fmt_ctx->streams[i_audio_stream_idx]->codec->sample_fmt != des_BitsPerSample ||
                       i_fmt_ctx->streams[i_audio_stream_idx]->codec->channels != des_channelCount ||
                       i_fmt_ctx->streams[i_audio_stream_idx]->codec->sample_rate != des_Frequency)
                    {
                        //                        av_frame_get_buffer(pout_audio_frame, 0);、不用获取了，虽然av_freep释放了，但是av_samples_alloc又会获得
                        if (swr_ctx == NULL)
                        {
                            swr_ctx = init_pcm_resample(pinframe,pout_audio_frame);
                        }
                        pcm_resample(swr_ctx,pinframe,pout_audio_frame);

                        
                        Mux->code_and_write(AVMEDIA_TYPE_AUDIO, pout_audio_frame);
                        av_freep(&(pout_audio_frame->data[0]));
      }
                    else
                    {
                        pinframe->pts = pinframe->pkt_pts;
                        //                        ffmpeg在解码的时候将解码出来的顺序时间戳给了frame的pkt_pts这个成员，所以我们可以直接用这个值赋值给frame的pts，在送进编码器，这样编码出来的pkt中的时间戳就和原来的audio对上了。

                        Mux->code_and_write(AVMEDIA_TYPE_AUDIO, pout_audio_frame);


                    }
                }
            }
            else
            {
                pkt.pts = av_rescale_q_rnd(pkt.pts, i_fmt_ctx->streams[i_audio_stream_idx]->time_base, o_audio_st->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
                pkt.dts = av_rescale_q_rnd(pkt.dts, i_fmt_ctx->streams[i_audio_stream_idx]->time_base, o_audio_st->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
                pkt.duration = av_rescale_q(pkt.duration, i_fmt_ctx->streams[i_audio_stream_idx]->time_base, o_audio_st->time_base);
                pkt.pos = -1;
                pkt.stream_index=o_audio_stream_idx;
                nRet = av_interleaved_write_frame(o_fmt_ctx, &pkt);
                cout<<"audio"<<endl;
                //                write_frame(ocodec,AUDIO_ID,pkt);
            }
        }
        
        av_packet_unref(&pkt);
    }
    
    
    
    
    
    //清缓存
    /* flush  encoders */
    for (int i = 0; i < i_fmt_ctx->nb_streams; i++) {
        
        /* flush encoder */
        flush_encoder(i);
        
    }
    
    if (pinframe)
    {
        av_frame_free(&pinframe);
        pinframe = NULL;
    }
    if (pout_video_frame)
    {
        av_frame_free(&pout_video_frame);
        pout_video_frame = NULL;
    }
    if (pout_audio_frame)
    {
        av_frame_free(&pout_audio_frame);
        pout_audio_frame = NULL;
    }
    if (swr_ctx)
    {
        swr_free(&swr_ctx);
        swr_ctx = NULL;
    }
    return 1;
}


void CTransCode::yuv_conversion(AVFrame * pinframe,AVFrame * poutframe)
{
    //设置转换context
    
    if (img_convert_ctx_video == NULL)
    {
        img_convert_ctx_video = sws_getContext(i_fmt_ctx->streams[i_video_stream_idx]->codec->width, i_fmt_ctx->streams[i_video_stream_idx]->codec->height,
                                               i_fmt_ctx->streams[i_video_stream_idx]->codec->pix_fmt,
                                               des_Width, des_Height,
                                               des_Video_pixelfroma,
                                               sws_flags, NULL, NULL, NULL);
        if (img_convert_ctx_video == NULL)
        {
            printf("Cannot initialize the conversion context\n");
        }
    }
    //开始转换
    sws_scale(img_convert_ctx_video, pinframe->data, pinframe->linesize,
              0, i_fmt_ctx->streams[i_video_stream_idx]->codec->height, poutframe->data, poutframe->linesize);
    poutframe->pkt_pts = pinframe->pkt_pts;
    poutframe->pkt_dts = pinframe->pkt_dts;
    //有时pkt_pts和pkt_dts不同，并且pkt_pts是编码前的dts,这里要给avframe传入pkt_dts而不能用pkt_pts
    //poutframe->pts = poutframe->pkt_pts;
    poutframe->pts = pinframe->pkt_dts;
    
    
    
    poutframe->width=des_Width;
    poutframe->height=des_Height;
    poutframe->format=des_Video_pixelfroma;
}


SwrContext * CTransCode:: init_pcm_resample(AVFrame *in_frame, AVFrame *out_frame)
{
    SwrContext * swr_ctx = NULL;
    swr_ctx = swr_alloc();
    if (!swr_ctx)
    {
        printf("swr_alloc error \n");
        return NULL;
    }
    AVCodecContext * audio_dec_ctx = i_fmt_ctx->streams[i_audio_stream_idx]->codec;
    
    if (audio_dec_ctx->channel_layout == 0)//不懂干嘛,把源数据的改了
    {
        audio_dec_ctx->channel_layout = av_get_default_channel_layout(i_fmt_ctx->streams[i_audio_stream_idx]->codec->channels);
    }
    /* set options */
    av_opt_set_int(swr_ctx, "in_channel_layout",    audio_dec_ctx->channel_layout, 0);
    av_opt_set_int(swr_ctx, "in_sample_rate",       audio_dec_ctx->sample_rate, 0);
    av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", audio_dec_ctx->sample_fmt, 0);
    if(i_fmt_ctx->streams[i_audio_stream_idx]->codec->channels > 2)
    {
        av_opt_set_int(swr_ctx, "out_channel_layout",    av_get_default_channel_layout(des_channelCount), 0);
    }
    else
    {
        av_opt_set_int(swr_ctx, "out_channel_layout", audio_dec_ctx->channel_layout, 0);
    }
    av_opt_set_int(swr_ctx, "out_sample_rate",       des_Frequency, 0);//这应该是目的的把sample——rate
    av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", des_BitsPerSample, 0);
    swr_init(swr_ctx);
    
    
    
    int64_t src_nb_samples = in_frame->nb_samples;//
    out_frame->nb_samples = av_rescale_rnd(swr_get_delay(swr_ctx,o_audio_st->codec->sample_rate) + src_nb_samples,
                                           o_audio_st->codec->sample_rate, o_audio_st->codec->sample_rate, AV_ROUND_UP);
    //一个输出一个输入
    int ret = av_samples_alloc(out_frame->data, &out_frame->linesize[0],
                               i_fmt_ctx->streams[i_audio_stream_idx]->codec->channels, out_frame->nb_samples,o_audio_st->codec->sample_fmt,1);
    if (ret < 0)
    {
        return NULL;
    }
    
    
    Mux->get_audiofifo()= av_audio_fifo_alloc(o_audio_st->codec->sample_fmt, o_audio_st->codec->channels,
                                              out_frame->nb_samples);
    
    return swr_ctx;
}



int CTransCode:: pcm_resample(SwrContext * pSwrCtx,AVFrame *in_frame, AVFrame *out_frame)
{
    if (pSwrCtx != NULL)
    {
        
        /*************************每次从新getdelay版本**********************/
        
        int64_t src_nb_samples = in_frame->nb_samples;//
        cout<<swr_get_delay(pSwrCtx,o_audio_st->codec->sample_rate)<<endl;
        
        
        //重新开空间
        out_frame->nb_samples = av_rescale_rnd(swr_get_delay(pSwrCtx,i_fmt_ctx->streams[i_audio_stream_idx]->codec->sample_rate) + src_nb_samples,
                                               o_audio_st->codec->sample_rate, i_fmt_ctx->streams[i_audio_stream_idx]->codec->sample_rate ,AV_ROUND_UP);
        
        
        int ret = av_samples_alloc(out_frame->data, &out_frame->linesize[0],
                                   i_fmt_ctx->streams[i_audio_stream_idx]->codec->channels, out_frame->nb_samples,o_audio_st->codec->sample_fmt,1);
        
        
        
        out_frame->nb_samples = swr_convert(pSwrCtx, out_frame->data, out_frame->nb_samples,
                                            (const uint8_t**)in_frame->data, in_frame->nb_samples);
        
        if (ret < 0)
        {
            return -1;
        }
        
        
        
        av_audio_fifo_realloc(Mux->get_audiofifo(), av_audio_fifo_size(Mux->get_audiofifo()) + out_frame->nb_samples);
        av_audio_fifo_write(Mux->get_audiofifo(),(void **)out_frame->data,out_frame->nb_samples);
        
        
        out_frame->pkt_pts = in_frame->pkt_pts;
        out_frame->pkt_dts = in_frame->pkt_dts;
        //有时pkt_pts和pkt_dts不同，并且pkt_pts是编码前的dts,这里要给avframe传入pkt_dts而不能用pkt_pts
        //out_frame->pts = out_frame->pkt_pts;
        out_frame->pts = in_frame->pkt_dts;
    }
    return 0;
}


void CTransCode:: flush_encoder(int stream_index){
    int ret;
    int got_frame;
    AVPacket pkt;
    AVMediaType type=o_fmt_ctx->streams[stream_index]->codec->codec_type;
    int (*enc_fun)(AVCodecContext *, AVPacket *, const AVFrame *, int *)=o_fmt_ctx->streams[stream_index]->codec->codec_type==AVMEDIA_TYPE_AUDIO ? avcodec_encode_audio2 : avcodec_encode_video2;
    if (!(o_fmt_ctx->streams[stream_index]->codec->codec->capabilities &
          AV_CODEC_CAP_DELAY))
    return;
    while (1) {
        pkt.data = NULL;
        pkt.size = 0;
        av_init_packet(&pkt);
        ret = enc_fun (o_fmt_ctx->streams[stream_index]->codec, &pkt,
                       NULL, &got_frame);
        
        if (ret < 0)//失败
        break;
        if (!got_frame){//清空了
            ret=0;
            break;
        }
        printf("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n",pkt.size);
        
        Mux->write_frame(type,pkt);
        
    }
    
}
