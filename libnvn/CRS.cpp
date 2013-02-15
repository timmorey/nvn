/**
  CRS.cpp - Created by Timothy Morey on 1/27/2013
*/


#include "nvn.h"

#include "CRS.hpp"

#include <string.h>


CRS::CRS()
  : _NDims(0)
{
}

CRS::CRS(int ndims)
  : _NDims(ndims)
{
  if(_NDims > 0)
  {
    strcpy(_DimName[0], "x");
    strcpy(_DimUnits[0], "unknown");
  }

  if(_NDims > 1)
  {
    strcpy(_DimName[1], "y");
    strcpy(_DimUnits[1], "unknown");
  }

  if(_NDims > 2)
  {
    strcpy(_DimName[2], "z");
    strcpy(_DimUnits[2], "unknown");
  }

  if(_NDims > 3)
  {
    strcpy(_DimName[3], "t");
    strcpy(_DimUnits[3], "unknown");
  }
}

CRS::CRS(int ndims, const char dimname[][MAX_NAME])
  : _NDims(ndims)
{
  for(int i = 0; i < _NDims; i++)
  {
    strcpy(_DimName[i], dimname[i]);
    strcpy(_DimUnits[i], "");
  }
}

CRS::CRS(int ndims, const char dimname[][MAX_NAME], const char dimunits[][MAX_NAME])
  : _NDims(ndims)
{
  for(int i = 0; i < _NDims; i++)
  {
    strcpy(_DimName[i], dimname[i]);
    strcpy(_DimUnits[i], dimunits[i]);
  }
}

CRS::CRS(const CRS& other)
{
  _NDims = other._NDims;
  for(int i = 0; i < _NDims; i++)
  {
    strcpy(_DimName[i], other._DimName[i]);
    strcpy(_DimUnits[i], other._DimUnits[i]);
  }
}

CRS::~CRS()
{

}

int CRS::GetDimName(int dim, char name[]) const
{
  int retval = NVN_NOERR;

  if(dim >= 0 && dim < _NDims)
  {
    strcpy(name, _DimName[dim]);
  }
  else
  {
    retval = NVN_EINVARGS;
  }

  return retval;
}

int CRS::GetDimUnits(int dim, char units[]) const
{
  int retval = NVN_NOERR;

  if(dim >= 0 && dim < _NDims)
  {
    strcpy(units, _DimUnits[dim]);
  }
  else
  {
    retval = NVN_EINVARGS;
  }

  return retval;
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

int CRS::SetNDims(int ndims)
{
  int retval = NVN_NOERR;

  if(ndims > 0 && ndims <= MAX_DIMS)
  {
    if(ndims > _NDims)
    {
      for(int i = _NDims; i < ndims; i++)
      {
        strcpy(_DimName[i], "");
        strcpy(_DimUnits[i], "");
      }
    }

    _NDims = ndims;
  }
  else
  {
    retval = NVN_EINVARGS;
  }

  return retval;
}

int CRS::SetDimName(int dim, const char* name)
{
  int retval = NVN_NOERR;

  if(dim >= 0 && dim < MAX_DIMS)
  {
    strcpy(_DimName[dim], name);
  }
  else
  {
    retval = NVN_EINVARGS;
  }

  return retval;
}

int CRS::SetDimUnits(int dim, const char* units)
{
  int retval = NVN_NOERR;

  if(dim >= 0 && dim < MAX_DIMS)
  {
    strcpy(_DimUnits[dim], units);
  }
  else
  {
    retval = NVN_EINVARGS;
  }

  return retval;
}
