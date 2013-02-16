/**
  CRS.hpp - Created by Timothy Morey on 1/27/2013
*/

#ifndef __CRS_HPP__
#define __CRS_HPP__


#include "nvn.h"


/**
  The CRS object provides an abstract base class for all coordinate reference
  systems.
*/
class CRS
{
protected:
  CRS();
  CRS(int ndims);
  CRS(int ndims, const char dimname[][MAX_NAME]);
  CRS(int ndims, const char dimname[][MAX_NAME], const char dimunits[][MAX_NAME]);
  CRS(const CRS& other);
  virtual ~CRS();

public:
  int Equal(const CRS& other) const;
  int FindDim(const char* name) const;
  int GetNDims() const { return _NDims; }
  int GetDimName(int dim, char name[]) const;
  int GetDimUnits(int dim, char units[]) const;

public:
  int SetDimName(int dim, const char* name);
  int SetDimUnits(int dim, const char* units);
  int SetNDims(int ndims);

protected:
  int _NDims;
  char _DimName[MAX_DIMS][MAX_NAME];
  char _DimUnits[MAX_DIMS][MAX_NAME];  
};

#endif
