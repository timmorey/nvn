/**
	 Layer.hpp - Created by Timothy Morey on 1/12/2013
 */

#ifndef __LAYER_HPP__
#define __LAYER_HPP__


#include "nvn.h"

#include "CRS.hpp"


class Layer
{
protected:
  Layer() {};

public:
  virtual ~Layer() {};

public:
  virtual int Render() = 0;

public:
  virtual NVN_BBox GetBounds() const = 0;
  virtual const CRS& GetCRS() const = 0;

};

#endif
