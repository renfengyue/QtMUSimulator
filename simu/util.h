#ifndef _UTIL_H_
#define _UTIL_H_

#include "all.h"

#if defined MUSIM_STD_DEBUG
#define MUSIM_STD_MSG0(string)                printf(string)
#define MUSIM_STD_MSG1(string, a)             printf(string, a)
#define MUSIM_STD_MSG2(string, a, b)          printf(string, a, b)
#define MUSIM_STD_MSG3(string, a, b, c)       printf(string, a, b, c)
#define MUSIM_STD_MSG4(string, a, b, c, d)    printf(string, a, b, c, d)
#else
#define MUSIM_STD_MSG0(string)
#define MUSIM_STD_MSG1(string, a)
#define MUSIM_STD_MSG2(string, a, b)
#define MUSIM_STD_MSG3(string, a, b, c)
#define MUSIM_STD_MSG4(string, a, b, c, d)
#endif

#if defined MUSIM_EXTSTD_DEBUG
#define MUSIM_EXTSTD_MSG0(string)             printf(string)
#define MUSIM_EXTSTD_MSG1(string, a)          printf(string, a)
#define MUSIM_EXTSTD_MSG2(string, a, b)       printf(string, a, b)
#define MUSIM_EXTSTD_MSG3(string, a, b, c)    printf(string, a, b, c)
#define MUSIM_EXTSTD_MSG4(string, a, b, c, d) printf(string, a, b, c, d)
#else
#define MUSIM_EXTSTD_MSG0(string)
#define MUSIM_EXTSTD_MSG1(string, a)
#define MUSIM_EXTSTD_MSG2(string, a, b)
#define MUSIM_EXTSTD_MSG3(string, a, b, c)
#define MUSIM_EXTSTD_MSG4(string, a, b, c, d)
#endif

#if defined MUSIM_ERR_DEBUG
#define MUSIM_ERR_MSG0(string)                printf(string, __FILE__, __LINE__)
#define MUSIM_ERR_MSG1(string, a)             printf(string, __FILE__, __LINE__, a)
#define MUSIM_ERR_MSG2(string, a, b)          printf(string, __FILE__, __LINE__, a, b)
#define MUSIM_ERR_MSG3(string, a, b, c)       printf(string, __FILE__, __LINE__, a, b, c)
#define MUSIM_ERR_MSG4(string, a, b, c, d)    printf(string, __FILE__, __LINE__, a, b, c, d)
#else
#define MUSIM_ERR_MSG0(string)
#define MUSIM_ERR_MSG1(string, a)
#define MUSIM_ERR_MSG2(string, a, b)
#define MUSIM_ERR_MSG3(string, a, b, c)
#define MUSIM_ERR_MSG4(string, a, b, c, d)
#endif

#endif // _UTIL_H_
