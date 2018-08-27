//
//  CQueueFrame.cpp
//  TransCodeForAndroid
//
//  Created by LYH on 2018/8/25.
//  Copyright © 2018年 LYH. All rights reserved.
//

#include "CQueueFrame.hpp"


CQueueFrame::CQueueFrame(int maxLength) {
    this->maxLength=maxLength;
    
    this->front=NULL;
    this->rear=NULL;
    this->length=0;                      //队列长度
    
    
}

int CQueueFrame::Push(AVFrame *frame,AVMediaType type) {
    AVFrametList *newNode;
    
    int nRet=0;
    if(!frame){
        end=true;
        this->QueueCondVar.notify_all();//通知一个wait的线程,避免等待时检测不到end
        return 0;
    }
    newNode=(AVFrametList *)av_malloc(sizeof(AVFrametList));
    if (newNode==NULL) {
        return -1;
    }
    //以下三行拷贝了原来data到新的地方，且不会动原来的
//    nRet= av_frame_ref(&newNode->frame, frame);
//    nRet=av_frame_get_buffer(&newNode->frame, 0);
//    nRet=av_frame_copy(&newNode->frame, frame);
    /* The following fields must be set on frame before calling this function:
    * - format (pixel format for video, sample format for audio)
    * - width and height for video
    * - nb_samples and channel_layout for audio
    */
//    av_frame_copy_props(frame, decodeFrame->frame);
//    av_frame_move_ref(frame, decodeFrame->frame);
    newNode->frame=*frame;//这个地方是内容复制一份，不是保存packet指针

    newNode->type=type;
    newNode->next=NULL;
    
    //QueueMtx.lock();普通加锁
    std::unique_lock<std::mutex> lck(this->QueueMtx);//自动加锁与解锁
    //lck ,unique_lock 对象
    if (this->rear==NULL) {//空队
        this->front=newNode;
        
    }else{
        this->rear->next=newNode;
    }
    this->rear=newNode;
    this->length++;
    this->QueueCondVar.notify_all();//通知一个wait的线程
    //QueueMtx.unlock();
    return 0;
}

int CQueueFrame::Pop(AVFrame *frame,AVMediaType *type, int block) {
    
    
    AVFrametList *frame_node_temp;
    int ret=0;
    std::unique_lock<std::mutex> lck(this->QueueMtx);
    while (1) {
        frame_node_temp=this->front;
        if (frame_node_temp!=NULL) {//有数据
            this->front=this->front->next;
            if (front==NULL) {//队空了、队尾要指向空，若队没空，队尾该在哪还在哪
                this->rear=NULL;
            }
            this->length--;
//            *frame=frame_node_temp->frame;
            av_frame_move_ref( frame, &frame_node_temp->frame);
            *type=frame_node_temp->type;
            av_free(frame_node_temp);
            ret=1;
            break;
        }else if(block==0){//没数据、非阻塞
            ret=0;
            break;
        }else{//没数据、阻塞
            if (end) {
                ret=0;
                break;
            }
            this->QueueCondVar.wait(lck);
        }
    }
    
    return ret;
    
}

void CQueueFrame::flush(){
    AVFrametList *ptemp=front;
    
    std::unique_lock<std::mutex> lck(this->QueueMtx);
    
    while (front!=NULL) {
        ptemp=front->next;
        front=ptemp;
        AVFrame *pFrame=&ptemp->frame;
        av_frame_free(&pFrame);
        av_freep(&ptemp);
    }
    rear = NULL;
    length = 0;
    
    
    
}
