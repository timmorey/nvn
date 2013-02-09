/*
 * ShadedSurfaceLayer.hpp
 *
 *  Created on: Feb 8, 2013
 *      Author: Timothy Morey
 */

#ifndef __SHADEDSURFACELAYER_HPP__
#define __SHADEDSURFACELAYER_HPP__


#include "DataGrid.hpp"
#include "Layer.hpp"


class ShadedSurfaceLayer : public Layer
{
public:
  ShadedSurfaceLayer(DataGrid* data);
  virtual ~ShadedSurfaceLayer();

public:
  ColorRamp& Ramp() { return _Ramp; }

public:
  virtual int GetWidth() const { return _DataGrid ? _DataGrid->GetDimLen(0) : -1; }
  virtual int GetHeight() const { return _DataGrid ? _DataGrid->GetDimLen(1) : -1; }
  virtual int GetDepth() const { return 0; }

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
