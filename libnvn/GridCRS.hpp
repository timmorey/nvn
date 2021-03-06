/**
  GridCRS.hpp - Created by Timothy Morey on 1/27/2013
*/

#ifndef __GRID_CRS_HPP__
#define __GRID_CRS_HPP__


#include "CRS.hpp"


/**
  A GridCRS represents the coordinate system for a n-dimensional regular
  rectangular grid.  Positions in the grid are indexed with integers.
*/
class GridCRS : public CRS
{
public:
  GridCRS(int gridndims);
  GridCRS(int gridndims, const char griddim[][MAX_NAME]);
  GridCRS(const GridCRS& other);
  ~GridCRS();

protected:

};

#endif
