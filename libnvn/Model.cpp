/**
	 Model.cpp - Created by Timothy Morey on 1/15/2013
 */


#include "nvn.h"

#include "CartesianCRS.hpp"
#include "Layer.hpp"
#include "Model.hpp"

#include <list>


Model::Model()
  : _Crs(4)
{

}

Model::~Model()
{

}

NVN_BBox Model::GetBounds() const
{
  NVN_BBox bounds = NVN_BBoxEmpty;

  std::list<Layer*>::const_iterator iter;
  for(iter = _Layers.begin(); iter != _Layers.end(); iter++)
  {
    NVN_BBoxUnion(bounds, (*iter)->GetBounds(), &bounds);
  }

  return bounds;
}

int Model::AddLayer(Layer* layer)
{
  int retval = NVN_NOERR;

  if(layer)
  {
    layer->SetModelCRS(_Crs);
    _Layers.push_back(layer);
  }

  return retval;
}

int Model::Render()
{
  int retval = NVN_NOERR;

  std::list<Layer*>::iterator iter;
  for(iter = _Layers.begin(); iter != _Layers.end(); iter++)
  {
    (*iter)->Render();
  }

  return retval;
}
