/**
	 Layer.hpp - Created by Timothy Morey on 1/12/2013
 */

#ifndef __LAYER_HPP__
#define __LAYER_HPP__


#include "nvn.h"

#include "CartesianCRS.hpp"


class Layer
{
protected:
  Layer() : _ModelCrs(4) {};

public:
  virtual ~Layer() {};

public:
  virtual int Render() = 0;
  virtual int SetModelCRS(const CartesianCRS& crs) = 0;

public:
  virtual NVN_BBox GetBounds() const = 0;
  virtual const CRS& GetDataCRS() const = 0;
  virtual const CartesianCRS& GetModelCRS() const { return _ModelCrs; }

protected:
  CartesianCRS _ModelCrs;

};

#endif
