//
//  main.cpp
//  TransCodeForAndroid
//
//  Created by LYH on 2018/8/21.
//  Copyright © 2018年 LYH. All rights reserved.
//

#include <iostream>
#include "CTransCode.hpp"
#include "CDemux.hpp"
#include "CMux.hpp"
#define  OUTPUTURL "/Users/lyh/Desktop/phone/1001.mp4"
#define  INPUTURL "/Users/lyh/Desktop/phone/100.mp4"

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
    start=clock();//us为单位
    TransCode.Trans();
    finish=clock();
    //////////////////////////////////////////////////////////////////////////
    printf("--------finish----------\n");
    printf("________%f(ms)__________\n",1000.0 * (finish-start)/CLOCKS_PER_SEC);
    
    return 0;
    
}
