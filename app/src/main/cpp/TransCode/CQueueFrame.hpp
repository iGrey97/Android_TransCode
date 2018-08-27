//
//  CQueueFrame.hpp
//  TransCodeForAndroid
//
//  Created by LYH on 2018/8/25.
//  Copyright © 2018年 LYH. All rights reserved.
//

#ifndef CQueueFrame_hpp
#define CQueueFrame_hpp

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


typedef struct AVFrametList {
    AVFrame frame;
    AVMediaType type;
    struct AVFrametList *next;
} AVFrametList;



class CQueueFrame{
public:
    CQueueFrame( int maxLength=128);
    int Push(AVFrame *frame,AVMediaType type);
    int Pop(AVFrame *frame,AVMediaType *type,int block);
    void flush();
    int GetLength(){return this->length;}
    int GetMaxlength(){return this->maxLength;}
    
    
private:
    AVFrametList *front;
    AVFrametList *rear;
    int maxLength;                  //最大对列长度
    int length;                     //队列长度
    mutex QueueMtx;                 //互斥量
    condition_variable QueueCondVar;//条件变量
    bool end=false;
    
};
#endif /* CQueueFrame_hpp */
