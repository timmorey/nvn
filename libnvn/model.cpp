/**
	 model.cpp - Created by Timothy Morey on 1/15/2013
 */

#include "Layer.hpp"
#include "model.hpp"

int Model::GetWidth() const
{
	int retval = 0;

	if(this->TheLayer)
		retval = this->TheLayer->GetWidth();

	return retval;
}

int Model::GetHeight() const
{
	int retval = 0;

	if(this->TheLayer)
		retval = this->TheLayer->GetHeight();

	return retval;
}
