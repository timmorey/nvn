/**
  GridTransform.hpp - Created by Timothy Morey on 1/27/2013
*/

#ifndef __GRID_TRANSFORM_HPP__
#define __GRID_TRANSFORM_HPP__


#include "CRS.hpp"
#include "GridCRS.hpp"


class GridTransform
{
public:
  GridTransform(const CRS& basecrs, const GridCRS& gridcrs);
  ~GridTransform();

public:
  int GridToModel(const MPI_Offset posin[], float posout[]) const;
  int ModelToGrid(const float posin[], float posout[]) const;
  int ModelToGrid(const float posin[], MPI_Offset posout[]) const;

protected:
  const CRS& _BaseCRS;
  const GridCRS& _GridCRS;
};

#endif
