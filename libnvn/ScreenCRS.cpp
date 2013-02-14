/**
  ScreenCRS.cpp - Created by Timothy Morey on 1/17/2013
*/


#include "ScreenCRS.hpp"

#include <string.h>


ScreenCRS::ScreenCRS(int x, int y)
  : CRS(),
    _XOffset(x),
    _YOffset(y)
{
  _NDims = 2;
  strcpy(_DimName[0], "x");
  strcpy(_DimName[1], "y");
  strcpy(_DimUnits[0], "pixels");
  strcpy(_DimUnits[1], "pixels");
}

ScreenCRS::~ScreenCRS()
{

}