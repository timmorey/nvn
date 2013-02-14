/**
  CRS.hpp - Created by Timothy Morey on 1/27/2013
*/

#ifndef __CRS_HPP__
#define __CRS_HPP__


#include "nvn.h"


/**
  The CRS object provides an abstract base class for all
  CRSs.
*/
class CRS
{
protected:
  CRS();
  CRS(int ndims, const char* dimname[]);
  CRS(int ndims, const char* dimname[], const char* dimunits[]);
  virtual ~CRS();

public:
  int GetNDims() const { return _NDims; }
  const char* GetDimName(int dim) const { return _DimName[dim]; }
  const char* GetDimUnits(int dim) const { return _DimUnits[dim]; }
  int FindDim(const char* name) const;

protected:
  int _NDims;
  char _DimName[MAX_DIMS][MAX_NAME];
  char _DimUnits[MAX_DIMS][MAX_NAME];  
};

#endif