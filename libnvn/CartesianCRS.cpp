/**
  CartesianCRS.cpp - Created by Timothy Morey on 1/27/2013
*/


#include "nvn.h"

#include "CartesianCRS.hpp"
#include "CRS.hpp"

#include <string.h>


CartesianCRS::CartesianCRS()
  : CRS()
{

}

CartesianCRS::CartesianCRS(int ndims)
  : CRS(ndims)
{

}

CartesianCRS::CartesianCRS(int ndims, const char dimname[][MAX_NAME], const char dimunits[][MAX_NAME])
  : CRS(ndims, dimname, dimunits)
{

}

CartesianCRS::~CartesianCRS()
{

}
