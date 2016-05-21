#ifndef _MAIN_H_
#define _MAIN_H_

#include "common/types.h"
#include "dynamic_libs/os_functions.h"

/* Main */
#ifdef __cplusplus
extern "C" {
#endif

//! C wrapper for our C++ functions
int Menu_Main(void);

unsigned char* screenBuffer;
int screen_buf0_size = 0;
int screen_buf1_size = 0;


#ifdef __cplusplus
}
#endif

#endif
