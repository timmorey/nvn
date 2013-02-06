/**
   DataGrid.hpp - Created by Timothy Morey on 1/18/2013
 */

#ifndef __DATAGRID_HPP__
#define __DATAGRID_HPP__


#include "nvn.h"
#include "variant.h"

#define MPICH_SKIP_MPICXX 1
#include <mpi.h>


class DataGrid
{
public:
  DataGrid(int ndims, MPI_Offset dimlen[], MPI_Datatype type, void* data);

public:
  MPI_Offset GetDimLen(int dim) const 
  { return dim >= 0 && dim < _NDims ? _DimLen[dim] : -1; }
  
  void* GetElem(const MPI_Offset i[]) 
  { return ((char*)_Data) + this->GetPos(i) * _TypeSize; }
  
  int GetElemAsVariant(const MPI_Offset i[], Variant* value) const;
  int GetPos(const MPI_Offset i[]) const;
  MPI_Datatype GetType() const { return _Type; }
  int GetTypeSize() const { return _TypeSize; }
  bool HasData(const MPI_Offset i[]) const;

public:
  int SetNodataValue(Variant value);

protected:
  int _NDims;
  MPI_Offset _DimLen[MAX_DIMS];
  MPI_Datatype _Type;
  int _TypeSize;
  void* _Data;
  Variant* _NodataValue;
};

#endif
