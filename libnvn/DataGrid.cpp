/**
   DataGrid.cpp - Created by Timothy Morey on 1/18/2013
 */


#include "DataGrid.hpp"

#define MPICH_SKIP_MPICXX 1
#include <mpi.h>
#include <string.h>


DataGrid::DataGrid(int ndims, MPI_Offset dimlen[], MPI_Datatype type, void* data)
  : _NDims(ndims),
    _Data(data),
    _Type(type),
    _NodataValue(0)
{
  memcpy(_DimLen, dimlen, ndims * sizeof(MPI_Offset));
  MPI_Type_size(_Type, &_TypeSize);
}

int DataGrid::GetElemAsVariant(const MPI_Offset i[], Variant* value) const
{
  int retval = NVN_NOERR;

	if(value)
	{
		int pos = this->GetPos(i);
		switch(_Type)
		{
		case MPI_FLOAT:
			value->Type = VariantTypeFloat;
			value->Value.FloatVal = ((float*)_Data)[pos];
			break;

		case MPI_DOUBLE:
			value->Type = VariantTypeDouble;
			value->Value.DoubleVal = ((double*)_Data)[pos];
			break;

		default:
			retval = NVN_EINVTYPE;
			fprintf(stderr, "Layer::GetValueAsVariant - Data type not supported.\n");
			break;
		}
	}
	else
	{
		retval = NVN_EINVARGS;
	}  

  return retval;
}

int DataGrid::GetPos(const MPI_Offset i[]) const
{
  int pos = 0;
  int stride = 1;

  for(int j = _NDims - 1; j >= 0; --j)
  {
    pos += i[j] * stride;
    stride *= _DimLen[j];
  }

  return pos;
}

bool DataGrid::HasData(const MPI_Offset i[]) const
{
  if(_NodataValue &&
     MPI_FLOAT == _Type && 
     abs(((float*)_Data)[this->GetPos(i)] - _NodataValue->Value.FloatVal) < 1e-4)
    return false;
  else
    return true;
}

int DataGrid::SetNodataValue(Variant value)
{
  if(_NodataValue)
    free(_NodataValue);

  _NodataValue = (Variant*)malloc(sizeof(Variant));
  *_NodataValue = value;
}
