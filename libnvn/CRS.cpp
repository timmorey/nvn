/**
  CRS.cpp - Created by Timothy Morey on 1/27/2013
*/


#include "CRS.hpp"

#include <string.h>


CRS::CRS()
  : _NDims(0)
{
}

CRS::CRS(int ndims, const char* dimname[])
{
  _NDims = ndims;
  for(int i = 0; i < _NDims; i++)
  {
    strcpy(_DimName[i], dimname[i]);
  }
}

CRS::CRS(int ndims, const char* dimname[], const char* dimunits[])
{
  _NDims = ndims;
  for(int i = 0; i < _NDims; i++)
  {
    strcpy(_DimName[i], dimname[i]);
    strcpy(_DimUnits[i], dimunits[i]);
  }
}

int CRS::FindDim(const char* name) const
{
  int retval = -1;

  for(int i = 0; i < _NDims; i++)
  {
    if(0 == strcmp(name, _DimName[i]))
    {
      retval = i;
      break;
    }
  }

  return retval;
}
