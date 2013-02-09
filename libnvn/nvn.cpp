/**
 * nvn.cpp - Created by Timothy Morey on 2/6/2013
 */


#include "nvn.h"

#include "gl-window.hpp"
#include "ShadedSurfaceLayer.hpp"
#include "Loader.hpp"
#include "model.hpp"


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
    m->TheLayer = l;
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
