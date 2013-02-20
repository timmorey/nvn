/**
 * nvn.cpp - Created by Timothy Morey on 2/6/2013
 */


#include "nvn.h"

#include "DataGrid.hpp"
#include "gl-window.hpp"
#include "Loader.hpp"
#include "Model.hpp"
#include "Plot2DLayer.hpp"
#include "ShadedSurfaceLayer.hpp"

#include <float.h>
#include <string.h>


const char* g_ErrMsg[NVN_NUMERRS] =
{
  "No error",
  "Unspecified error",
  "Invalid arguments",
  "Unexpected null pointer",
  "Not initialized",
  "GLX Error",
  "X11 Error",
  "Invalid type",
  "Queue full",
  "Not unique",
  "Unknown format",
  "Failed to establish network connection",
  "Failed to start thread",
  "Socket communication error"
};

NVN_BBox NVN_BBoxEmpty =
{
  { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX },
  { FLT_MIN, FLT_MIN, FLT_MIN, FLT_MIN }
};


/*****************************************************************************
 * Public function implementations
 *****************************************************************************/

extern "C" NVN_Err NVN_AddLayer(NVN_Model model, NVN_Layer layer)
{
  NVN_Err retval = NVN_NOERR;

  if(model && layer)
  {
    Model* m = (Model*)model;
    Layer* l = (Layer*)layer;
    m->AddLayer(l);
  }
  else
  {
    retval = NVN_EINVARGS;
  }

  return retval;
}

extern "C" NVN_Err NVN_BBoxUnion(NVN_BBox b1, NVN_BBox b2, NVN_BBox* u)
{
  NVN_Err retval = NVN_NOERR;

  if(u)
  {

    for(int i = 0; i < MAX_DIMS; i++)
    {
      if(b1.Min[i] < b1.Max[i] || b2.Min[i] < b2.Max[i])
      {
        u->Min[i] = MIN(b1.Min[i], b2.Min[i]);
        u->Max[i] = MAX(b1.Max[i], b2.Max[i]);
      }
      else
      {
        u->Min[i] = FLT_MAX;
        u->Max[i] = FLT_MIN;
      }
    }
  }
  else
  {
    retval = NVN_EINVARGS;
  }

  return retval;
}

extern "C" NVN_Err NVN_CreateDataGrid(int ndims,
                                      const MPI_Offset dimlen[],
                                      MPI_Datatype type,
                                      void* data,
                                      NVN_DataGrid* grid)
{
  NVN_Err retval = NVN_NOERR;

  if(grid && data)
  {
    *grid = (NVN_DataGrid)new DataGrid(ndims, dimlen, type, data);
  }
  else
  {
    retval = NVN_EINVARGS;
  }

  return retval;
}

extern "C" NVN_Err NVN_CreateModel(NVN_Model* model)
{
  NVN_Err retval = NVN_NOERR;

  if(model)
  {
    *model = (NVN_Model)new Model();
  }
  else
  {
    retval = NVN_EINVARGS;
  }

  return retval;
}

extern "C" NVN_Err NVN_CreatePlot2DLayer(NVN_DataGrid x, NVN_DataGrid y, int color,
                                         NVN_Layer* layer)
{
  NVN_Err retval = NVN_NOERR;

  if(layer)
  {
    *layer = (NVN_Layer)new Plot2DLayer((DataGrid*)x, (DataGrid*)y, color);
  }
  else
  {
    retval = NVN_EINVARGS;
  }

  return retval;
}

extern "C" NVN_Err NVN_CreateShadedSurfaceLayer(NVN_DataGrid grid, NVN_Layer* layer)
{
  NVN_Err retval = NVN_NOERR;

  if(grid && layer)
  {
    DataGrid* g = (DataGrid*)grid;
    *layer = (NVN_Layer)new ShadedSurfaceLayer(g);
  }
  else
  {
    retval = NVN_EINVARGS;
  }

  return retval;
}

extern "C" NVN_Err NVN_CreateWindow(const char* title,
                                    int x, int y,
                                    int width, int height,
                                    int borderless,
                                    NVN_Window* window)
{
  NVN_Err retval = NVN_NOERR;

  if(window)
  {
    *window = (NVN_Window)new GLWindow(title, x, y,
        width, height, (bool)borderless);
  }
  else
  {
    retval = NVN_EINVARGS;
  }

  return retval;
}

extern "C" NVN_Err NVN_DestroyWindow(NVN_Window window)
{
  NVN_Err retval = NVN_NOERR;

  if(window)
  {
    GLWindow* w = (GLWindow*)window;
    w->CloseWindow();
    delete w;
  }

  return retval;
}

extern "C" NVN_Err NVN_ErrMsg(NVN_Err err, char msg[], size_t len)
{
  NVN_Err retval = NVN_NOERR;

  if(err >= 0 && err < NVN_NUMERRS)
  {
    strncpy(msg, g_ErrMsg[err], len - 1);
  }
  else
  {
    retval = NVN_EINVARGS;
  }

  return retval;
}

extern "C" NVN_Err NVN_GetViewParms(NVN_Window window, float* centerx, float* centery,
                                    float* zoomlevel, float* xrotation, float* zrotation)
{
  NVN_Err retval = NVN_NOERR;

  if(window)
  {
    GLWindow* w = (GLWindow*)window;
    retval = w->GetViewParms(centerx, centery, zoomlevel, xrotation, zrotation);
  }
  else
  {
    retval = NVN_EINVARGS;
  }

  return retval;
}

extern "C" NVN_Err NVN_LoadDataGrid(NVN_DataGridDescriptor desc, NVN_DataGrid* grid)
{
  NVN_Err retval = NVN_NOERR;

  if(grid)
  {
    FileFormat format = FileFormatUnknown;
    DetermineFileFormat(desc.Filename, &format);
    DataGrid* g = 0;
    switch(format)
    {
    case FileFormatCDF1:
    case FileFormatCDF2:
    case FileFormatCDF5:
      LoadPNetCDFGrid(desc.Filename, desc.Varname, desc.Start, desc.Count, &g);
      break;
    case FileFormatCReSISGrid:
      LoadCReSISASCIIGrid(desc.Filename, &g);
      break;
    default:
      retval = NVN_EUNKFORMAT;
      break;
    }

    *grid = (NVN_DataGrid)g;
  }
  else
  {
    retval = NVN_EINVARGS;
  }

  return retval;
}

extern "C" NVN_Err NVN_SetViewParms(NVN_Window window, float centerx, float centery,
                                    float zoomlevel, float xrotation, float zrotation)
{
  NVN_Err retval = NVN_NOERR;

  if(window)
  {
    GLWindow* w = (GLWindow*)window;
    retval = w->SetViewParms(centerx, centery, zoomlevel, xrotation, zrotation);
  }
  else
  {
    retval = NVN_EINVARGS;
  }

  return retval;
}

extern "C" NVN_Err NVN_ShowModel(NVN_Window window, NVN_Model model)
{
  NVN_Err retval = NVN_NOERR;

  if(window && model)
  {
    GLWindow* w = (GLWindow*)window;
    Model* m = (Model*)model;
    w->ShowModel(m);
  }
  else
  {
    retval = NVN_EINVARGS;
  }

  return retval;
}


/*****************************************************************************
 * Public predicate implementations
 *****************************************************************************/

extern "C" int NVV_BBoxIntersectsP(NVN_BBox b1, NVN_BBox b2)
{
  int intersects = 0;

  // First check to see that b1 and b2 both have extent in some dimension
  for(int i = 0; i < MAX_DIMS && !intersects; i++)
  {
    intersects = b1.Min[i] < b1.Max[i] && b2.Min[i] < b2.Max[i];
  }

  // Check to see if they overlap in the dimensions they both occupy
  for(int i = 0; i < MAX_DIMS && intersects; i++)
  {
    intersects = (!(b1.Min[i] > b2.Max[i] || b1.Max[i] < b2.Min[i]));
  }

  return intersects;
}

extern "C" int NVN_IsWindowActiveP(NVN_Window window)
{
  int active = 0;

  if(window)
  {
    GLWindow* w = (GLWindow*)window;
    active = w->IsActive();
  }

  return active;
}
