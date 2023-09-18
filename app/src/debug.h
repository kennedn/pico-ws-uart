#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdio.h>

#ifdef ENABLE_DEBUG
#define DEBUG(...) printf("%s: ", __func__), printf(__VA_ARGS__), printf("\n")
#else
#define DEBUG(...) do {} while(0)
#endif


#endif
