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
  virtual float GetWidth() const;
  virtual float GetHeight() const;
  virtual float GetDepth() const;

public:
  virtual int Render();

protected:
  int DrawTriangle(MPI_Offset pt1[], MPI_Offset pt2[], MPI_Offset pt3[]) const;

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
