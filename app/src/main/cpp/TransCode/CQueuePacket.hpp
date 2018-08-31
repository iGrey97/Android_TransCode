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
#include "CSemNamed.hpp"
#include "LogHelper.h"

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
    CQueuePacket( const char *semName,const int maxLength=128);
    int Push(AVPacket *packet,int who);
    int Pop(AVPacket *packet,int who);
    int GetLength();
    int GetMaxlength(){return this->maxLength;}
    
    
private:
    CSemNamed *semFull;
    CSemNamed *semEmpty;
    CSemNamed *semMtx;
    AVPacketList *front;
    AVPacketList *rear;
    int maxLength;                  //最大对列长度
    int length;                     //队列长度

    
};

#endif /* CQueuePacket_hpp */
