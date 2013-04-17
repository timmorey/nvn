/*
 * GlacierLayer.hpp
 *
 *  Created on: Apr 16, 2013
 *      Author: Timothy Morey
 */

#ifndef __GLACIERLAYER_HPP__
#define __GLACIERLAYER_HPP__


#include "color-ramp.h"
#include "nvn.h"
#include "variant.h"

#include "GridTransform.hpp"
#include "Layer.hpp"


class DataGrid;

class GlacierLayer : public Layer
{
public:
  GlacierLayer(DataGrid* topg, DataGrid* usurf);
  virtual ~GlacierLayer();

public:
  ColorRamp& Ramp() { return _Ramp; }

public:
  virtual NVN_BBox GetBounds() const;
  virtual const CRS& GetDataCRS() const;

public:
  virtual int Render();
  virtual int SetModelCRS(const CartesianCRS& crs);

protected:
  int DrawIceQuad(const MPI_Offset pt1[], const MPI_Offset pt2[],
                  const MPI_Offset pt3[], const MPI_Offset pt4[],
                  const GridTransform& transform) const;
  int DrawLandQuad(const MPI_Offset pt1[], const MPI_Offset pt2[],
                   const MPI_Offset pt3[], const MPI_Offset pt4[],
                   const GridTransform& transform) const;
  int DrawTriangle(float x1, float y1, float z1, int c1,
                   float x2, float y2, float z2, int c2,
                   float x3, float y3, float z3, int c3) const;

protected:
  DataGrid* _TopgGrid;
  DataGrid* _UsurfGrid;
  Variant _MinVal;
  Variant _MaxVal;
  ColorRamp _Ramp;
  unsigned int _DisplayList;
  bool _Compiled;
};

#endif
