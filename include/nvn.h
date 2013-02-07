/**
	 nvn.h - Created by Timothy Morey on 1/11/2013
 */

#ifndef __NVN_H__
#define __NVN_H__

#ifdef __cplusplus
extern "C"
{
#endif


#define MPICH_SKIP_MPICXX 1
#include <mpi.h>

#include <stdint.h>


/******************************************************************************
 * Constants:
 ******************************************************************************/

#define MAX_DIMS 4
#define MAX_NAME 64
#define MAX_PATH 256

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
#define NVN_EUNKFORMAT     9


/*****************************************************************************
 * Public interface types
 *****************************************************************************/

typedef int NVN_Err;
typedef intptr_t NVN_DataGrid;
typedef intptr_t NVN_Layer;
typedef intptr_t NVN_Model;
typedef intptr_t NVN_Window;

typedef struct
{
  char Filename[MAX_PATH];
  char Varname[MAX_NAME];
  MPI_Offset Start[MAX_DIMS];
  MPI_Offset Count[MAX_DIMS];
} NVN_DataGridDescriptor;


/*****************************************************************************
 * Public interface functions
 *****************************************************************************/

NVN_Err NVN_AddLayer(NVN_Model model, NVN_Layer layer);

NVN_Err NVN_CreateModel(NVN_Model* model);

NVN_Err NVN_CreateShadedSurfaceGridLayer(NVN_DataGrid grid, NVN_Layer* layer);

NVN_Err NVN_CreateWindow(const char* title,
			             int x, int y,
		                 int width, int height,
		                 int borderless,
		                 NVN_Window* window);

NVN_Err NVN_DestroyWindow(NVN_Window window);

NVN_Err NVN_LoadDataGrid(NVN_DataGridDescriptor desc, NVN_DataGrid* grid);

NVN_Err NVN_ShowModel(NVN_Window window, NVN_Model model);


/*****************************************************************************
 * Public interface predicates
 *****************************************************************************/

int NVN_IsWindowActiveP(NVN_Window window);


#ifdef __cplusplus
}
#endif

#endif
