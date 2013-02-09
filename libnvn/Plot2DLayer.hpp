/*
 * Plot2DLayer.hpp
 *
 *  Created on: Feb 8, 2013
 *      Author: Timothy Morey
 */

#ifndef __PLOT2DLAYER_HPP__
#define __PLOT2DLAYER_HPP__


#include "variant.h"

#include "Layer.hpp"

#define MPICH_SKIP_MPICXX 1
#include <mpi.h>


class DataGrid;

class Plot2DLayer : public Layer
{
public:
  Plot2DLayer(DataGrid* x, DataGrid* y);
  virtual ~Plot2DLayer();

public:
  virtual float GetWidth() const;
  virtual float GetHeight() const;
  virtual float GetDepth() const;

public:
  virtual int Render();

protected:
  DataGrid* _X;
  DataGrid* _Y;
  int _N;
  VariantType _DataType;
  Variant _MinX, _MinY;
  Variant _MaxX, _MaxY;
};

#endif
