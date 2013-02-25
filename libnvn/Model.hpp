/**
	 Model.hpp - Created by Timothy Morey on 1/12/2013
 */

#ifndef __MODEL_HPP__
#define __MODEL_HPP__


#include "nvn.h"

#include "CartesianCRS.hpp"

#include <list>


class Layer;

class Model
{
public:
  Model();
  ~Model();

public:
  NVN_BBox GetBounds() const;
  const CartesianCRS& GetCRS() const { return _Crs; }
  int GetNDims() const;

public:
  int AddLayer(Layer* layer);
  int Render();

protected:
  std::list<Layer*> _Layers;
  CartesianCRS _Crs;
};

#endif
