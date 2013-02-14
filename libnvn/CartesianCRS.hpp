/**
  CartesianCRS.hpp - Created by Timothy Morey on 1/27/2013
*/

#ifndef __CARTESIAN_CRS_HPP__
#define __CARTESIAN_CRS_HPP__


#include "nvn.h"

#include "CRS.hpp"


class CartesianCRS : public CRS
{
public:
  CartesianCRS(int ndims, const char* dimname[], const char* dimunits[]);
  virtual ~CartesianCRS();

};

#endif
