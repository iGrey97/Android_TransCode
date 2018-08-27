//
//  CTransCode.cpp
//  iTransCode
//
//  Created by LYH on 2018/8/13.
//  Copyright © 2018年 LYH. All rights reserved.
//

#include "CTransCode.hpp"






CTransCode::CTransCode(void (*c_sendInfo)(int what, string info),CDemux *Demux,CMux *Mux,int sws_flags)
:c_sendInfo(c_sendInfo),Demux(Demux),Mux(Mux),sws_flags(sws_flags){
    _isFail= false;

    
    audioSemEmpty=Mux->get_audioSemEmpty();
    audioSemFull=Mux->get_audioSemFull();
    audioMtx=Mux->get_audioMtx();
    
    
    pinframe = NULL;
    pout_video_frame = NULL;
    pout_audio_frame = NULL;
    swr_ctx = NULL;
    img_convert_ctx_video = NULL;
    
    video_directWrite=Mux->get_video_directWrite();
    audio_directWrite=Mux->get_audio_directWrite();
    
    
    o_video_codec_ctx=Mux->get_o_video_codec_ctx();
    o_audio_codec_ctx=Mux->get_o_audio_codec_ctx();
    o_video_st=Mux->get_o_video_st();
    o_audio_st=Mux->get_o_audio_st();
    
    i_video_stream_idx=Demux->get_video_stream_idx();
    i_audio_stream_idx=Demux->get_audio_stream_idx();
    i_fmt_ctx=Demux->get_i_fmt_ctx();
    i_video_codec_ctx=Demux->get_i_video_codec_ctx();
    i_audio_codec_ctx=Demux->get_i_audio_codec_ctx();
    
    
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
        _isFail=true;
        return ;
    }
    pinframe->pts = 0;
    //video
    pout_video_frame = av_frame_alloc();
    if (pout_video_frame == NULL)
    {
        printf("avcodec_alloc_frame pout_video_frame error\n");
        _isFail=true;
        return ;
    }
    pout_video_frame->pts = 0;
    
    //audio
    pout_audio_frame = av_frame_alloc();
    if (pout_audio_frame == NULL)
    {
        printf("avcodec_alloc_frame pout_audio_frame error\n");
        _isFail=true;
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
//            if (!video_directWrite) {
//                <#statements#>
//            }
            //如果是视频需要编解码
//            if(des_bit_rate != i_fmt_ctx->streams[i_video_stream_idx]->codecpar->bit_rate ||
//               des_Width  != i_fmt_ctx->streams[i_video_stream_idx]->codecpar->width ||
//               des_Height != i_fmt_ctx->streams[i_video_stream_idx]->codecpar->height ||
//               video_codecID != i_fmt_ctx->streams[i_video_stream_idx]->codecpar->codec_id )//||
////               des_frameRate != av_q2d(i_fmt_ctx->streams[i_video_stream_idx]->r_frame_rate))
            if(!video_directWrite)
            {
                nRet = Demux->decode(AVMEDIA_TYPE_VIDEO, pinframe, pkt);
                if (nRet == 0)
                {
                    //进度
                    int64_t pts=pkt.dts;
                    int64_t duration=i_fmt_ctx->streams[i_video_stream_idx]->duration;
                    int schedule=pts*1.0/duration*100;

                    //使用流的方式int2String
                    stringstream stream;
                    stream<<schedule;
                    std::string strTemp=stream.str();   //此处也可以用 stream>>string_temp
                    if(this->c_sendInfo!= nullptr){
                        this->c_sendInfo(1,strTemp);//调试去看的话不懂为啥strTemp传过去非法的
                    }

                    LOGI("schedule:%d %%",schedule);

                    yuv_conversion(pinframe,pout_video_frame);
                    pout_video_frame->pts = pinframe->best_effort_timestamp;

                    nRet = Mux->code_and_write(AVMEDIA_TYPE_VIDEO, pout_video_frame);

                }
            }
            else
            {
                pkt.pos = -1;
                pkt.stream_index=o_video_stream_idx;
                nRet = av_interleaved_write_frame(o_fmt_ctx, &pkt);
                cout<<"video"<<endl;
                LOGI("video");
            }
        }
        //音频
        else if (pkt.stream_index == i_audio_stream_idx)
        {
            
            //如果是音频需要编解码
//            if(audio_codecID != i_fmt_ctx->streams[i_audio_stream_idx]->codecpar->codec_id  ||
//               des_BitsPerSample != i_fmt_ctx->streams[i_audio_stream_idx]->codec->sample_fmt||
//               des_Frequency !=i_fmt_ctx->streams[i_audio_stream_idx]->codecpar->sample_rate||
//               des_channelCount != i_fmt_ctx->streams[i_audio_stream_idx]->codecpar->channels)
            if(!audio_directWrite)
            {
                nRet = Demux->decode(AVMEDIA_TYPE_AUDIO, pinframe, pkt);
                if (nRet == 0)
                {
     
                        //                        av_frame_get_buffer(pout_audio_frame, 0);、不用获取了，虽然av_freep释放了，但是av_samples_alloc又会获得
                        if (swr_ctx == NULL)
                        {
                            swr_ctx = init_pcm_resample(pinframe,pout_audio_frame);
                            if (!swr_ctx) {
                                return 0;//失败
                            }
                            
                        }
                        if(pcm_resample(swr_ctx,pinframe,pout_audio_frame)<0)
                        {
                            return 0;//失败
                        }

                        
                        Mux->code_and_write(AVMEDIA_TYPE_AUDIO, pout_audio_frame);
                        av_freep(&(pout_audio_frame->data[0]));
                 

                }
            }
            else
            {
                //格式一模一样直接写

                pkt.pos = -1;
                pkt.stream_index=o_audio_stream_idx;
                nRet = av_interleaved_write_frame(o_fmt_ctx, &pkt);
                cout<<"audio"<<endl;
                LOGI("audio");
            }
        }
        
        av_packet_unref(&pkt);
    }
    
    
    
    
    
    //清缓存
    /* flush  encoders */
//    for (int i = 0; i < i_fmt_ctx->nb_streams; i++) {
//        
//        /* flush encoder */
//        if
//        flush_encoder(i);
//        
//    }
    if(!video_directWrite){
        flush_encoder(o_video_stream_idx);
    }
    if(!audio_directWrite){
        flush_encoder(o_audio_stream_idx);
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
    LOGI("schedule:%d %%",100);
    if(this->c_sendInfo!= nullptr){
        this->c_sendInfo(1,"100");//调试去看的话不懂为啥strTemp传过去非法的
    }
    return 1;
}


void CTransCode::yuv_conversion(AVFrame * pinframe,AVFrame * poutframe)
{
    //设置转换context
    
    if (img_convert_ctx_video == NULL)
    {
        img_convert_ctx_video = sws_getContext(i_fmt_ctx->streams[i_video_stream_idx]->codecpar->width, i_fmt_ctx->streams[i_video_stream_idx]->codecpar->height,
                                               i_video_codec_ctx->pix_fmt,
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
              0, i_fmt_ctx->streams[i_video_stream_idx]->codecpar->height, poutframe->data, poutframe->linesize);
//    poutframe->pkt_pts = pinframe->pkt_pts;
//    poutframe->pkt_dts = pinframe->pkt_dts;
//    //有时pkt_pts和pkt_dts不同，并且pkt_pts是编码前的dts,这里要给avframe传入pkt_dts而不能用pkt_pts
//    //poutframe->pts = poutframe->pkt_pts;
//    poutframe->pts = pinframe->pkt_dts;
    poutframe->pts=pinframe->pts;
    
    
    
    
    
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
//    AVCodecContext * audio_dec_ctx = i_fmt_ctx->streams[i_audio_stream_idx]->codec;
    AVCodecContext * audio_dec_ctx = i_audio_codec_ctx;
    if (audio_dec_ctx->channel_layout == 0)//不懂干嘛,把源数据的改了
    {
        audio_dec_ctx->channel_layout = av_get_default_channel_layout(i_fmt_ctx->streams[i_audio_stream_idx]->codecpar->channels);
    }
    /* set options */
    av_opt_set_int(swr_ctx, "in_channel_layout",    audio_dec_ctx->channel_layout, 0);
    av_opt_set_int(swr_ctx, "in_sample_rate",       audio_dec_ctx->sample_rate, 0);
    av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", audio_dec_ctx->sample_fmt, 0);
    if(i_fmt_ctx->streams[i_audio_stream_idx]->codecpar->channels > 2)
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
    out_frame->nb_samples = (int)av_rescale_rnd(swr_get_delay(swr_ctx,o_audio_st->codecpar->sample_rate) + src_nb_samples,
                                           o_audio_st->codecpar->sample_rate, o_audio_st->codecpar->sample_rate, AV_ROUND_UP);
    //一个输出一个输入
    int ret = av_samples_alloc(out_frame->data, &out_frame->linesize[0],
                               i_fmt_ctx->streams[i_audio_stream_idx]->codecpar->channels, out_frame->nb_samples,o_audio_codec_ctx->sample_fmt,1);
    if (ret < 0)
    {
        return NULL;
    }
    
    
    Mux->get_audiofifo()= av_audio_fifo_alloc(o_audio_codec_ctx->sample_fmt, o_audio_st->codecpar->channels,
                                              out_frame->nb_samples);
    
    return swr_ctx;
}



int CTransCode:: pcm_resample(SwrContext * pSwrCtx,AVFrame *in_frame, AVFrame *out_frame)
{
    if (pSwrCtx != NULL)
    {
        
        /*************************每次从新getdelay版本**********************/
        
        int64_t src_nb_samples = in_frame->nb_samples;//
        cout<<"delay:"<<swr_get_delay(pSwrCtx,o_audio_st->codecpar->sample_rate)<<endl;
        LOGI("getdelay%d",swr_get_delay(pSwrCtx,o_audio_st->codec->sample_rate));
        
        //重新开空间
        out_frame->nb_samples = (int)av_rescale_rnd(swr_get_delay(pSwrCtx,i_fmt_ctx->streams[i_audio_stream_idx]->codecpar->sample_rate) + src_nb_samples,
                                               o_audio_st->codecpar->sample_rate, i_fmt_ctx->streams[i_audio_stream_idx]->codecpar->sample_rate ,AV_ROUND_UP);
        
        
        int ret = av_samples_alloc(out_frame->data, &out_frame->linesize[0],
                                   i_fmt_ctx->streams[i_audio_stream_idx]->codecpar->channels, out_frame->nb_samples,o_audio_codec_ctx->sample_fmt,1);
        
        
        if (ret < 0)
        {
            return -1;
        }
        out_frame->nb_samples = swr_convert(pSwrCtx, out_frame->extended_data, out_frame->nb_samples,
                                            (const uint8_t**)in_frame->extended_data, in_frame->nb_samples);
        
      
        
       
        
        
//        std::unique_lock<std::mutex> lck(*(Mux->get_audioMtx()));
//        lck.lock();
        
        audioSemEmpty->P();
        
        
        audioMtx->P();
        cout<<"write:" <<av_audio_fifo_size(Mux->get_audiofifo())<<endl;
//        if(av_audio_fifo_size(Mux->get_audiofifo())>4096){
////           this->audioCondVar.wait(lck);//太多等待
//            Mux->get_audioCondVar()->wait(lck);
//        }
        
        
        av_audio_fifo_realloc(Mux->get_audiofifo(), av_audio_fifo_size(Mux->get_audiofifo()) + out_frame->nb_samples);
        
        av_audio_fifo_write(Mux->get_audiofifo(),(void **)out_frame->data,out_frame->nb_samples);
        
        audioMtx->V();
        audioSemFull->V();
        
//        out_frame->pkt_pts = in_frame->pkt_pts;
//        out_frame->pkt_dts = in_frame->pkt_dts;
//        //有时pkt_pts和pkt_dts不同，并且pkt_pts是编码前的dts,这里要给avframe传入pkt_dts而不能用pkt_pts
//        //out_frame->pts = out_frame->pkt_pts;
//        out_frame->pts = in_frame->pkt_dts;
        out_frame->pts=in_frame->pts;
    }
    return 0;
}


void CTransCode:: flush_encoder(int stream_index){
    int error=0;
  
    AVPacket pkt;
    AVCodecContext *CodecCtx;
    
    AVMediaType type=o_fmt_ctx->streams[stream_index]->codecpar->codec_type;
//    int (*enc_fun)(AVCodecContext *, AVPacket *, const AVFrame *, int *)=o_fmt_ctx->streams[stream_index]->codecpar->codec_type==AVMEDIA_TYPE_AUDIO ? avcodec_encode_audio2 : avcodec_encode_video2;
    if(type==AVMEDIA_TYPE_AUDIO){
        CodecCtx =o_audio_codec_ctx;
        if (!(o_audio_codec_ctx->codec->capabilities & AV_CODEC_CAP_DELAY))
            return;
    }else{
         CodecCtx =o_video_codec_ctx;
        if (!(o_video_codec_ctx->codec->capabilities &AV_CODEC_CAP_DELAY))
            return;
    }
   
   

    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;
   
    

    error = avcodec_send_frame(CodecCtx, NULL);
    /* The encoder signals that it has nothing more to encode. */
    if (error == AVERROR_EOF) {
         fprintf(stderr, "send packetAVERROR_EOF\n");
    } else if (error < 0) {
        fprintf(stderr, "Could not send packet for encoding (error '%s')\n",av_err2str(error));
        return ;
    }
    while (1) {//发一次nullptr,就可以了
        /* Receive one encoded frame from the encoder. */
        error = avcodec_receive_packet(CodecCtx, &pkt);
        /* If the encoder asks for more data to be able to provide an
         * encoded frame, return indicating that no data is present. */
        if (error == AVERROR(EAGAIN)) {
  
            /* If the last frame has been encoded, stop encoding. */
        } else if (error == AVERROR_EOF) {
    
            break;
            
        } else if (error < 0) {
            fprintf(stderr, "Could not encode frame (error '%s')\n",av_err2str(error));
            break;
            /* Default case: Return encoded data. */
        } else {
            
//            pkt.stream_index=o_video_stream_idx;//这样设置一下就可以直接写了，不用再设置pts、dts
            
            printf("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n",pkt.size);
            LOGI("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n",pkt.size);
            Mux->write_frame(type,pkt);
            av_packet_unref(&pkt);
        }
    }
    
}

void  CTransCode::read_packet(){
    
    AVPacket pkt;
    while (1)
    {
        av_init_packet(&pkt);
        if (av_read_frame(i_fmt_ctx, &pkt) < 0)
        {
            QueuePkt.Push(nullptr);
            break;
        }
        QueuePkt.Push(&pkt);

    }
    
}


int CTransCode::decodec_push_frame()
{
    AVPacket pkt;
//    AVFrame inFrame;
//    AVFrame outFrame;
//
//    AVFrame *pInFrame;
//    AVFrame *pOutFrame;
//
    int nRet=0;
//
//    nRet=av_frame_get_buffer(&inFrame, 0);
//    if(nRet){
//        return 0;
//    }
//    nRet=av_frame_get_buffer(&outFrame, 0);
//    if(nRet){
//        return 0;
//    }
    while (1)
    {
        av_init_packet(&pkt);
        nRet=QueuePkt.Pop(&pkt, 1);
        if (!nRet) {//finish
            
            QueueFrame.Push(nullptr,AVMEDIA_TYPE_UNKNOWN);
            break;
        }
        
        if(pkt.stream_index == i_video_stream_idx )
        {
            if(!video_directWrite)
            {
                nRet = Demux->decode(AVMEDIA_TYPE_VIDEO, pinframe, pkt);
                if (nRet == 0)
                {
                   
//                    //进度
//                    int64_t pts=pkt.dts;
//                    int64_t duration=i_fmt_ctx->streams[i_video_stream_idx]->duration;
//                    int schedule=pts*1.0/duration*100;
//
//                    //使用流的方式int2String
//                    stringstream stream;
//                    stream<<schedule;
//                    std::string strTemp=stream.str();   //此处也可以用 stream>>string_temp
//                    if(this->c_sendInfo!= nullptr){
//                        this->c_sendInfo(1,strTemp);//调试去看的话不懂为啥strTemp传过去非法的
//                    }
//
//                    LOGI("schedule:%d %%",schedule);
//
                    yuv_conversion(pinframe,pout_video_frame);
//                    pout_video_frame->pts = pinframe->best_effort_timestamp;

                    QueueFrame.Push(pout_video_frame,AVMEDIA_TYPE_VIDEO);
//                    nRet = Mux->code_and_write(AVMEDIA_TYPE_VIDEO, pout_video_frame);
                   
                }
            }
            else
            {
                pkt.pos = -1;
                pkt.stream_index=o_video_stream_idx;
                nRet = av_interleaved_write_frame(o_fmt_ctx, &pkt);
                cout<<"video"<<endl;
                LOGI("video");
            }
        }
        //音频
        else if (pkt.stream_index == i_audio_stream_idx)
        {
            
            
            if(!audio_directWrite)
            {
                nRet = Demux->decode(AVMEDIA_TYPE_AUDIO, pinframe, pkt);
                if (nRet == 0)
                {
                  
//                    //                        av_frame_get_buffer(pout_audio_frame, 0);、不用获取了，虽然av_freep释放了，但是av_samples_alloc又会获得
                    if (swr_ctx == NULL)
                    {
                        swr_ctx = init_pcm_resample(pinframe,pout_audio_frame);
                        if (!swr_ctx) {
                            return 0;//失败
                        }

                    }
                    if(pcm_resample(swr_ctx,pinframe,pout_audio_frame)<0)
                    {
                        return 0;//失败
                    }
                    QueueFrame.Push(pout_audio_frame,AVMEDIA_TYPE_AUDIO);

                    
                    
                }
            }
            else
            {
                //格式一模一样直接写
                
                pkt.pos = -1;
                pkt.stream_index=o_audio_stream_idx;
                nRet = av_interleaved_write_frame(o_fmt_ctx, &pkt);
                cout<<"audio"<<endl;
                LOGI("audio");
            }
        }
        
        av_packet_unref(&pkt);
    }
    return 1;
}

int CTransCode:: encodec_push_pack(){
    AVPacket pktWrite;
    AVFrame frame;
    int nRet=0;
    AVMediaType type;
    while (1)
    {
      
        nRet=QueueFrame.Pop(&frame,&type ,1);
        if (!nRet) {//finish
            QueuePktWrite.Push(nullptr);
            break;
        }
//        while (1) {
            nRet=Mux->encode(type, &frame, &pktWrite);
            if(nRet==0){
                QueuePktWrite.Push(&pktWrite);
//                if (type==AVMEDIA_TYPE_AUDIO) {
//                    continue;
//                }
//                else{
//                    break;
//                }
            }
//            else{
//                break;
//            }
//        }
       
        
       
    }
    return 1;
}
int CTransCode:: write_pack(){
    AVPacket pktWrite;
    int nRet=0;
    AVMediaType type;
    while (1)
    {
        
        nRet=QueuePktWrite.Pop(&pktWrite,1);
        if (!nRet) {//finish
            nRet=QueuePktWrite.GetLength();

            cout<<"finish"<<endl;
                        break;
        }
        type= pktWrite.stream_index==o_video_stream_idx?AVMEDIA_TYPE_VIDEO:AVMEDIA_TYPE_AUDIO;
        Mux->write_frame(type, pktWrite);
       
        
    }
    //清缓存
    if(!video_directWrite){
        flush_encoder(o_video_stream_idx);
    }
    if(!audio_directWrite){
        flush_encoder(o_audio_stream_idx);
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
    LOGI("schedule:%d %%",100);
    if(this->c_sendInfo!= nullptr){
        this->c_sendInfo(1,"100");//调试去看的话不懂为啥strTemp传过去非法的
    }
    return 1;
}

//int CTransCode::decode_and_push_frame(){
//
//    AVPacket pkt;
//    int nRet=0;
//    while (1)
//    {
//        av_init_packet(&pkt);
//        nRet=QueuePkt.Pop(&pkt, 1);
//        if (!nRet) {//finish
//            break;
//        }
//
//        if(pkt.stream_index == i_video_stream_idx )
//        {
//            //            if (!video_directWrite) {
//            //                <#statements#>
//            //            }
//            //如果是视频需要编解码
//            //            if(des_bit_rate != i_fmt_ctx->streams[i_video_stream_idx]->codecpar->bit_rate ||
//            //               des_Width  != i_fmt_ctx->streams[i_video_stream_idx]->codecpar->width ||
//            //               des_Height != i_fmt_ctx->streams[i_video_stream_idx]->codecpar->height ||
//            //               video_codecID != i_fmt_ctx->streams[i_video_stream_idx]->codecpar->codec_id )//||
//            ////               des_frameRate != av_q2d(i_fmt_ctx->streams[i_video_stream_idx]->r_frame_rate))
//            if(!video_directWrite)
//            {
//                nRet = Demux->decode(AVMEDIA_TYPE_VIDEO, pinframe, pkt);
//                if (nRet == 0)
//                {
//                    //进度
//                    int64_t pts=pkt.dts;
//                    int64_t duration=i_fmt_ctx->streams[i_video_stream_idx]->duration;
//                    int schedule=pts*1.0/duration*100;
//
//                    //使用流的方式int2String
//                    stringstream stream;
//                    stream<<schedule;
//                    std::string strTemp=stream.str();   //此处也可以用 stream>>string_temp
//                    if(this->c_sendInfo!= nullptr){
//                        this->c_sendInfo(1,strTemp);//调试去看的话不懂为啥strTemp传过去非法的
//                    }
//
//                    LOGI("schedule:%d %%",schedule);
//
//                    yuv_conversion(pinframe,pout_video_frame);
//                    pout_video_frame->pts = pinframe->best_effort_timestamp;
//
//                    nRet = Mux->code_and_write(AVMEDIA_TYPE_VIDEO, pout_video_frame);
//
//                }
//            }
//            else
//            {
//                pkt.pos = -1;
//                pkt.stream_index=o_video_stream_idx;
//                nRet = av_interleaved_write_frame(o_fmt_ctx, &pkt);
//                cout<<"video"<<endl;
//                LOGI("video");
//            }
//        }
//        //音频
//        else if (pkt.stream_index == i_audio_stream_idx)
//        {
//
//            //如果是音频需要编解码
//            //            if(audio_codecID != i_fmt_ctx->streams[i_audio_stream_idx]->codecpar->codec_id  ||
//            //               des_BitsPerSample != i_fmt_ctx->streams[i_audio_stream_idx]->codec->sample_fmt||
//            //               des_Frequency !=i_fmt_ctx->streams[i_audio_stream_idx]->codecpar->sample_rate||
//            //               des_channelCount != i_fmt_ctx->streams[i_audio_stream_idx]->codecpar->channels)
//            if(!audio_directWrite)
//            {
//                nRet = Demux->decode(AVMEDIA_TYPE_AUDIO, pinframe, pkt);
//                if (nRet == 0)
//                {
//
//                    //                        av_frame_get_buffer(pout_audio_frame, 0);、不用获取了，虽然av_freep释放了，但是av_samples_alloc又会获得
//                    if (swr_ctx == NULL)
//                    {
//                        swr_ctx = init_pcm_resample(pinframe,pout_audio_frame);
//                        if (!swr_ctx) {
//                            return 0;//失败
//                        }
//
//                    }
//                    if(pcm_resample(swr_ctx,pinframe,pout_audio_frame)<0)
//                    {
//                        return 0;//失败
//                    }
//
//
//                    Mux->code_and_write(AVMEDIA_TYPE_AUDIO, pout_audio_frame);
//                    av_freep(&(pout_audio_frame->data[0]));
//
//
//                }
//            }
//            else
//            {
//                //格式一模一样直接写
//
//                pkt.pos = -1;
//                pkt.stream_index=o_audio_stream_idx;
//                nRet = av_interleaved_write_frame(o_fmt_ctx, &pkt);
//                cout<<"audio"<<endl;
//                LOGI("audio");
//            }
//        }
//
//        av_packet_unref(&pkt);
//    }
//
//
//
//
//
//    //清缓存
//    if(!video_directWrite){
//        flush_encoder(o_video_stream_idx);
//    }
//    if(!audio_directWrite){
//        flush_encoder(o_audio_stream_idx);
//    }
//
//    if (pinframe)
//    {
//        av_frame_free(&pinframe);
//        pinframe = NULL;
//    }
//    if (pout_video_frame)
//    {
//        av_frame_free(&pout_video_frame);
//        pout_video_frame = NULL;
//    }
//    if (pout_audio_frame)
//    {
//        av_frame_free(&pout_audio_frame);
//        pout_audio_frame = NULL;
//    }
//    if (swr_ctx)
//    {
//        swr_free(&swr_ctx);
//        swr_ctx = NULL;
//    }
//    LOGI("schedule:%d %%",100);
//    if(this->c_sendInfo!= nullptr){
//        this->c_sendInfo(1,"100");//调试去看的话不懂为啥strTemp传过去非法的
//    }
//    return 1;
//}

