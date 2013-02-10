/*
 * ReferenceFrameLayer.hpp
 *
 *  Created on: Feb 9, 2013
 *      Author: Timothy Morey
 */

#ifndef __REFERENCEFRAMELAYER_HPP__
#define __REFERENCEFRAMELAYER_HPP__


#include "nvn.h"
#include "Layer.hpp"


class ReferenceFrameLayer : public Layer
{
public:
  ReferenceFrameLayer(int dims);
  virtual ~ReferenceFrameLayer();

public:
  virtual NVN_BBox GetBounds() const { return _Bounds; }

public:
  virtual int Render();
  int SetBounds(NVN_BBox bounds);

protected:
  NVN_BBox _Bounds;
  int _NDims;
};


#endif
