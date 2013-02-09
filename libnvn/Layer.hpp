/**
	 Layer.hpp - Created by Timothy Morey on 1/12/2013
 */

#ifndef __LAYER_HPP__
#define __LAYER_HPP__


class Layer
{
protected:
  Layer() {};

public:
  virtual ~Layer() {};

public:
  virtual int Render() = 0;

public:
  virtual float GetWidth() const = 0;
  virtual float GetHeight() const = 0;
  virtual float GetDepth() const = 0;

};

#endif
