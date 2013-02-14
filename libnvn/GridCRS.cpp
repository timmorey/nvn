/**
  GridCRS.cpp - Created by Timothy Morey on 1/27/2013
*/


#include "GridCRS.hpp"

#include <string.h>


GridCRS::GridCRS(int ndims, const char* dimname[])
  : CRS(ndims, dimname)
{
  for(int i = 0; i < _NDims; i++)
  {
    strcpy(_DimUnits[i], "");
  }
}

GridCRS::~GridCRS()
{

}