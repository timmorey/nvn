/*
 * ShadedSurfaceLayer.hpp
 *
 *  Created on: Feb 8, 2013
 *      Author: Timothy Morey
 */

#ifndef __SHADEDSURFACELAYER_HPP__
#define __SHADEDSURFACELAYER_HPP__


#include "color-ramp.h"
#include "variant.h"

#include "CartesianCRS.hpp"
#include "GridTransform.hpp"
#include "Layer.hpp"


class DataGrid;

class ShadedSurfaceLayer : public Layer
{
public:
  ShadedSurfaceLayer(DataGrid* data);
  virtual ~ShadedSurfaceLayer();

public:
  ColorRamp& Ramp() { return _Ramp; }

public:
  virtual NVN_BBox GetBounds() const;
  virtual const CRS& GetDataCRS() const;

public:
  virtual int Render();
  virtual int SetModelCRS(const CartesianCRS& crs);

protected:
  int DrawQuad(const MPI_Offset pt1[], const MPI_Offset pt2[],
               const MPI_Offset pt3[], const MPI_Offset pt4[],
               const GridTransform& transform) const;
  int DrawTriangle(const MPI_Offset pt1[], const MPI_Offset pt2[],
                   const MPI_Offset pt3[]) const;
  int DrawTriangle(float x1, float y1, float z1, int c1,
                   float x2, float y2, float z2, int c2,
                   float x3, float y3, float z3, int c3) const;

protected:
  DataGrid* _DataGrid;
  ColorRamp _Ramp;
  Variant _MinVal;
  Variant _MaxVal;
  char* _TexBitmap;
  int _TexWidth;
  int _TexHeight;
  unsigned int _TextureID;
  unsigned int _DisplayList;
  bool _Compiled;
};


#endif
