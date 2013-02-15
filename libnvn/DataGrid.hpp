/**
   DataGrid.hpp - Created by Timothy Morey on 1/18/2013
 */

#ifndef __DATAGRID_HPP__
#define __DATAGRID_HPP__


#include "nvn.h"
#include "variant.h"

#include "GridCRS.hpp"

#define MPICH_SKIP_MPICXX 1
#include <mpi.h>


class DataGrid
{
public:
  DataGrid(int ndims, const MPI_Offset dimlen[], MPI_Datatype type, void* data);
  DataGrid(int ndims, const char dimnames[][MAX_NAME], const MPI_Offset dimlen[], MPI_Datatype type, void* data);
  ~DataGrid();

public:
  const GridCRS& GetCRS() const { return _Crs; }

  MPI_Offset GetDimLen(int dim) const 
  { return dim >= 0 && dim < _NDims ? _DimLen[dim] : -1; }
  
  void* GetElem(const MPI_Offset i[]) 
  { return ((char*)_Data) + this->GetPos(i) * _TypeSize; }
  
  int GetElemAsVariant(const MPI_Offset i[], Variant* value) const;
  int GetPos(const MPI_Offset i[]) const;
  MPI_Datatype GetType() const { return _Type; }
  int GetTypeSize() const { return _TypeSize; }
  VariantType GetVarType() const { return _VarType; }
  bool HasData(const MPI_Offset i[]) const;

public:
  int SetNodataValue(Variant value);

protected:
  int _NDims;
  MPI_Offset _DimLen[MAX_DIMS];
  MPI_Datatype _Type;
  VariantType _VarType;
  int _TypeSize;
  void* _Data;
  Variant* _NodataValue;
  GridCRS _Crs;
};

#endif
