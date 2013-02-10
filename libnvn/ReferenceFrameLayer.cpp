/*
 * ReferenceFrameLayer.cpp
 *
 *  Created on: Feb 9, 2013
 *      Author: Timothy Morey
 */


#include "nvn.h"

#include "ReferenceFrameLayer.hpp"

#include <GL/gl.h>
#include <string.h>


ReferenceFrameLayer::ReferenceFrameLayer(int ndims)
  : _Bounds(NVN_BBoxEmpty),
    _NDims(ndims)
{
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

    if(2 <= _NDims)
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

    if(3 <= _NDims)
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
