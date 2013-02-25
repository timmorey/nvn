/**
	 nvn.h - Created by Timothy Morey on 1/11/2013
 */

#ifndef __NVN_H__
#define __NVN_H__

#ifdef __cplusplus
#define MPICH_SKIP_MPICXX 1

extern "C"
{
#endif


#include <mpi.h>

#include <stdint.h>
#include <stdlib.h>


/******************************************************************************
 * Constants:
 ******************************************************************************/

#define MAX_DIMS 4
#define MAX_NAME 64
#define MAX_PATH 256
#define MAX_ERRMSG 256

#define XDIM 0
#define YDIM 1
#define ZDIM 2
#define TDIM 3

#define DEG2RADF 0.0174532925f
#define EPSILONF 1e-6f
#define EPSILOND 1e-12


/******************************************************************************
 * Error codes:
 ******************************************************************************/

#define NVN_NOERR          0

#define NVN_ERROR          1
#define NVN_EINVARGS       2
#define NVN_ENULL          3
#define NVN_ENOTINIT       4
#define NVN_EGLXFAIL       5
#define NVN_EXWINFAIL      6
#define NVN_EINVTYPE       7
#define NVN_EQFULL         8
#define NVN_ENOTUNIQUE     9
#define NVN_EUNKFORMAT    10
#define NVN_ECONNFAIL     11
#define NVN_ETHREADFAIL   12
#define NVN_ECOMMFAIL     13
#define NVN_ECLIENTGONE   14

#define NVN_NUMERRS       15


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

typedef struct
{
  float Min[MAX_DIMS];
  float Max[MAX_DIMS];
} NVN_BBox;

 extern NVN_BBox NVN_BBoxEmpty;


/*****************************************************************************
 * Public interface functions
 *****************************************************************************/

NVN_Err NVN_AddLayer(NVN_Model model, NVN_Layer layer);

NVN_Err NVN_BBoxUnion(NVN_BBox b1, NVN_BBox b2, NVN_BBox* u);

NVN_Err NVN_CreateDataGrid(int ndims,
                           const MPI_Offset dimlen[],
                           MPI_Datatype type,
                           void* data,
                           NVN_DataGrid* grid);

NVN_Err NVN_CreateModel(NVN_Model* model);

NVN_Err NVN_Create2DPlotLayer(NVN_DataGrid x, NVN_DataGrid y, int color,
                              NVN_Layer* layer);

NVN_Err NVN_CreateShadedSurfaceLayer(NVN_DataGrid grid, NVN_Layer* layer);

NVN_Err NVN_CreateWindow(const char* title,
			             int x, int y,
		                 int width, int height,
		                 int borderless,
		                 NVN_Window* window);

NVN_Err NVN_DestroyWindow(NVN_Window window);

NVN_Err NVN_ErrMsg(NVN_Err err, char msg[], size_t len);

NVN_Err NVN_GetViewParms(NVN_Window window, float* centerx, float* centery,
                         float* zoomlevel, float* xrotation, float* zrotation);

NVN_Err NVN_LoadDataGrid(NVN_DataGridDescriptor desc, NVN_DataGrid* grid);

NVN_Err NVN_SetViewParms(NVN_Window window, float centerx, float centery,
                         float zoomlevel, float xrotation, float zrotation);

NVN_Err NVN_ShowModel(NVN_Window window, NVN_Model model);


/*****************************************************************************
 * Public interface predicates
 *****************************************************************************/

int NVN_BBoxIntersectsP(NVN_BBox b1, NVN_BBox b2);

int NVN_IsWindowActiveP(NVN_Window window);


/*****************************************************************************
 * Macros:
 *****************************************************************************/

#define MIN(x,y) x < y ? x : y
#define MAX(x,y) x > y ? x : y


#ifdef __cplusplus
}
#endif

#endif
