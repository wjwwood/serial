#ifndef __LOG_H_
#define __LOG_H_

#include <android/log.h>

#define LOG_TAG "serial"	

#if NDK_DEBUG

#define LOGA(cond,fmt,...)          ((cond) ? ((void)0) : __android_log_assert(0, LOG_TAG, fmt, ##__VA_ARGS__))
#define LOGV(fmt,...)               __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, fmt, ##__VA_ARGS__)
#define LOGD(fmt,...)               __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, fmt, ##__VA_ARGS__)

#define LOG_ASSERT(cond,fmt,...)    ((cond) ? ((void)0) : __android_log_assert(0, tag, fmt, ##__VA_ARGS__))
#define LOG_VERBOSE(tag,fmt,...)    __android_log_print(ANDROID_LOG_VERBOSE, tag, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(tag,fmt,...)      __android_log_print(ANDROID_LOG_VERBOSE, tag, fmt, ##__VA_ARGS__)

#else

#define LOGA(...)
#define LOGV(...)
#define LOGD(...)

#define LOG_VERBOSE(...)
#define LOG_DEBUG(...)

#endif

#define LOGI(fmt,...)               __android_log_print(ANDROID_LOG_INFO, LOG_TAG, fmt, ##__VA_ARGS__)
#define LOGW(fmt,...)               __android_log_print(ANDROID_LOG_WARN, LOG_TAG, fmt, ##__VA_ARGS__)
#define LOGE(fmt,...)               __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, fmt, ##__VA_ARGS__)

#define LOG_INFO(tag,fmt,...)       __android_log_print(ANDROID_LOG_INFO, tag, fmt, ##__VA_ARGS__)
#define LOG_WARN(tag,fmt,...)       __android_log_print(ANDROID_LOG_WARN, tag, fmt, ##__VA_ARGS__)
#define LOG_ERROR(tag,fmt,...)      __android_log_print(ANDROID_LOG_ERROR, tag, fmt, ##__VA_ARGS__)

#endif
