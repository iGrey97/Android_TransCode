//
//  main.cpp
//  TransCodeForAndroid
//
//  Created by LYH on 2018/8/21.
//  Copyright © 2018年 LYH. All rights reserved.
//
//
#include <iostream>
#include "CTransCode.hpp"
#include "CDemux.hpp"
#include "CMux.hpp"
#include <pthread.h>
#define  OUTPUTURL "/Users/lyh/Desktop/phone/1002.mp4"
#define  INPUTURL "/Users/lyh/Desktop/phone/100.mp4"
//#define INPUTURL   "/Users/lyh/Documents/MyProgram/FFMPEG/Transcoding/transcodeV2_0/father.avi"
//#define OUTPUTURL  "/Users/lyh/Documents/MyProgram/FFMPEG/Transcoding/transcodeV2_0/father1111.mp4"
void *ReadFun(void *arg){
    CTransCode *TransCode=(CTransCode *)arg;
    TransCode->read_packet();
    return nullptr;
}
void *WriteFun(void *arg){
    CTransCode *TransCode=(CTransCode *)arg;
    TransCode->write_pack();
    return nullptr;
}
void *DecodecFun(void *arg){
    CTransCode *TransCode=(CTransCode *)arg;
    TransCode->decodec_push_frame();
    return nullptr;
}
void *EncodecFun(void *arg){
    CTransCode *TransCode=(CTransCode *)arg;
    TransCode->encodec_push_pack();
    return nullptr;
}

int main(int argc, const char * argv[]) {

    CDemux Demux(nullptr,INPUTURL);
    if(Demux.isFail()){
        return 0;
    }
    CMux Mux(nullptr,&Demux,OUTPUTURL);
    if(Mux.isFail()){
        return 0;
    }
    CTransCode TransCode(nullptr,&Demux,&Mux);
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
//    TransCode.Trans();
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

    return 0;

}


//#include <pthread.h>
//#include <stdio.h>
//#include <semaphore.h>
//#include<iostream>
//#include "CQueuePacket.hpp"
//using namespace std;
//
//#define BUFF_SIZE 10
//char buffer[BUFF_SIZE];
//int count1=0; // 缓冲池里的信息数目
//sem_t  *sem_mutex; // 生产者和消费者的互斥锁
//sem_t  *p_sem_mutex; // 空的时候，对消费者不可进
//sem_t  *c_sem_mutex; // 满的时候，对生产者不可进
//CQueuePacket  *QueuePkt;
//CQueuePacket  *QueuePktWrite;
//void * Producer(void *arg)
//{
//    AVPacket *pkt;
//    while(1)
//    {
//        pkt=av_packet_alloc();
////        av_packet
//        QueuePkt->Push(pkt,1);
//        cout<<"Producer:"<<QueuePkt->GetLength()<<endl;
//
////        sem_wait(p_sem_mutex); //当缓冲池未满时
////        sem_wait(sem_mutex); //等待缓冲池空闲
////        count1++;
////        cout<<"Producer:"<<count1<<endl;
////        sem_post(sem_mutex);
////        //        if(count < BUFF_SIZE)//缓冲池未满
////        //            sem_post(&p_sem_mutex);
////        //        if(count > 0) //缓冲池不为空
////        sem_post(c_sem_mutex);
//    }
//    return NULL;
//}
//void * Consumer(void *arg)
//{
//    AVPacket pkt;
//    int nRet;
//    while(1)
//    {
//        nRet=QueuePkt->Pop(&pkt, 1);
//        if(nRet!=1){
//            cout<<"error"<<endl;
//        }
//        cout<<"Consumer:"<<QueuePkt->GetLength()<<endl;
//
//
//
////        sem_wait(c_sem_mutex);//缓冲池未空时
////        sem_wait(sem_mutex); //等待缓冲池空闲∫
////        count1--;
////        cout<<"Consumer:"<<count1<<endl;
////        sem_post(sem_mutex);
////        //        if(count > 0)
////        sem_post(p_sem_mutex);
//    }
//    return NULL;
//}
//sem_t * i_sem_open(const char *name, unsigned int value){
//
//    sem_t *sem_mutex   = sem_open(name  , 0);
//    if (sem_mutex!=SEM_FAILED) {//如果存在
//        sem_close(sem_mutex);
//        sem_unlink(name);
//        sem_mutex   = sem_open(name  , O_CREAT, 0666, value);
//
//    }else{
//        sem_mutex   = sem_open(name  , O_CREAT, 0666, value);
//    }
//    return sem_mutex;
//}
//
//int main()
//{
//    pthread_t ptid,ctid;
//    //initialize the semaphores
////    p_sem_mutex = i_sem_open("/p_sem_mutex",  5);
////    c_sem_mutex = i_sem_open("/c_sem_mutex",  0);
////    sem_mutex   = i_sem_open("/sem_mutex"  ,  1);
////
//
//     p_sem_mutex = i_sem_open("/p_sem_mutex",  5);
//    QueuePkt=new CQueuePacket("/QueuePkt",2);
//    //    QueueFrame;
//    //creating producer and consumer threads
//    if(pthread_create(&ptid, NULL,Producer, NULL))
//    {
//        printf("\n ERROR creating thread 1");
//        //        exit(1);
//    }
//    if(pthread_create(&ctid, NULL,Consumer, NULL))
//    {
//        printf("\n ERROR creating thread 2");
//        //        exit(1);
//    }
//    if(pthread_join(ptid, NULL)) /* wait for the producer to finish */
//    {
//        printf("\n ERROR joining thread");
//        //        exit(1);
//    }
//    if(pthread_join(ctid, NULL)) /* wait for consumer to finish */
//    {
//        printf("\n ERROR joining thread");
//        //        exit(1);
//    }
//
//    //exit the main thread
//    pthread_exit(NULL);
//    return 1;
//}
