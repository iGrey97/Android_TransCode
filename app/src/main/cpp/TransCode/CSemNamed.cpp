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

   sem_mutex   = sem_open(name  , 0);
    if (sem_mutex!=SEM_FAILED) {//如果存在
        sem_close(sem_mutex);
        sem_unlink(name);
        sem_mutex   = sem_open(name  , O_CREAT, 0666, value);
        
    }else{
        sem_mutex   = sem_open(name  , O_CREAT, 0666, value);
    }
   
}


CSemNamed::~CSemNamed() { 
    sem_close(sem_mutex);
    sem_unlink(name);
}
