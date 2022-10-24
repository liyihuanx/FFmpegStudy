//
// Created by k1 on 2022/7/6.
//

#ifndef FFMPEGSTUDY_UTIL_H
#define FFMPEGSTUDY_UTIL_H


#define NELEM(m) (sizeof(m) / sizeof((m)[0]))

static long long getSysCurrentTime() {
    struct timeval time;
    gettimeofday(&time, NULL);
    long long curTime = ((long long) (time.tv_sec)) * 1000 + time.tv_usec / 1000;
    return curTime;
}


// 方法的开始时间和结束时间
#define FUN_BEGIN_TIME(FUN) {\
    LOGD("%s:%s func start", __FILE__, FUN); \
    long long t0 = getSysCurrentTime();

#define FUN_END_TIME(FUN) \
    long long t1 = getSysCurrentTime(); \
    LOGD("%s:%s func cost time %ldms", __FILE__, FUN, (long)(t1-t0));}


#endif //FFMPEGSTUDY_UTIL_H

