/**
	 model.hpp - Created by Timothy Morey on 1/12/2013
 */

#ifndef __MODEL_HPP__
#define __MODEL_HPP__

class Layer;

class Model
{
public:
	Model() : TheLayer(0) {}
	~Model() {}

public:
	int GetWidth() const;
	int GetHeight() const;

public:
	Layer* TheLayer;
};

#endif
