/**
  ScreenCRS.hpp - Created by Timothy Morey on 1/27/2013
*/

#ifndef __SCREEN_CRS_HPP__
#define __SCREEN_CRS_HPP__


#include "CRS.hpp"


/**
  A ScreenCRS describes the CRS used to express pixel locations on screen or in
  a GUI window.  It is a two-dimensional CRS with the origin at the top-left 
  corner of the screen or window.  The dimensions are "x" and "y", and the 
  units are "pixels".  X coordiante values increase when moving left to right 
  across the screen, and Y coordinate values increase when moving top to bottom
  down the screen.
*/
class ScreenCRS : public CRS
{
public:
  ScreenCRS(int x, int y);
  virtual ~ScreenCRS();

protected:
  int _XOffset, _YOffset;
};

#endif