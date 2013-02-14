/**
  ViewTransform.cpp - Created by Timothy Morey on 2/5/2013
*/

#include "ViewTransform.hpp"

ViewTransform::ViewTransform(ScreenCRS& screen, CartesianCRS& model)
  : _ScreenCRS(screen),
    _ModelCRS(model)
{

}

ViewTransform::~ViewTransform()
{
}


int ViewTransform::ModelToScreen(float xin, float yin, int& xout, int& yout) const
{
  int retval = NVN_NOERR;
  return retval;
}

int ViewTransform::ModelToScreen(float xin, float yin, float zin, int& xout, int& yout) const
{
  int retval = NVN_NOERR;
  return retval;
}

int ViewTransform::ScreenToModel(int xin, int yin, float& xout, float& yout) const
{
  int retval = NVN_NOERR;
  return retval;
}

int ViewTransform::ScreenToModel(int xin, int yin, float& xout, float& yout, float& zout) const
{
  int retval = NVN_NOERR;
  return retval;
}
