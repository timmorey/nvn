/*
 * ReferenceFrameLayer.cpp
 *
 *  Created on: Feb 9, 2013
 *      Author: Timothy Morey
 */


#include "nvn.h"

#include "CartesianCRS.hpp"
#include "ReferenceFrameLayer.hpp"

#include <GL/gl.h>
#include <string.h>


ReferenceFrameLayer::ReferenceFrameLayer(const CartesianCRS& crs, NVN_BBox bounds)
  : _Bounds(bounds)
{
  this->SetModelCRS(crs);
}

ReferenceFrameLayer::~ReferenceFrameLayer()
{

}

int ReferenceFrameLayer::Render()
{
  int retval = NVN_NOERR;

  glLineWidth(1.0f);
  glBegin(GL_LINES);
  {
    glColor4f(0.0f, 1.0f, 0.0f, 1.0f);

    if(2 <= _ModelCrs.GetNDims())
    {
      glVertex3f(_Bounds.Min[XDIM], _Bounds.Min[YDIM], _Bounds.Max[ZDIM]);
      glVertex3f(_Bounds.Min[XDIM], _Bounds.Max[YDIM], _Bounds.Max[ZDIM]);

      glVertex3f(_Bounds.Min[XDIM], _Bounds.Max[YDIM], _Bounds.Max[ZDIM]);
      glVertex3f(_Bounds.Max[XDIM], _Bounds.Max[YDIM], _Bounds.Max[ZDIM]);

      glVertex3f(_Bounds.Max[XDIM], _Bounds.Max[YDIM], _Bounds.Max[ZDIM]);
      glVertex3f(_Bounds.Max[XDIM], _Bounds.Min[YDIM], _Bounds.Max[ZDIM]);

      glVertex3f(_Bounds.Max[XDIM], _Bounds.Min[YDIM], _Bounds.Max[ZDIM]);
      glVertex3f(_Bounds.Min[XDIM], _Bounds.Min[YDIM], _Bounds.Max[ZDIM]);
    }

    if(3 <= _ModelCrs.GetNDims())
    {
      glVertex3f(_Bounds.Min[XDIM], _Bounds.Min[YDIM], _Bounds.Min[ZDIM]);
      glVertex3f(_Bounds.Min[XDIM], _Bounds.Max[YDIM], _Bounds.Min[ZDIM]);

      glVertex3f(_Bounds.Min[XDIM], _Bounds.Max[YDIM], _Bounds.Min[ZDIM]);
      glVertex3f(_Bounds.Max[XDIM], _Bounds.Max[YDIM], _Bounds.Min[ZDIM]);

      glVertex3f(_Bounds.Max[XDIM], _Bounds.Max[YDIM], _Bounds.Min[ZDIM]);
      glVertex3f(_Bounds.Max[XDIM], _Bounds.Min[YDIM], _Bounds.Min[ZDIM]);

      glVertex3f(_Bounds.Max[XDIM], _Bounds.Min[YDIM], _Bounds.Min[ZDIM]);
      glVertex3f(_Bounds.Min[XDIM], _Bounds.Min[YDIM], _Bounds.Min[ZDIM]);

      glVertex3f(_Bounds.Min[XDIM], _Bounds.Min[YDIM], _Bounds.Min[ZDIM]);
      glVertex3f(_Bounds.Min[XDIM], _Bounds.Min[YDIM], _Bounds.Max[ZDIM]);

      glVertex3f(_Bounds.Max[XDIM], _Bounds.Min[YDIM], _Bounds.Min[ZDIM]);
      glVertex3f(_Bounds.Max[XDIM], _Bounds.Min[YDIM], _Bounds.Max[ZDIM]);

      glVertex3f(_Bounds.Max[XDIM], _Bounds.Max[YDIM], _Bounds.Min[ZDIM]);
      glVertex3f(_Bounds.Max[XDIM], _Bounds.Max[YDIM], _Bounds.Max[ZDIM]);

      glVertex3f(_Bounds.Min[XDIM], _Bounds.Max[YDIM], _Bounds.Min[ZDIM]);
      glVertex3f(_Bounds.Min[XDIM], _Bounds.Max[YDIM], _Bounds.Max[ZDIM]);
    }
  }
  glEnd();

  return retval;
}

int ReferenceFrameLayer::SetBounds(NVN_BBox bounds)
{
  int retval = NVN_NOERR;
  _Bounds = bounds;
  return retval;
}

int ReferenceFrameLayer::SetModelCRS(const CartesianCRS& crs)
{
  int retval = NVN_NOERR;
  _ModelCrs = crs;
  return retval;
}
