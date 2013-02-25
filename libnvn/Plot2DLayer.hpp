/*
 * Plot2DLayer.hpp
 *
 *  Created on: Feb 8, 2013
 *      Author: Timothy Morey
 */

#ifndef __PLOT2DLAYER_HPP__
#define __PLOT2DLAYER_HPP__


#include "variant.h"

#include "CartesianCRS.hpp"
#include "Layer.hpp"

#define MPICH_SKIP_MPICXX 1
#include <mpi.h>


class DataGrid;

class Plot2DLayer : public Layer
{
public:
  Plot2DLayer(DataGrid* x, DataGrid* y, int color);
  virtual ~Plot2DLayer();

public:
  virtual NVN_BBox GetBounds() const;
  virtual const CRS& GetDataCRS() const { return _DataCrs; }

public:
  virtual int Render();
  virtual int SetModelCRS(const CartesianCRS& crs);

protected:
  DataGrid* _X;
  DataGrid* _Y;
  int _Color;
  int _N;
  VariantType _DataType;
  Variant _MinX, _MinY;
  Variant _MaxX, _MaxY;
  CartesianCRS _DataCrs;
};

#endif
