//
//  LogHelper.h
//  mtmv
//
//  Created by cwq on 15/7/29.
//  Copyright (c) 2015年 meitu. All rights reserved.
//

#ifndef mtmv_LogHelper_h
#define mtmv_LogHelper_h

#define DEBUG_NATVIE

#ifdef DEBUG_NATVIE

#include <android/log.h>
#define  LOG_TAG    "MULTIMEDIATOOLS"
#define  LOGV(...)  __android_log_print(ANDROID_LOG_VERBOSE,LOG_TAG,__VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#else // !DEBUG_NATVIE

#define  LOGV(...)
#define  LOGD(...)
#define  LOGI(...)
#define  LOGW(...)
#define  LOGE(...)

#endif // !DEBUG_NATVIE

#endif
