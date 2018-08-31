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
#include "LogHelper.h"
#include "CSemNamed.hpp"

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
    CQueueFrame(const char *semName, const int maxLength=128);
    int Push(AVFrame *frame,AVMediaType type);
    int Pop(AVFrame *frame,AVMediaType *type,int block);
    int GetLength();
    int GetMaxlength(){return this->maxLength;}
    
    
private:
    CSemNamed *semFull;
    CSemNamed *semEmpty;
    CSemNamed *semMtx;
    AVFrametList *front;
    AVFrametList *rear;
    const int maxLength;                  //最大对列长度
    int length;                     //队列长度
    bool end=false;
    
};
#endif /* CQueueFrame_hpp */
