/**
	 Layer.hpp - Created by Timothy Morey on 1/12/2013
 */

#ifndef __LAYER_HPP__
#define __LAYER_HPP__


#include "color-ramp.h"
#include "variant.h"

#define MPICH_SKIP_MPICXX 1
#include <mpi.h>


class Layer
{
protected:
  Layer() {};

public:
  virtual ~Layer() {};

public:
  virtual int Render() = 0;

public:
  virtual int GetWidth() const = 0;
  virtual int GetHeight() const = 0;
  virtual int GetDepth() const = 0;

};

#endif
