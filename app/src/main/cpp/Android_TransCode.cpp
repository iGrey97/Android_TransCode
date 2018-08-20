//
// Created by LYH on 2018/8/15.
//
//#include "com_meitu_lyh_android_transcode_MainActivity.h"
#include "com_meitu_lyh_android_transcode_Android_TransCode.h"
#include <string>

#include "CMux.hpp"
#include "CDemux.hpp"
#include "CTransCode.hpp"
JNIEXPORT jint JNICALL Java_com_meitu_lyh_android_1transcode_Android_1TransCode_jni_1trancode
        (JNIEnv *env, jobject object, jstring i_file, jstring o_file, jint width, jint height, jint channels, jint audio_samp_rate){

    LOGD("cpp Start");

    //GetStringUTFChars可以把一个jstring指针（指向JVM内部的Unicode字符序列）转化成一个UTF-8格式的C字符串。
    const char *c_i_file=env->GetStringUTFChars(i_file,NULL);
    if (c_i_file == NULL) {
    //因为JVM需要为新诞生的UTF-8字符串分配内存，这个操作有可能因为内存太少而失败。
        return 0; /* OutOfMemoryError already thrown */

    }
    std::string str_i_file(c_i_file);
    env->ReleaseStringUTFChars(i_file,c_i_file);

    const  char *c_o_file=env->GetStringUTFChars(o_file,NULL);
    if (c_o_file == NULL) {

        return 0; /* OutOfMemoryError already thrown */

    }
    std::string str_o_file(c_o_file);
    env->ReleaseStringUTFChars(o_file,c_o_file);



    CDemux Demux(str_i_file.c_str());
    CMux   Mux(&Demux,str_o_file.c_str(),width,height ,channels,audio_samp_rate);

    CTransCode TransCode(&Demux,&Mux);

    return TransCode.Trans();
}

//JNIEXPORT jint JNICALL Java_com_meitu_lyh_android_1transcode_MainActivity_Android_1TransCode
//        (JNIEnv *env, jobject obj, jstring i_file, jstring o_file){
//
//
//
//    const char *c_i_file= env->GetStringUTFChars(i_file,NULL);
//    std::string i_file_str(c_i_file);
//    env->ReleaseStringUTFChars(i_file,c_i_file);
//
//    const char *c_o_file= env->GetStringUTFChars(o_file,NULL);
//    std::string o_file_str(c_o_file);
//    env->ReleaseStringUTFChars(o_file,c_o_file);
//
////    av_register_all();
////    av_log(NULL, AV_LOG_ERROR, "错误了!\n");
//
//
//
//
//
//
//}
//
////int itran(){
////
////    av_register_all();
////    av_log(NULL, AV_LOG_ERROR, "错误了!\n");
////    CDemux Demux("aaa");
////    return 1;
////}