//
//  CSemNamed.hpp
//  TransCodeForAndroid
//
//  Created by LYH on 2018/8/27.
//  Copyright © 2018年 LYH. All rights reserved.
//

#ifndef CSemNamed_hpp
#define CSemNamed_hpp
#include <semaphore.h>
#include <stdio.h>
class CSemNamed{
public:
    CSemNamed(const char *name, unsigned int value);
    ~CSemNamed();
    void P(){   sem_wait(sem_mutex);}//-1
    void V(){   sem_post(sem_mutex);}//+1
private:
    const char *name;
    sem_t *sem_mutex;
};

#endif /* CSemNamed_hpp */
