/**
  ViewTransform.hpp - Created by Timothy Morey on 1/27/2013
*/

#ifndef __VIEW_TRANSFORM_HPP__
#define __VIEW_TRANSFORM_HPP__


#include "CartesianCRS.hpp"
#include "CRS.hpp"
#include "ScreenCRS.hpp"

class ViewTransform
{
public:
  ViewTransform(ScreenCRS& screen, CartesianCRS& model);
  ~ViewTransform();

public:
  const ScreenCRS& GetScreenCRS() const { return _ScreenCRS; }
  const CartesianCRS& GetModelCRS() const { return _ModelCRS; }

public:
  int ModelToScreen(float xin, float yin, int& xout, int& yout) const;
  int ModelToScreen(float xin, float yin, float zin, int& xout, int& yout) const;
  int ScreenToModel(int xin, int yin, float& xout, float& yout) const;
  int ScreenToModel(int xin, int yin, float& xout, float& yout, float& zout) const;

protected:
  ScreenCRS& _ScreenCRS;
  CartesianCRS& _ModelCRS;
};

#endif