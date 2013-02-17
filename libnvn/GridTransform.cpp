/**
  GridTransform.cpp - Created by Timothy Morey on 1/27/2013
*/


#include "GridTransform.hpp"
#include "nvn.h"

#include <math.h>


GridTransform::GridTransform(const CRS& basecrs, const GridCRS& gridcrs)
  : _BaseCRS(basecrs),
    _GridCRS(gridcrs)
{

}

GridTransform::~GridTransform()
{

}

int GridTransform::GridToModel(const MPI_Offset posin[], float posout[]) const
{
  int retval = NVN_NOERR;
  char dimname[MAX_NAME];

  for(int i = 0; i < _GridCRS.GetNDims(); i++)
  {
    _GridCRS.GetDimName(i, dimname);
    int basedim = _BaseCRS.FindDim(dimname);
    if(basedim >= 0)
      posout[basedim] = (float)posin[i];
  }

  return retval;
}

int GridTransform::ModelToGrid(const float posin[], float posout[]) const
{
  int retval = NVN_NOERR;
  char dimname[MAX_NAME];

  for(int i = 0; i < _BaseCRS.GetNDims(); i++)
  {
    _BaseCRS.GetDimName(i, dimname);
    int griddim = _GridCRS.FindDim(dimname);
    if(griddim >= 0)
      posout[griddim] = posin[i];
  }

  return retval;
}

int GridTransform::ModelToGrid(const float posin[], MPI_Offset posout[]) const
{
  int retval = NVN_NOERR;
  char dimname[MAX_NAME];

  for(int i = 0; i < _BaseCRS.GetNDims(); i++)
  {
    _BaseCRS.GetDimName(i, dimname);
    int griddim = _GridCRS.FindDim(dimname);
    if(griddim >= 0)
      posout[griddim] = roundf(posin[i]);
  }

  return retval;
}
