//
//  CQueuePacket.hpp
//  TransCodeForAndroid
//
//  Created by LYH on 2018/8/25.
//  Copyright © 2018年 LYH. All rights reserved.
//

#ifndef CQueuePacket_hpp
#define CQueuePacket_hpp

#include <stdio.h>

#include <iostream>
#include <string>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>


using namespace std;
extern "C"
{
    
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include "libavutil/avutil.h"
#include "libavutil/mem.h"
#include "libavutil/fifo.h"
    
#include "libswresample/swresample.h"
    
    
}

class CQueuePacket{
public:
    CQueuePacket( int maxLength=128);
    int Push(AVPacket *packet);
    int Pop(AVPacket *packet,int block);
    void flush();
    int GetLength(){return this->length;}
    int GetMaxlength(){return this->maxLength;}
    
    
private:
    AVPacketList *front;
    AVPacketList *rear;
    int maxLength;                  //最大对列长度
    int length;                     //队列长度
    int size;                       //队列总大小
    mutex QueueMtx;                 //互斥量
    condition_variable QueueCondVar;//条件变量
    bool end=false;
    
};

#endif /* CQueuePacket_hpp */
