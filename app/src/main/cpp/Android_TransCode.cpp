//
// Created by LYH on 2018/8/15.
//
//#include "com_meitu_lyh_android_transcode_MainActivity.h"
#include "com_meitu_lyh_android_transcode_Android_TransCode.h"
#include <string>
#include <iostream>
#include "CMux.hpp"
#include "CDemux.hpp"
#include "CTransCode.hpp"
#include <pthread.h>
using namespace std;


JavaVM *g_jvm = NULL;
jobject object_global;
jmethodID methodID_func_global;

void c_sendInfo(int what, std::string info){


    JNIEnv *env= nullptr;
    if (g_jvm->AttachCurrentThread(&env, NULL) != JNI_OK) {
        LOGE("AttachCurrentThread() failed");
        return ;
    }

    jstring jstrInfo = env->NewStringUTF(info.c_str());

    //调用signTest方法
    env->CallVoidMethod(object_global,methodID_func_global,what,jstrInfo);
    env->DeleteLocalRef(jstrInfo);


}





void *ReadFun(void *arg){
    CTransCode *TransCode=(CTransCode *)arg;
    TransCode->read_packet();
    return nullptr;
}
void *DecodecFun(void *arg){
    CTransCode *TransCode=(CTransCode *)arg;
    TransCode->decodec_push_frame();
    g_jvm->DetachCurrentThread();
    return nullptr;
}
void *EncodecFun(void *arg){
    CTransCode *TransCode=(CTransCode *)arg;
    TransCode->encodec_push_pack();
    return nullptr;
}
void *WriteFun(void *arg){
    CTransCode *TransCode=(CTransCode *)arg;
    TransCode->write_pack();
    g_jvm->DetachCurrentThread();//一定要报JNIENV从线程解绑定，不然会段错误
    return nullptr;
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



    jclass native_clazz = env->GetObjectClass(object);
    jmethodID methodID_func=env->GetMethodID(native_clazz,"sendInfo","(ILjava/lang/String;)V");
    //特别注意：String后面一定有分号（；）结束的,I不用




    //JNIEnv指针只在当前线程中有效
    //若想在别的线程使用，得保存JavaVM，在别的线程用AttachCurrentThread将线程附加到javaVM上，获得属于当前线程的JNIEnv
    // 线程结束要DetachCurrentThread分离
    env->GetJavaVM(&g_jvm);
    object_global=env->NewGlobalRef(object);//全局引用，否则在局部的话结束了会释放掉，别的线程不能用了
    methodID_func_global=methodID_func;




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
    clock_t start,finish;
    printf("--------start----------\n");
    //////////////////////////////////////////////////////////////////////////

    pthread_t Thread_Read_Id;
    pthread_t Thread_Decodec_Id;
    pthread_t Thread_Encodec_Id;
    pthread_t Thread_Write_Id;



    start=clock();//us为单位

    pthread_create(&Thread_Read_Id, NULL, ReadFun, (void *)&TransCode);
    pthread_create(&Thread_Decodec_Id,NULL,DecodecFun, (void *)&TransCode);
    pthread_create(&Thread_Encodec_Id,NULL,EncodecFun,(void *)&TransCode);
    pthread_create(&Thread_Write_Id, NULL, WriteFun, (void *)&TransCode);
    pthread_join(Thread_Read_Id, NULL);
    pthread_join(Thread_Decodec_Id, NULL);
    pthread_join(Thread_Encodec_Id, NULL);
    pthread_join(Thread_Write_Id, NULL);
    finish=clock();
    //////////////////////////////////////////////////////////////////////////
    printf("--------finish----------\n");
    printf("________%f(ms)__________\n",1000.0 * (finish-start)/CLOCKS_PER_SEC);

    c_sendInfo(1,"100");

    return TransCode.Trans();







}







