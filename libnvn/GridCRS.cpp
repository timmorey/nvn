/**
  GridCRS.cpp - Created by Timothy Morey on 1/27/2013
*/


#include "GridCRS.hpp"

#include <string.h>


GridCRS::GridCRS(int ndims)
  : CRS(ndims)
{

}

GridCRS::GridCRS(int ndims, const char dimname[][MAX_NAME])
  : CRS(ndims, dimname)
{

}

GridCRS::GridCRS(const GridCRS& other)
  : CRS(other)
{

}

GridCRS::~GridCRS()
{

}
