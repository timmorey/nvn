/*
 * GlacierLayer.cpp
 *
 *  Created on: Apr 16, 2013
 *      Author: Timothy Morey
 */


#include "color-ramp.h"
#include "nvn.h"
#include "variant.h"

#include "DataGrid.hpp"
#include "GlacierLayer.hpp"

#define MPICH_SKIP_MPICXX 1
#include <mpi.h>

#include <math.h>

#include <GL/gl.h>
#include <GL/glu.h>


GlacierLayer::GlacierLayer(DataGrid* topg, DataGrid* usurf)
  : _TopgGrid(topg),
    _UsurfGrid(usurf),
    _Ramp(DefaultColorRamp),
    _DisplayList(0),
    _Compiled(false)
{
  if(_TopgGrid && _UsurfGrid)
  {
    Variant value;
    MPI_Offset pos[MAX_DIMS];
    _MinVal = MaxVariant(MPITypeToVariantType(_TopgGrid->GetType()));
    _MaxVal = MinVariant(MPITypeToVariantType(_TopgGrid->GetType()));
    for(pos[XDIM] = 0; pos[XDIM] < _TopgGrid->GetDimLen(XDIM); pos[XDIM]++)
    {
      for(pos[YDIM] = 0; pos[YDIM] < _TopgGrid->GetDimLen(YDIM); pos[YDIM]++)
      {
        if(_TopgGrid->HasData(pos))
        {
          _TopgGrid->GetElemAsVariant(pos, &value);
          if(0 > VariantCompare(value, _MinVal)) _MinVal = value;
          if(0 < VariantCompare(value, _MaxVal)) _MaxVal = value;
        }

        if(_UsurfGrid->HasData(pos))
        {
          _UsurfGrid->GetElemAsVariant(pos, &value);
          if(0 < VariantCompare(value, _MaxVal)) _MaxVal = value;
        }
      }
    }

    printf("min=%f, max=%f\n",
           VariantValueAsDouble(_MinVal), VariantValueAsDouble(_MaxVal));
  }
}

GlacierLayer::~GlacierLayer()
{

}

NVN_BBox GlacierLayer::GetBounds() const
{
  NVN_BBox bounds = NVN_BBoxEmpty;

  GridTransform transform(_ModelCrs, _TopgGrid->GetCRS());

  MPI_Offset minpti[MAX_DIMS], maxpti[MAX_DIMS];

  minpti[XDIM] = 0;
  minpti[YDIM] = 0;

  maxpti[XDIM] = _TopgGrid->GetDimLen(0) - 1;
  maxpti[YDIM] = _TopgGrid->GetDimLen(1) - 1;

  transform.GridToModel(minpti, bounds.Min);
  transform.GridToModel(maxpti, bounds.Max);

  bounds.Min[ZDIM] = VariantValueAsFloat(_MinVal) / 100.0f;
  bounds.Max[ZDIM] = VariantValueAsFloat(_MaxVal) / 100.0f;

  return bounds;
}

const CRS& GlacierLayer::GetDataCRS() const
{
  if(_TopgGrid)
    return _TopgGrid->GetCRS();
  else
    return _ModelCrs;
}

int GlacierLayer::Render()
{
  if(! _Compiled)
  {
    if(_DisplayList)
    {
      glDeleteLists(_DisplayList, 1);
      _DisplayList = 0;
    }

    int datawidth = _TopgGrid->GetDimLen(0);
    int dataheight = _TopgGrid->GetDimLen(1);
    MPI_Offset nw[MAX_DIMS], ne[MAX_DIMS], se[MAX_DIMS], sw[MAX_DIMS];
    MPI_Offset i[MAX_DIMS];
    Variant value;
    int color;
    GridTransform transform(_ModelCrs, _TopgGrid->GetCRS());

    _DisplayList = glGenLists(1);

    glNewList(_DisplayList, GL_COMPILE);
    {
      glBegin(GL_TRIANGLES);
      {
        for(sw[0] = 0, se[0] = 1, ne[0] = 1, nw[0] = 0;
            se[0] < datawidth && ne[0] < datawidth;
            sw[0]++, se[0]++, ne[0]++, nw[0]++)
        {
          for(sw[1] = 0, se[1] = 0, ne[1] = 1, nw[1] = 1;
              se[1] < dataheight && ne[1] < dataheight;
              sw[1]++, se[1]++, ne[1]++, nw[1]++)
          {
            if(_TopgGrid->HasData(ne) &&
               _TopgGrid->HasData(se) &&
               _TopgGrid->HasData(sw) &&
               _TopgGrid->HasData(nw))
            {
              this->DrawLandQuad(nw, ne, se, sw, transform);
            }
          }
        }

        for(sw[0] = 0, se[0] = 1, ne[0] = 1, nw[0] = 0;
            se[0] < datawidth && ne[0] < datawidth;
            sw[0]++, se[0]++, ne[0]++, nw[0]++)
        {
          for(sw[1] = 0, se[1] = 0, ne[1] = 1, nw[1] = 1;
              se[1] < dataheight && ne[1] < dataheight;
              sw[1]++, se[1]++, ne[1]++, nw[1]++)
          {
            if(_UsurfGrid->HasData(ne) &&
               _UsurfGrid->HasData(se) &&
               _UsurfGrid->HasData(sw) &&
               _UsurfGrid->HasData(nw))
            {
              this->DrawIceQuad(nw, ne, se, sw, transform);
            }
          }
        }
      }
      glEnd();
    }
    glEndList();

    _Compiled = true;
  }

  glCallList(_DisplayList);

  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  NVN_BBox bounds = this->GetBounds();
  int watercolor = 0x66FF0000;
  glBegin(GL_TRIANGLES);
  {
    this->DrawTriangle(bounds.Min[0], bounds.Min[1], 0.0f, watercolor,
                       bounds.Min[0], bounds.Max[1], 0.0f, watercolor,
                       bounds.Max[0], bounds.Max[1], 0.0f, watercolor);
    this->DrawTriangle(bounds.Max[0], bounds.Max[1], 0.0f, watercolor,
                       bounds.Max[0], bounds.Min[1], 0.0f, watercolor,
                       bounds.Min[0], bounds.Min[1], 0.0f, watercolor);
  }
  glEnd();
}

int GlacierLayer::SetModelCRS(const CartesianCRS& crs)
{
  int retval = NVN_NOERR;
  _ModelCrs = crs;
  _Compiled = false;
  return retval;
}

int GlacierLayer::DrawIceQuad(const MPI_Offset pt1[], const MPI_Offset pt2[],
                              const MPI_Offset pt3[], const MPI_Offset pt4[],
                              const GridTransform& transform) const
{
  int retval = NVN_NOERR;
  Variant v1, v2, v3, v4;
  float p1[MAX_DIMS], p2[MAX_DIMS], p3[MAX_DIMS], p4[MAX_DIMS];
  int color;

  _UsurfGrid->GetElemAsVariant(pt1, &v1);
  _UsurfGrid->GetElemAsVariant(pt2, &v2);
  _UsurfGrid->GetElemAsVariant(pt3, &v3);
  _UsurfGrid->GetElemAsVariant(pt4, &v4);

  color = 0xBBFFFFFF;

  transform.GridToModel(pt1, p1);
  p1[ZDIM] = VariantValueAsFloat(v1) / 100.0f;

  transform.GridToModel(pt2, p2);
  p2[ZDIM] = VariantValueAsFloat(v2) / 100.0f;

  transform.GridToModel(pt3, p3);
  p3[ZDIM] = VariantValueAsFloat(v3) / 100.0f;

  transform.GridToModel(pt4, p4);
  p4[ZDIM] = VariantValueAsFloat(v4) / 100.0f;

  if(fabs(p1[ZDIM] - p3[ZDIM]) < fabs(p2[ZDIM] - p4[ZDIM]))
  {
    this->DrawTriangle(p1[XDIM], p1[YDIM], p1[ZDIM], color,
                       p2[XDIM], p2[YDIM], p2[ZDIM], color,
                       p3[XDIM], p3[YDIM], p3[ZDIM], color);
    this->DrawTriangle(p3[XDIM], p3[YDIM], p3[ZDIM], color,
                       p4[XDIM], p4[YDIM], p4[ZDIM], color,
                       p1[XDIM], p1[YDIM], p1[ZDIM], color);
  }
  else
  {
    this->DrawTriangle(p2[XDIM], p2[YDIM], p2[ZDIM], color,
                       p3[XDIM], p3[YDIM], p3[ZDIM], color,
                       p4[XDIM], p4[YDIM], p4[ZDIM], color);
    this->DrawTriangle(p4[XDIM], p4[YDIM], p4[ZDIM], color,
                       p1[XDIM], p1[YDIM], p1[ZDIM], color,
                       p2[XDIM], p2[YDIM], p2[ZDIM], color);
  }

  return retval;
}

int GlacierLayer::DrawLandQuad(const MPI_Offset pt1[], const MPI_Offset pt2[],
                               const MPI_Offset pt3[], const MPI_Offset pt4[],
                               const GridTransform& transform) const
{
  int retval = NVN_NOERR;
  Variant v1, v2, v3, v4;
  float p1[MAX_DIMS], p2[MAX_DIMS], p3[MAX_DIMS], p4[MAX_DIMS];
  int c1, c2, c3, c4;

  _TopgGrid->GetElemAsVariant(pt1, &v1);
  _TopgGrid->GetElemAsVariant(pt2, &v2);
  _TopgGrid->GetElemAsVariant(pt3, &v3);
  _TopgGrid->GetElemAsVariant(pt4, &v4);

  c1 = GetColor(_Ramp, v1, _MinVal, _MaxVal);
  c2 = GetColor(_Ramp, v2, _MinVal, _MaxVal);
  c3 = GetColor(_Ramp, v3, _MinVal, _MaxVal);
  c4 = GetColor(_Ramp, v4, _MinVal, _MaxVal);

  transform.GridToModel(pt1, p1);
  p1[ZDIM] = VariantValueAsFloat(v1) / 100.0f;

  transform.GridToModel(pt2, p2);
  p2[ZDIM] = VariantValueAsFloat(v2) / 100.0f;

  transform.GridToModel(pt3, p3);
  p3[ZDIM] = VariantValueAsFloat(v3) / 100.0f;

  transform.GridToModel(pt4, p4);
  p4[ZDIM] = VariantValueAsFloat(v4) / 100.0f;

  if(fabs(p1[ZDIM] - p3[ZDIM]) < fabs(p2[ZDIM] - p4[ZDIM]))
  {
    this->DrawTriangle(p1[XDIM], p1[YDIM], p1[ZDIM], c1,
                       p2[XDIM], p2[YDIM], p2[ZDIM], c2,
                       p3[XDIM], p3[YDIM], p3[ZDIM], c3);
    this->DrawTriangle(p3[XDIM], p3[YDIM], p3[ZDIM], c3,
                       p4[XDIM], p4[YDIM], p4[ZDIM], c4,
                       p1[XDIM], p1[YDIM], p1[ZDIM], c1);
  }
  else
  {
    this->DrawTriangle(p2[XDIM], p2[YDIM], p2[ZDIM], c2,
                       p3[XDIM], p3[YDIM], p3[ZDIM], c3,
                       p4[XDIM], p4[YDIM], p4[ZDIM], c4);
    this->DrawTriangle(p4[XDIM], p4[YDIM], p4[ZDIM], c4,
                       p1[XDIM], p1[YDIM], p1[ZDIM], c1,
                       p2[XDIM], p2[YDIM], p2[ZDIM], c2);
  }

  return retval;
}

int GlacierLayer::DrawTriangle(float x1, float y1, float z1, int c1,
                               float x2, float y2, float z2, int c2,
                               float x3, float y3, float z3, int c3) const
{
  int retval = NVN_NOERR;
  float ux, uy, uz;
  float vx, vy, vz;

  ux = x2 - x1;  vx = x3 - x1;
  uy = y2 - y1;  vy = y3 - y1;
  uz = z2 - z1;  vz = z3 - z1;

  glNormal3f(uy*vz - uz*vy,
             uz*vx - ux*vz,
             ux*vy - uy*vx);

  glColor4ub(GetR(c1), GetG(c1), GetB(c1), GetA(c1));
  glVertex3f(x1, y1, z1);

  glColor4ub(GetR(c2), GetG(c2), GetB(c2), GetA(c2));
  glVertex3f(x2, y2, z2);

  glColor4ub(GetR(c3), GetG(c3), GetB(c3), GetA(c3));
  glVertex3f(x3, y3, z3);
}
