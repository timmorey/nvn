/**
	 nvn.h - Created by Timothy Morey on 1/11/2013
 */

#ifndef __NVN_H__
#define __NVN_H__

#ifdef __cplusplus
extern "C"
{
#endif


#include <stdint.h>


/******************************************************************************
 * Constants:
 ******************************************************************************/

#define MAX_DIMS 4
#define MAX_NAME 64

#define DEG2RADF 0.0174532925f


/******************************************************************************
 * Error codes:
 ******************************************************************************/

#define NVN_NOERR 0

#define NVN_ERROR         -1
#define NVN_EINVARGS       1
#define NVN_ENULL          2
#define NVN_ENOTINIT       3
#define NVN_EGLXFAIL       4
#define NVN_EXWINFAIL      5
#define NVN_EINVTYPE       6
#define NVN_EQFULL         7
#define NVN_ENOTUNIQUE     8


/*****************************************************************************
 * Public interface
 *****************************************************************************/

typedef int NVN_Err;
typedef intptr_t NVN_Window;

NVN_Err NVN_CreateWindow(const char* title,
			             int x, int y,
		                 int width, int height,
		                 int borderless,
		                 NVN_Window* window);

NVN_Err NVN_DestroyWindow(NVN_Window window);

#ifdef __cplusplus
}
#endif

#endif
