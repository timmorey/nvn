/*
 * ReferenceFrameLayer.hpp
 *
 *  Created on: Feb 9, 2013
 *      Author: Timothy Morey
 */

#ifndef __REFERENCEFRAMELAYER_HPP__
#define __REFERENCEFRAMELAYER_HPP__


#include "nvn.h"

#include "CartesianCRS.hpp"
#include "Layer.hpp"


class ReferenceFrameLayer : public Layer
{
public:
  ReferenceFrameLayer(const CartesianCRS& crs, NVN_BBox bounds);
  virtual ~ReferenceFrameLayer();

public:
  virtual NVN_BBox GetBounds() const { return _Bounds; }
  virtual const CRS& GetDataCRS() const { return _ModelCrs; }

public:
  virtual int Render();
  int SetBounds(NVN_BBox bounds);
  int SetModelCRS(const CartesianCRS& crs);

protected:
  NVN_BBox _Bounds;
};


#endif
