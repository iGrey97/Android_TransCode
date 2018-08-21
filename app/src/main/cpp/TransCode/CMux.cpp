//
//  CMux.cpp
//  iTransCode
//
//  Created by LYH on 2018/8/12.
//  Copyright © 2018年 LYH. All rights reserved.
//

#include "CMux.hpp"
CMux::CMux(void (*c_sendInfo)(int what, string info),CDemux *Demux,const char* o_Filename,
           int des_Width,
           int des_Height ,
           int des_ChannelCount,      //声道
           int des_Frequency,      //采样率


           double des_FrameRate,  //帧率
           AVCodecID des_Video_codecID ,
           AVPixelFormat des_Video_pixelfromat ,
           int des_bit_rate ,
           int des_gop_size ,
           int des_max_b_frame ,
           int des_thread_count ,

           //audio param
           uint64_t  des_Layout,

           AVCodecID des_Audio_codecID ,//AV_CODEC_ID_AC3
           AVSampleFormat des_BitsPerSample
           )
:c_sendInfo(c_sendInfo),Demux(Demux),o_Filename(o_Filename)
{
    _isFail= false;

    this->i_fmt_ctx=Demux->get_i_fmt_ctx();
    //video param

    //-1则保持出入输出一样
    if(des_Width==-1) {
        this->des_Width=Demux->get_i_Width();
    }else{
        this->des_Width=des_Width;
    }

    if(des_Height==-1) {
        this->des_Height=Demux->get_i_Height();
    }else{
        this->des_Height=des_Height;
    }



    if(des_ChannelCount==-1) {
        this->des_ChannelCount=Demux->get_i_ChannelCount();       //声道
    }else{
        this->des_ChannelCount=des_ChannelCount;       //声道
    }

    if(des_Frequency==-1) {
        this->des_Frequency=Demux->get_i_Frequency();
    }else{
        this->des_Frequency=des_Frequency;
    }




    this->des_FrameRate=des_FrameRate;  //帧率
    this->des_Video_codecID =des_Video_codecID;
    this->des_Video_pixelfromat=des_Video_pixelfromat;
    this->des_bit_rate=des_bit_rate;
    this->des_gop_size=des_gop_size;
    this->des_max_b_frame=des_max_b_frame;
    this->des_thread_count=des_thread_count;
    i_video_stream_idx=Demux->get_video_stream_idx();
    i_audio_stream_idx=Demux->get_audio_stream_idx();

    //audio param
    this->des_Layout=des_Layout;

    //    44100
    //aac
    this->des_Audio_codecID = AV_CODEC_ID_AAC;//AV_CODEC_ID_AC3
    this->des_BitsPerSample=des_BitsPerSample;





    this->o_fmt_ctx = nullptr;
    this->o_video_stream_idx =-1;
    this->o_audio_stream_idx =-1;




    this->o_video_st = nullptr;
    this->o_audio_st = nullptr;

    this->audio_codec = nullptr ;
    this->video_codec = nullptr;

    this->vbsf_aac_adtstoasc = nullptr;
    this->audiofifo = nullptr;




    int ret=0;
    vbsf_aac_adtstoasc = NULL;

    /* allocate the output media context */
    avformat_alloc_output_context2(&o_fmt_ctx, NULL,NULL, o_Filename);
    if (!o_fmt_ctx)
    {

        if(this->c_sendInfo!= nullptr){
            this-> c_sendInfo(2,"alloc_output_context err\n");//调试去看的话不懂为啥strTemp传过去非法的
        }
        _isFail=true;
        return ;
    }
    AVOutputFormat* ofmt = NULL;
    ofmt = o_fmt_ctx->oformat;

    /* open the output file, if needed */
    if (!(ofmt->flags & AVFMT_NOFILE)){

        ret= avio_open(&o_fmt_ctx->pb, o_Filename, AVIO_FLAG_WRITE);
        if (ret < 0)
        {
            av_strerror(ret, szError, 256);
            printf("Could not open '%s'\n", o_Filename);
            LOGE("Could not open'%s'\n", szError);

            if(this->c_sendInfo!= nullptr){
                this-> c_sendInfo(2,szError);//调试去看的话不懂为啥strTemp传过去非法的
            }
            _isFail=true;
            return ;
        }
    }
    for (int i = 0; i < i_fmt_ctx->nb_streams; i++){

        if (i_fmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){

            o_video_stream_idx = i;//记录输出序号

            double FrameRate = i_fmt_ctx->streams[i]->r_frame_rate.num /(double)i_fmt_ctx->streams[i]->r_frame_rate.den;
            this->des_FrameRate =(int)(FrameRate + 0.5);

            ofmt->video_codec = des_Video_codecID;
            //匹配视频yuv格式
            video_support(des_Video_codecID,&this->des_Video_pixelfromat);
            //如果是视频需要编解码
            if(this->des_bit_rate != i_fmt_ctx->streams[i_video_stream_idx]->codec->bit_rate ||
               this->des_Width != i_fmt_ctx->streams[i_video_stream_idx]->codec->width ||
               this->des_Height != i_fmt_ctx->streams[i_video_stream_idx]->codec->height ||
               this->des_Video_codecID != i_fmt_ctx->streams[i_video_stream_idx]->codec->codec_id ||
               this->des_FrameRate != av_q2d(i_fmt_ctx->streams[i_video_stream_idx]->r_frame_rate))
            {
                o_video_st = add_out_stream(i_fmt_ctx, AVMEDIA_TYPE_VIDEO,&video_codec);
            }
            else
            {
                //直接重封装就好了


                o_video_st = avformat_new_stream(o_fmt_ctx,i_fmt_ctx->streams[i_video_stream_idx]->codec->codec);
                if (!o_video_st) {
                    if(this->c_sendInfo!= nullptr){
                        this->c_sendInfo(2,"Failed to new stream \n");//调试去看的话不懂为啥strTemp传过去非法的
                    }
                    _isFail=true;
                    return ;
                }
                //复制AVCodecContext的设置（Copy the settings of AVCodecContext）
                ret = avcodec_copy_context(o_video_st->codec, i_fmt_ctx->streams[i_video_stream_idx]->codec);
                if (ret < 0) {
                    printf( "Failed to copy context from input to output stream codec context\n");
                    LOGE("Failed to copy context from input to output stream codec context\n");

                    if(this->c_sendInfo!= nullptr){
                        this->c_sendInfo(2,"Failed to copy context from input to output stream codec context\n");//调试去看的话不懂为啥strTemp传过去非法的
                    }
                    _isFail=true;
                    return ;
                }

                o_video_st->codec->codec_tag = 0;
                if (o_fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
                    o_video_st->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

            }




        }else if (i_fmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO){
            o_audio_stream_idx = i;





            ofmt->audio_codec = des_Audio_codecID;
            //匹配音频pcm格式
            audio_support(des_Audio_codecID, &this->des_ChannelCount, &this->des_Layout,&this->des_Frequency, &this->des_BitsPerSample);

            //如果是音频需要编解码
            if(this->des_Audio_codecID != i_fmt_ctx->streams[i_audio_stream_idx]->codec->codec_id  ||
               this->des_BitsPerSample != i_fmt_ctx->streams[i_audio_stream_idx]->codec->sample_fmt||
               this->des_Frequency !=i_fmt_ctx->streams[i_audio_stream_idx]->codec->sample_rate||
               this->des_ChannelCount != i_fmt_ctx->streams[i_audio_stream_idx]->codec->channels)
            {
                o_audio_st = add_out_stream(i_fmt_ctx, AVMEDIA_TYPE_AUDIO,&audio_codec);

            }
            else
            {



                //输入输出格式一样只需要封装

                o_audio_st = avformat_new_stream(i_fmt_ctx,i_fmt_ctx->streams[i_audio_stream_idx]->codec->codec);
                if (!o_audio_st) {
                    if(this->c_sendInfo!= nullptr){
                        this->c_sendInfo(2,"Failed to new stream\n");//调试去看的话不懂为啥strTemp传过去非法的
                    }
                    _isFail=true;
                    return ;
                }
                //复制AVCodecContext的设置（Copy the settings of AVCodecContext）
                ret = avcodec_copy_context(o_audio_st->codec, i_fmt_ctx->streams[i_audio_stream_idx]->codec);
                if (ret < 0) {
                    printf( "Failed to copy context from input to output stream codec context\n");
                    if(this->c_sendInfo!= nullptr){
                        this->c_sendInfo(2,"Failed to copy context from input to output stream codec context\n");//调试去看的话不懂为啥strTemp传过去非法的
                    }
                    _isFail=true;
                    return ;
                }

                o_audio_st->codec->codec_tag = 0;
                if (o_fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
                    o_audio_st->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;



            }
            //android 下面失败
//            if ((strstr(o_fmt_ctx->oformat->name, "flv") != NULL) ||
//                (strstr(o_fmt_ctx->oformat->name, "mp4") != NULL) ||
//                (strstr(o_fmt_ctx->oformat->name, "mov") != NULL) ||
//                (strstr(o_fmt_ctx->oformat->name, "3gp") != NULL))
//            {

//                if (o_audio_st->codec->codec_id == AV_CODEC_ID_AAC)
//                {
//
//                    audio_filter = av_bsf_get_by_name("aac_adtstoasc");
//                    if(!audio_filter)
//                    {
//                        av_log(NULL,AV_LOG_ERROR,"Unkonw bitstream filter");
//                        LOGE("Unkonw bitstream filter");
//                        return ;
//                    }
//
//                    ret = av_bsf_alloc(audio_filter, &audio_bsf_ctx);
//                    if(!ret){
//                        av_log(NULL,AV_LOG_ERROR,"av_bsf_alloc");
//                        LOGE("av_bsf_alloc");
//                        return ;
//                    }
//
//                    vbsf_aac_adtstoasc =  av_bitstream_filter_init("aac_adtstoasc");
//                    if(vbsf_aac_adtstoasc == NULL)
//                    {
//                        return ;
//                    }
//                }
//            }


        }

    }


    av_dump_format(o_fmt_ctx, 0, o_Filename, 1);

    if (o_video_stream_idx != -1)//如果存在视频
    {
        //如果是视频需要编解码
        if(this->des_bit_rate != i_fmt_ctx->streams[i_video_stream_idx]->codec->bit_rate ||
           this->des_Width != i_fmt_ctx->streams[i_video_stream_idx]->codec->width ||
           this->des_Height != i_fmt_ctx->streams[i_video_stream_idx]->codec->height ||
           this->des_Video_codecID != i_fmt_ctx->streams[i_video_stream_idx]->codec->codec_id ||
           this->des_FrameRate != av_q2d(i_fmt_ctx->streams[i_video_stream_idx]->r_frame_rate))
        {
            //解码初始化
            ret = Demux->openDecode(i_video_stream_idx);
            if(ret!=1){
                //打开失败
                if(this->c_sendInfo!= nullptr){
                    this->c_sendInfo(2,"Failed to openDecode\n");
                }
            }
            //编码初始化
            ret = openCode(o_video_stream_idx);
            if(ret!=1){
                //打开失败
                if(this->c_sendInfo!= nullptr){
                    this->c_sendInfo(2,"Failed to openCode\n");
                }
            }
        }
    }
    //如果是音频需要编解码
    if(o_audio_stream_idx != -1)//如果存在音频
    {
        if(this->des_Audio_codecID != i_fmt_ctx->streams[i_audio_stream_idx]->codec->codec_id  ||
           this->des_BitsPerSample != i_fmt_ctx->streams[i_audio_stream_idx]->codec->sample_fmt||
           this->des_Frequency !=i_fmt_ctx->streams[i_audio_stream_idx]->codec->sample_rate||
           this->des_ChannelCount != i_fmt_ctx->streams[i_audio_stream_idx]->codec->channels)
        {
            //解码初始化
            ret = Demux->openDecode(i_audio_stream_idx);
            if(ret!=1){
                //打开失败
                if(this->c_sendInfo!= nullptr){
                    this->c_sendInfo(2,"Failed to openDecode\n");
                }
            }
            //编码初始化
            ret = openCode(o_audio_stream_idx);
            if(ret!=1){
                //打开失败
                if(this->c_sendInfo!= nullptr){
                    this->c_sendInfo(2,"Failed to openCode\n");
                }
            }
        }
    }
     ret = avformat_write_header(o_fmt_ctx, NULL);//会设置流的时基（视频90000、音频sample_rate）
    if (ret != 0)
    {
        printf("Call avformat_write_header function failed.\n");
        if(this->c_sendInfo!= nullptr){
            this->c_sendInfo(2,"Call avformat_write_header function failed.\n");//调试去看的话不懂为啥strTemp传过去非法的
        }
        _isFail=true;
        return ;
    }
    return ;
}


CMux:: ~CMux(){

    if(!this->o_fmt_ctx){
        return;
    }
    int i = 0;
    int nRet = av_write_trailer(o_fmt_ctx);
    if (nRet < 0)
    {
        av_strerror(nRet, szError, 256);
//        printf(szError);
        printf("\n");
        printf("Call av_write_trailer function failed\n");
        if(!c_sendInfo){
            c_sendInfo(2,szError);//调试去看的话不懂为啥strTemp传过去非法的
        }
    }
    if (vbsf_aac_adtstoasc !=NULL)
    {
        av_bitstream_filter_close(vbsf_aac_adtstoasc);
        vbsf_aac_adtstoasc = NULL;
    }
    av_dump_format(o_fmt_ctx, -1, o_Filename, 1);

    if (i_video_stream_idx != -1)//如果存在视频
    {


        //如果是视频需要编解码
        if(des_bit_rate != i_fmt_ctx->streams[i_video_stream_idx]->codec->bit_rate ||
           des_Width != i_fmt_ctx->streams[i_video_stream_idx]->codec->width ||
           des_Height != i_fmt_ctx->streams[i_video_stream_idx]->codec->height ||
           des_Video_codecID != i_fmt_ctx->streams[i_video_stream_idx]->codec->codec_id ||
           des_FrameRate != av_q2d(i_fmt_ctx->streams[i_video_stream_idx]->r_frame_rate))
        {
            Demux->closeDecode(i_video_stream_idx);
            closeCode(o_video_stream_idx);
        }
    }
    if(i_audio_stream_idx != -1)//如果存在音频
    {
        //如果是音频需要编解码
        if(des_Audio_codecID != i_fmt_ctx->streams[i_audio_stream_idx]->codec->codec_id  ||
           des_BitsPerSample != i_fmt_ctx->streams[i_audio_stream_idx]->codec->sample_fmt||
           des_Frequency !=i_fmt_ctx->streams[i_audio_stream_idx]->codec->sample_rate||
           des_ChannelCount != i_fmt_ctx->streams[i_audio_stream_idx]->codec->channels)
        {
            Demux->closeDecode(i_audio_stream_idx);
            closeCode(o_audio_stream_idx);
        }
    }
    /* Free the streams. */
    for (i = 0; i < o_fmt_ctx->nb_streams; i++)
    {
        av_freep(&o_fmt_ctx->streams[i]->codec);
        av_freep(&o_fmt_ctx->streams[i]);
    }
    if (!(o_fmt_ctx->oformat->flags & AVFMT_NOFILE))
    {
        /* Close the output file. */
        avio_close(o_fmt_ctx->pb);
    }
    av_free(o_fmt_ctx);

}

int CMux::audio_support(enum AVCodecID codecId,int *channel,uint64_t * layout,int *samplePerSec,AVSampleFormat * sample_fmt)
{
    AVCodec *codec = avcodec_find_encoder(codecId);

    //支持的声道
    if(NULL != codec->channel_layouts)
    {
        *layout = av_get_default_channel_layout(*channel);
        if(0 == *layout)
        {
            return 0;
        }

        int i = 0;
        int j = 0;
        while(0 != codec->channel_layouts[j])
        {
            printf("pCodec->channel_layouts[j] : %d\n",codec->channel_layouts[j]);
            ++j;
        }
        while(0 != codec->channel_layouts[i])
        {
            if(*layout == codec->channel_layouts[i])
            {
                break;
            }
            ++i;
        }
        //未找到
        if(0 == codec->channel_layouts[i])
        {
            *layout = codec->channel_layouts[i-1];
            *channel = av_get_channel_layout_nb_channels(*layout);
        }
    }

    //支持的采样率
    if(NULL != codec->supported_samplerates)
    {
        int i = 0;
        int j = 0;
        while(0 != codec->supported_samplerates[j])
        {
            printf("pCodec->supported_samplerates[j] : %d\n",codec->supported_samplerates[j]);
            ++j;
        }
        while(0 != codec->supported_samplerates[i])
        {
            if(*samplePerSec == codec->supported_samplerates[i])
            {
                break;
            }
            ++i;
        }
        //未找到
        if(0 == codec->supported_samplerates[i])
        {
            *samplePerSec = codec->supported_samplerates[i-1];
        }
    }

    //支持的样本
    if(NULL != codec->sample_fmts)
    {
        int i = 0;
        int j = 0;
        while(-1 != codec->sample_fmts[j])
        {
            printf("pCodec->sample_fmts[j] : %d\n",codec->sample_fmts[j]);
            ++j;
        }
        while(-1 != codec->sample_fmts[i])
        {
            if(*sample_fmt == codec->sample_fmts[i])
            {
                break;
            }
            ++i;
        }
        //未找到
        if(-1 == codec->sample_fmts[i])
        {
            *sample_fmt = codec->sample_fmts[i-1];
        }
    }

    return 1;
}

int CMux::video_support(enum AVCodecID codecId,AVPixelFormat * video_pixelfromat)
{


    AVCodec *codec = avcodec_find_encoder(codecId);
    //支持的yuv格式
    if(NULL != codec->pix_fmts)
    {
        int i = 0;
        int j = 0;
        while(-1 != codec->pix_fmts[j])
        {
            printf("pCodec->pix_fmts[j] : %d\n",codec->pix_fmts[j]);
            ++j;
        }
        while(-1 != codec->pix_fmts[i])
        {
            if(*video_pixelfromat == codec->pix_fmts[i])
            {
                break;
            }
            ++i;
        }
        //未找到
        if(-1 == codec->pix_fmts[i])
        {
            *video_pixelfromat = codec->pix_fmts[i-1];
        }
    }
    return 1;
}



AVStream * CMux::add_out_stream(AVFormatContext* i_fmt_ctx,AVMediaType codec_type_t,AVCodec **codec)
{
    AVCodecContext* output_codec_context = NULL;
    AVStream * in_stream = NULL;
    AVStream * output_stream = NULL;
    AVCodecID codecID;

    switch (codec_type_t)
    {
        case AVMEDIA_TYPE_AUDIO:
            codecID = des_Audio_codecID;
            in_stream = i_fmt_ctx->streams[i_audio_stream_idx];
            break;
        case AVMEDIA_TYPE_VIDEO:
            codecID = des_Video_codecID;
            in_stream = i_fmt_ctx->streams[i_video_stream_idx];
            break;
        default:
            break;
    }






    /* find the encoder */
    *codec = avcodec_find_encoder(codecID);
    if (!(*codec))
    {
        return NULL;
    }

    output_stream = avformat_new_stream(o_fmt_ctx,*codec);
    if (!output_stream)
    {
        return NULL;
    }

    output_stream->id = o_fmt_ctx->nb_streams - 1;
    output_codec_context = output_stream->codec;


    output_stream->time_base  = in_stream->time_base;//编码时 avformat_write_header()后可能被系统改变（如90000），改不改取决于格式，不设置也由系统设置

    switch (codec_type_t)
    {
        case AVMEDIA_TYPE_AUDIO:


            output_codec_context->codec_id = des_Audio_codecID;
            output_codec_context->codec_type = codec_type_t;
            output_stream->start_time = 0;
            output_codec_context->sample_rate = i_fmt_ctx->streams[i_audio_stream_idx]->codec->sample_rate;//m_dwFrequency;
            output_codec_context->sample_rate =des_Frequency;
            if(i_fmt_ctx->streams[i_audio_stream_idx]->codec->channels > 2)
            {
                output_codec_context->channels = des_ChannelCount;
                output_codec_context->channel_layout = av_get_default_channel_layout(des_ChannelCount);
            }
            else
            {
                output_codec_context->channels  = i_fmt_ctx->streams[i_audio_stream_idx]->codec->channels;
                if (i_fmt_ctx->streams[i_audio_stream_idx]->codec->channel_layout == 0)
                {
                    output_codec_context->channel_layout = av_get_default_channel_layout(i_fmt_ctx->streams[i_audio_stream_idx]->codec->channels);
                }
                else
                {
                    output_codec_context->channel_layout = i_fmt_ctx->streams[i_audio_stream_idx]->codec->channel_layout;
                }
            }
            //这个码率有些编码器不支持特别大，例如wav的码率是1411200 比aac大了10倍多
            output_codec_context->bit_rate = 128000;//icodec->streams[audio_stream_idx]->codec->bit_rate;
            //            output_codec_context->frame_size = audio_frame_size;
            output_codec_context->sample_fmt  = des_BitsPerSample; //样本
            //             output_codec_context->sample_fmt = (*codec)->sample_fmts[0];
            //            m_dwBitsPerSample=  output_codec_context->sample_fmt ;
            output_codec_context->block_align = 0;


            //查看音频支持的声道，采样率，样本
            //            audio_support(*codec,&output_codec_context->channels,
            //                          (int *)&output_codec_context->channel_layout,
            //                          &output_codec_context->sample_rate,
            //                          &output_codec_context->sample_fmt);
            //            m_dwChannelCount = output_codec_context->channels;
            //            m_dwFrequency = output_codec_context->sample_rate;
            //            m_dwBitsPerSample = output_codec_context->sample_fmt;
            break;
        case AVMEDIA_TYPE_VIDEO:
            AVRational r_frame_rate_t;
            r_frame_rate_t.num = 100;
            r_frame_rate_t.den = (int)(des_FrameRate * 100);
            output_codec_context->time_base = in_stream->codec->time_base;
            //            output_stream->time_base  = in_stream->time_base;
            output_stream->r_frame_rate.num = r_frame_rate_t.den;
            output_stream->r_frame_rate.den = r_frame_rate_t.num;
            output_codec_context->codec_id = des_Video_codecID;
            output_codec_context->codec_type = codec_type_t;
            output_stream->start_time = 0;
            output_codec_context->pix_fmt = des_Video_pixelfromat;
            output_codec_context->width = des_Width;
            output_codec_context->height = des_Height;
            output_codec_context->bit_rate = des_bit_rate;

            //            output_codec_context->bit_rate =in_stream->codec->bit_rate;
            //            bit_rate= output_codec_context->bit_rate ;

            output_codec_context->gop_size  = des_gop_size;         /* emit one intra frame every twelve frames at most */;
            output_codec_context->max_b_frames = des_max_b_frame;    //设置B帧最大数
            output_codec_context->thread_count = des_thread_count;  //线程数目
            output_codec_context->me_range = 16;
            output_codec_context->max_qdiff = 4;
            output_codec_context->qmin = 20; //调节清晰度和编码速度 //这个值调节编码后输出数据量越大输出数据量越小，越大编码速度越快，清晰度越差
            output_codec_context->qmax = 40; //调节清晰度和编码速度
            output_codec_context->qcompress = 0.6;
            //            //查看视频支持的yuv格式
            //            video_support(*codec,&output_codec_context->pix_fmt);
            //            video_pixelfromat = output_codec_context->pix_fmt;
            break;
        default:
            break;
    }
    //这个很重要，要么纯复用解复用，不做编解码写头会失败,
    //另或者需要编解码如果不这样，生成的文件没有预览图，还有添加下面的header失败，置0之后会重新生成extradata
    output_codec_context->codec_tag = 0;
    //if(! strcmp( output_format_context-> oformat-> name,  "mp4" ) ||
    //    !strcmp (output_format_context ->oformat ->name , "mov" ) ||
    //    !strcmp (output_format_context ->oformat ->name , "3gp" ) ||
    //    !strcmp (output_format_context ->oformat ->name , "flv" ))
    if(AVFMT_GLOBALHEADER & o_fmt_ctx->oformat->flags)
    {
        output_codec_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    return output_stream;
}


int CMux::openCode(int stream_idx)
{
    AVCodecContext *CodecCtx = NULL;

    if (stream_idx == o_audio_stream_idx)
    {
        CodecCtx = o_audio_st->codec;
        //打开编码器
        int nRet = avcodec_open2(CodecCtx, audio_codec, NULL);
        if (nRet < 0)
        {
            printf("Could not open encoder\n");
            return 0;
        }
    }
    else if (stream_idx == o_video_stream_idx)
    {

        CodecCtx = o_video_st->codec;
        //打开编码器
        int nRet = avcodec_open2(CodecCtx, video_codec, NULL);
        if (nRet < 0)
        {
            printf("Could not open encoder\n");
            return -1;
        }
    }
    return 1;
}


int CMux:: closeCode(int stream_idx)
{
    AVCodecContext *CodecCtx = NULL;

    if (stream_idx == o_audio_stream_idx)
    {
        CodecCtx = o_audio_st->codec;
    }
    else if (stream_idx == o_video_stream_idx)
    {
        CodecCtx = o_video_st->codec;
    }
    avcodec_close(CodecCtx);
    return 1;
}


void CMux:: write_frame(AVMediaType type,AVPacket &pkt){
    int64_t pts = 0, dts = 0;
    int nRet = -1;

    if(type == AVMEDIA_TYPE_VIDEO)
    {
#ifdef STREAM_TB

        nRet = av_interleaved_write_frame(o_fmt_ctx, &pkt_t);
        if (nRet != 0)
        {
            printf("error av_interleaved_write_frame _ video\n");
        }
#else



        AVPacket videopacket_t;//备份一下，应为av_interleaved_write_frame会释放掉
        av_init_packet(&videopacket_t);

        videopacket_t.pts = av_rescale_q_rnd(pkt.pts, i_fmt_ctx->streams[i_video_stream_idx]->time_base, o_video_st->time_base, AV_ROUND_NEAR_INF);
        videopacket_t.dts = av_rescale_q_rnd(pkt.dts, i_fmt_ctx->streams[i_video_stream_idx]->time_base, o_video_st->time_base, AV_ROUND_NEAR_INF);
        videopacket_t.duration = av_rescale_q(pkt.duration,i_fmt_ctx->streams[i_video_stream_idx]->time_base, o_video_st->time_base);
        videopacket_t.flags = pkt.flags;
        videopacket_t.stream_index = o_video_stream_idx; //这里add_out_stream顺序有影响
        videopacket_t.data = pkt.data;
        videopacket_t.size = pkt.size;



        nRet = av_interleaved_write_frame(o_fmt_ctx, &videopacket_t);
        if (nRet != 0)
        {
            printf("error av_interleaved_write_frame _ video\n");
        }
#endif
        printf("video\n");
        LOGI("video");
    }
    else if(type == AVMEDIA_TYPE_AUDIO)
    {
        AVPacket audiopacket_t;
        av_init_packet(&audiopacket_t);

        audiopacket_t.flags = pkt.flags;
        audiopacket_t.stream_index =o_audio_stream_idx; //这里add_out_stream顺序有影响
        audiopacket_t.data = pkt.data;
        audiopacket_t.size = pkt.size;

        // android下失败
        //添加过滤器
//        if(! strcmp(o_fmt_ctx->oformat-> name,  "mp4" ) ||
//           !strcmp (o_fmt_ctx ->oformat ->name , "mov" ) ||
//           !strcmp (o_fmt_ctx ->oformat ->name , "3gp" ) ||
//           !strcmp (o_fmt_ctx ->oformat ->name , "flv" ))
//        {
//            if (o_audio_st->codec->codec_id == AV_CODEC_ID_AAC)
//            {
//                if (vbsf_aac_adtstoasc != NULL)
//                {
//                    AVPacket filteredPacket = audiopacket_t;
//                    int a = av_bitstream_filter_filter(vbsf_aac_adtstoasc,
//                                                       o_audio_st->codec, NULL,&filteredPacket.data, &filteredPacket.size,
//                                                       audiopacket_t.data, audiopacket_t.size, audiopacket_t.flags & AV_PKT_FLAG_KEY);
//                    if (a >  0)
//                    {
//                        av_free_packet(&audiopacket_t);
//
//                        audiopacket_t = filteredPacket;
//                    }
//                    else if (a == 0)
//                    {
//                        audiopacket_t = filteredPacket;
//                    }
//                    else if (a < 0)
//                    {
//                        fprintf(stderr, "%s failed for stream %d, codec %s",
//                                vbsf_aac_adtstoasc->filter->name,audiopacket_t.stream_index,o_audio_st->codec->codec ?  o_audio_st->codec->codec->name : "copy");
//                        av_free_packet(&audiopacket_t);
//
//                    }
//                }
//            }
//        }
        nRet = av_interleaved_write_frame(o_fmt_ctx, &audiopacket_t);
        if (nRet != 0)
        {
            printf("error av_interleaved_write_frame _ audio\n");
        }
        printf("audio\n");
        LOGI("audio");
    }
}

int CMux:: code_and_write(AVMediaType type,AVFrame * frame)
{
    int nRet=0;
    AVCodecContext *CodecCtx = NULL;
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = NULL; // packet data will be allocated by the encoder
    pkt.size = 0;
    int frameFinished = 0 ;
//    int i_audio_stream_idx=Demux->get_audio_stream_idx();
//    int i_video_stream_idx=Demux->get_video_stream_idx();
    if (type == AVMEDIA_TYPE_AUDIO)
    {

        CodecCtx = o_audio_st->codec;
        //如果进和出的的声道，样本，采样率不同,需要重采样
        if(i_fmt_ctx->streams[i_audio_stream_idx]->codec->sample_fmt != des_BitsPerSample ||
           i_fmt_ctx->streams[i_audio_stream_idx]->codec->channels != des_ChannelCount ||
           i_fmt_ctx->streams[i_audio_stream_idx]->codec->sample_rate != des_Frequency)
        {
            int64_t pts_t = frame->pts;


            AVFrame * pFrameResample = av_frame_alloc();

            pFrameResample->nb_samples     = CodecCtx->frame_size;
            pFrameResample->channel_layout = CodecCtx->channel_layout;
            pFrameResample->channels       = CodecCtx->channels;
            pFrameResample->format         = CodecCtx->sample_fmt;
            pFrameResample->sample_rate    = CodecCtx->sample_rate;
            int error = 0;
            if ((error = av_frame_get_buffer(pFrameResample, 0)) < 0)
            {
                av_frame_free(&pFrameResample);
                return error;
            }

            while (av_audio_fifo_size(audiofifo) >= pFrameResample->nb_samples) //取出写入的未读的包
            {
                av_audio_fifo_read(audiofifo,(void **)pFrameResample->data,pFrameResample->nb_samples);



                pFrameResample->pts=frame->pts;

                nRet =avcodec_encode_audio2(CodecCtx,&pkt,pFrameResample,&frameFinished);
                if (nRet>=0 && frameFinished)
                {
                    write_frame(AVMEDIA_TYPE_AUDIO,pkt);
                    av_free_packet(&pkt);
                }
            }
            if (pFrameResample)
            {
                av_frame_unref(pFrameResample);

                av_frame_free(&pFrameResample);
                pFrameResample = NULL;
            }


        }
        else
        {
            nRet = avcodec_encode_audio2(CodecCtx,&pkt,frame,&frameFinished);
            if (nRet>=0 && frameFinished)
            {
                write_frame(AVMEDIA_TYPE_AUDIO,pkt);
                av_free_packet(&pkt);
            }
        }
        //free



    }
    else if (type == AVMEDIA_TYPE_VIDEO)
    {
        CodecCtx = o_video_st->codec;
        avcodec_encode_video2(CodecCtx,&pkt,frame,&frameFinished);

        pkt.stream_index=o_video_stream_idx;//这样设置一下就可以直接写了，不用再设置pts、dts
        if (frameFinished)
        {
            write_frame(AVMEDIA_TYPE_VIDEO,pkt);
            av_free_packet(&pkt);
        }

    }
    return 1;
}
