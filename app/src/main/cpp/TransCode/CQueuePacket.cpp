
//  CQueuePacket.cpp
//  TransCodeForAndroid
//
//  Created by LYH on 2018/8/25.
//  Copyright © 2018年 LYH. All rights reserved.
//

#include "CQueuePacket.hpp"

CQueuePacket::CQueuePacket(const char *semName,const int maxLength):
maxLength(maxLength){
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

int CQueuePacket::Push(AVPacket *packet,int who) {
    AVPacketList *newNode;
    
  
    semEmpty->P();

    newNode=(AVPacketList *)av_malloc(sizeof(AVPacketList));
    if (newNode==NULL) {
        semEmpty->V();
        return -1;
    }
    if(!packet){
        newNode->pkt.data=NULL;
 
    }else{
        //    av_packet_copy_props(&newNode->pkt, packet);//不拷贝buf里的东西
        av_packet_move_ref(&newNode->pkt, packet);//全部拷贝一份，原来的置为空
        
    }
    

    
    
    newNode->next=NULL;
    
    semMtx->P();
    if (this->rear==NULL) {//空队
        this->front=newNode;
        
    }else{
        this->rear->next=newNode;
    }
    this->rear=newNode;
    this->length++;
    LOGE("%d Packet::Push length：%d\n",who,this->length);
    semMtx->V();

    semFull->V();
    return 0;
}

int CQueuePacket::GetLength(){
    int tempLength=0;
    semMtx->P();
    tempLength=this->length;
    semMtx->V();
    return tempLength;
}

int CQueuePacket::Pop(AVPacket *packet, int who) {
    
    
    AVPacketList *pkt_node_temp;
    int ret=0;

    semFull->P();
    semMtx->P();

    pkt_node_temp=this->front;

    if (pkt_node_temp!=NULL) {//有数据
        this->front=this->front->next;
        if (front==NULL) {//队空了、队尾要指向空，若队没空，队尾该在哪还在哪
            this->rear=NULL;
        }
        this->length--;
        LOGE("%d Packet::Pop length：%d\n",who,this->length);
        


        *packet=pkt_node_temp->pkt;
        av_free(pkt_node_temp);
        if (packet->data==nullptr) {
            ret=0;
        }else{
            ret=1;
        }
        semMtx->V();
    } else{
        LOGE("%d Packet::Pop length：%d err\n",who,this->length);
    }
    semEmpty->V();
    return ret;
    
}


