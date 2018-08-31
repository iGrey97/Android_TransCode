//
//  CSemNamed.cpp
//  TransCodeForAndroid
//
//  Created by LYH on 2018/8/27.
//  Copyright © 2018年 LYH. All rights reserved.
//

#include "CSemNamed.hpp"


CSemNamed::CSemNamed(const char *name, unsigned int value)
:name(name){
    //mac 上不能使用无名的，
    //android 上不能使用有名的


    //使用有名信号量（需内核支持）
    sem_mutex   = sem_open(name  , 0);
    if (sem_mutex!=SEM_FAILED) {//如果存在
        sem_close(sem_mutex);
        sem_unlink(name);
        sem_mutex   = sem_open(name  , O_CREAT, 0666, value);

    }else{
        sem_mutex   = sem_open(name  , O_CREAT, 0666, value);
    }

    //使用无名信号量
    if(sem_mutex== nullptr){
        nameflag= false;
        sem_mutex =new sem_t;
        sem_init(sem_mutex,0,value);
    }
   
}


CSemNamed::~CSemNamed() {

    sem_close(sem_mutex);//关闭信号量，进程终止时，会自动调用它
    if(nameflag) {
        sem_unlink(name);//删除信号量，立即删除信号量名字，当其他进程都关闭它时，销毁它
    }

}
