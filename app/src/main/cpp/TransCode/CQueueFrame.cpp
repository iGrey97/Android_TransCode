//
//  CQueueFrame.cpp
//  TransCodeForAndroid
//
//  Created by LYH on 2018/8/25.
//  Copyright © 2018年 LYH. All rights reserved.
//

#include "CQueueFrame.hpp"


CQueueFrame::CQueueFrame(const char *semName,const int maxLength)
:maxLength(maxLength){
    string semEmptyName=semName;
    semEmptyName+="Empty";
    string semFullName=semName;
    semFullName+="Full";
    string semMtxName=semName;
    semMtxName+="Mtx";
    
    semEmpty=new CSemNamed(semEmptyName.c_str(),maxLength);
    semFull=new CSemNamed(semFullName.c_str(),0);
    semMtx=new CSemNamed(semMtxName.c_str(),1);
    
    this->front=NULL;
    this->rear=NULL;
    this->length=0;                      //队列长度
    
    
}



int CQueueFrame::Push(AVFrame *frame,AVMediaType type) {
    AVFrametList *newNode;
    
    semEmpty->P();
    newNode=(AVFrametList *)av_malloc(sizeof(AVFrametList));
    if (newNode==NULL) {
        semEmpty->P();
        return -1;
    }

    if(frame==nullptr){
        *(newNode->frame.data)=nullptr;
        
    }else{
         newNode->frame=*frame;//这个地方是内容复制一份，不是保存packet指针
        /////////////
//        if(type==AVMEDIA_TYPE_VIDEO){
//            newNode->frame.format=frame->format;
//            newNode->frame.width=frame->width;
//            newNode->frame.height=frame->height;
//            //        nRet= av_frame_ref(&newNode->frame, frame);
//            //        if(nRet<0){
//            //            cout<<"error"<<endl;
//            //        }
//            nRet=av_frame_get_buffer(&(newNode->frame), 0);
//            if(nRet<0){
//                cout<<"error"<<endl;
//            }
//            nRet=av_frame_copy(&(newNode->frame), frame);
//            if(nRet<0){
//                cout<<"error"<<endl;
//            }
//        }

    }
    
    newNode->type=type;
    newNode->next=NULL;
    

    semMtx->P();
    if (this->rear==NULL) {//空队
        this->front=newNode;
        
    }else{
        this->rear->next=newNode;
    }
    this->rear=newNode;
    this->length++;
    LOGE("frame::Push length：%d\n",this->length);
    semMtx->V();
    semFull->V();

    return 0;
}

int CQueueFrame::Pop(AVFrame *frame,AVMediaType *type, int block) {
    
    
    AVFrametList *frame_node_temp;
    int ret=0;

    semFull->P();
    semMtx->P();

    frame_node_temp=this->front;
    if (frame_node_temp!=NULL) {//有数据
        this->front=this->front->next;
        if (front==NULL) {//队空了、队尾要指向空，若队没空，队尾该在哪还在哪
            this->rear=NULL;
        }
        this->length--;
        LOGE("frame::Pop length：%d\n",this->length);
        *frame=frame_node_temp->frame;
        semMtx->V();
        av_frame_move_ref( frame, &frame_node_temp->frame);
        *type=frame_node_temp->type;
        av_free(frame_node_temp);
        if(  *(frame->data)==nullptr){
            ret=0;
        }else{
           ret=1;
        }

    }
    semEmpty->V();
    return ret;
    
}

int CQueueFrame::GetLength(){
    int tempLength=0;
    semMtx->P();
    tempLength=this->length;
    semMtx->V();
    return tempLength;
}






