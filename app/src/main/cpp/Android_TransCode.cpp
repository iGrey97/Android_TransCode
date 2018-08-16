//
// Created by LYH on 2018/8/15.
//
#include "com_meitu_lyh_android_transcode_MainActivity.h"
#include <string>





JNIEXPORT jint JNICALL Java_com_meitu_lyh_android_1transcode_MainActivity_Android_1TransCode
        (JNIEnv *env, jobject obj, jstring i_file, jstring o_file){



    const char *c_i_file= env->GetStringUTFChars(i_file,NULL);
    std::string i_file_str(c_i_file);
    env->ReleaseStringUTFChars(i_file,c_i_file);

    const char *c_o_file= env->GetStringUTFChars(o_file,NULL);
    std::string o_file_str(c_o_file);
    env->ReleaseStringUTFChars(o_file,c_o_file);

//    av_register_all();
//    av_log(NULL, AV_LOG_ERROR, "错误了!\n");



    CDemux Demux(i_file_str.c_str());
    CMux   Mux(&Demux,o_file_str.c_str());
    CTransCode TransCode(&Demux,&Mux);
    return TransCode.Trans();


}

//int itran(){
//
//    av_register_all();
//    av_log(NULL, AV_LOG_ERROR, "错误了!\n");
//    CDemux Demux("aaa");
//    return 1;
//}