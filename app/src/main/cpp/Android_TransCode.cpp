//
// Created by LYH on 2018/8/15.
//
//#include "com_meitu_lyh_android_transcode_MainActivity.h"
#include "com_meitu_lyh_android_transcode_Android_TransCode.h"
#include <string>

#include "CMux.hpp"
#include "CDemux.hpp"
#include "CTransCode.hpp"

JNIEnv *env_global= nullptr;
jobject object_global;
jmethodID methodID_func_global;

void c_sendInfo(int what, std::string info){

    jstring jstrInfo = env_global->NewStringUTF(info.c_str());

    //调用signTest方法
    env_global->CallIntMethod(object_global,methodID_func_global,what,jstrInfo);


}




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



    //
    jclass native_clazz = env->GetObjectClass(object);
    jmethodID methodID_func=env->GetMethodID(native_clazz,"sendInfo","(ILjava/lang/String;)V");
    //特别注意：String后面一定有分号（；）结束的,I不用

    env_global= env;
    object_global=object;
    methodID_func_global=methodID_func;


//    c_sendInfo(1,"58");



    CDemux Demux(c_sendInfo,str_i_file.c_str());
    if(Demux.isFail()){
        return 0;
    }
    CMux   Mux(c_sendInfo,&Demux,str_o_file.c_str(),width,height ,channels,audio_samp_rate);
    if(Mux.isFail()){
        return 0;
    }
    CTransCode TransCode(c_sendInfo,&Demux,&Mux);
    if(TransCode.isFail()){
        return 0;
    }
    return TransCode.Trans();





}

