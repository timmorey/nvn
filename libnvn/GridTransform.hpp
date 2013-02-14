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
  GridTransform(CRS& basecrs, GridCRS& gridcrs);
  ~GridTransform();

public:
  int GridToModel(const int posin[], float posout[]) const;
  int ModelToGrid(const float posin[], float posout[]) const;
  int ModelToGrid(const float posin[], int posout[]) const;

protected:
  CRS& _BaseCRS;
  GridCRS& _GridCRS;
};

#endif