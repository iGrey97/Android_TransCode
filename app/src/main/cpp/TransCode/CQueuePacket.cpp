
//  CQueuePacket.cpp
//  TransCodeForAndroid
//
//  Created by LYH on 2018/8/25.
//  Copyright © 2018年 LYH. All rights reserved.
//

#include "CQueuePacket.hpp"

CQueuePacket::CQueuePacket(int maxLength) {
    this->maxLength=maxLength;
    
    this->front=NULL;
    this->rear=NULL;
    this->length=0;                      //队列长度
    this->size=0;                       //队列总大小
    
    
}

int CQueuePacket::Push(AVPacket *packet) {
    AVPacketList *newNode;
    
  
    
    if(!packet){
        end=true;
        this->QueueCondVar.notify_all();//通知一个wait的线程,避免等待时检测不到end
        return 0;
    }
    newNode=(AVPacketList *)av_malloc(sizeof(AVPacketList));
    if (newNode==NULL) {
        return -1;
    }

    
//    av_packet_copy_props(&newNode->pkt, packet);//不拷贝buf里的东西
    av_packet_move_ref(&newNode->pkt, packet);//全部拷贝一份，原来的置为空
    
    
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
    this->size+=packet->size;
    this->QueueCondVar.notify_all();//通知一个wait的线程
    //QueueMtx.unlock();
    return 0;
}

int CQueuePacket::Pop(AVPacket *packet, int block) {
    
    
    AVPacketList *pkt_node_temp;
    int ret=0;
    std::unique_lock<std::mutex> lck(this->QueueMtx);
    while (1) {
        pkt_node_temp=this->front;
        if (pkt_node_temp!=NULL) {//有数据
            this->front=this->front->next;
            if (front==NULL) {//队空了、队尾要指向空，若队没空，队尾该在哪还在哪
                this->rear=NULL;
            }
            this->length--;
            this->size-=pkt_node_temp->pkt.size;
            *packet=pkt_node_temp->pkt;
            av_free(pkt_node_temp);
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

void CQueuePacket::flush(){
    AVPacketList *ptemp=front;
    
    std::unique_lock<std::mutex> lck(this->QueueMtx);
    
    while (front!=NULL) {
        ptemp=front->next;
        front=ptemp;
        //        av_free_packet(&ptemp->pkt);
        av_packet_unref(&ptemp->pkt);
        
        av_freep(&ptemp);
    }
    rear = NULL;
    length = 0;
    size = 0;
    
    
    
}
