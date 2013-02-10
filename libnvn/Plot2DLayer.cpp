/*
 * Plot2DLayer.cpp
 *
 *  Created on: Feb 8, 2013
 *      Author: Timothy Morey
 */

#include "nvn.h"
#include "variant.h"

#include "DataGrid.hpp"
#include "Plot2DLayer.hpp"

#include <GL/gl.h>
#include <stdlib.h>
#include <string.h>


Plot2DLayer::Plot2DLayer(DataGrid* x, DataGrid* y)
  : _X(x),
    _Y(y),
    _N(0),
    _DataType()
{
  if(_X && _Y)
  {
    _N = MIN(_X->GetDimLen(0), _Y->GetDimLen(0));
    _DataType = MPITypeToVariantType(_X->GetType());
  }

  _MinX = MaxVariant(_DataType);
  _MinY = MaxVariant(_DataType);
  _MaxX = MinVariant(_DataType);
  _MaxY = MinVariant(_DataType);

  Variant xv, yv;
  MPI_Offset i[MAX_DIMS];
  for(i[0] = 0; i[0] < _N; i[0]++)
  {
    _X->GetElemAsVariant(i, &xv);
    if(0 > VariantCompare(xv, _MinX)) _MinX = xv;
    if(0 < VariantCompare(xv, _MaxX)) _MaxX = xv;

    _Y->GetElemAsVariant(i, &yv);
    if(0 > VariantCompare(yv, _MinY)) _MinY = yv;
    if(0 < VariantCompare(yv, _MaxY)) _MaxY = yv;
  }
}

Plot2DLayer::~Plot2DLayer()
{

}

NVN_BBox Plot2DLayer::GetBounds() const
{
  NVN_BBox bounds = NVN_BBoxEmpty;

  bounds.Min[XDIM] = VariantValueAsFloat(_MinX);
  bounds.Min[YDIM] = VariantValueAsFloat(_MinY);

  bounds.Max[XDIM] = VariantValueAsFloat(_MaxX);
  bounds.Max[YDIM] = VariantValueAsFloat(_MaxY);

  return bounds;
}

int Plot2DLayer::Render()
{
  int retval = NVN_NOERR;

  // TODO: current rendering won't work well if there is a single point, and
  // won't be efficient if there are a lot of points.

  MPI_Offset i[MAX_DIMS];
  Variant x, y;

  glLineWidth(10.0f);
  glBegin(GL_LINE_STRIP);
  {
    for(i[0] = 0; i[0] < _N; i[0]++)
    {
      _X->GetElemAsVariant(i, &x);
      _Y->GetElemAsVariant(i, &y);

      glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
      glVertex3f(VariantValueAsFloat(x), VariantValueAsFloat(y), 0.0f);
    }
  }
  glEnd();

  return retval;
}

