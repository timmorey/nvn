/**
	 layer.hpp - Created by Timothy Morey on 1/12/2013
 */

#ifndef __LAYER_HPP__
#define __LAYER_HPP__


#include "color-ramp.h"
#include "variant.h"

#include "DataGrid.hpp"

#define MPICH_SKIP_MPICXX 1
#include <mpi.h>


class Layer
{
public:
	Layer(DataGrid* grid);
	~Layer();

public:
	ColorRamp& Ramp() { return _Ramp; }

public:
	int Render();

public:
	int GetWidth() const { return _DataGrid ? _DataGrid->GetDimLen(0) : -1; }
	int GetHeight() const { return _DataGrid ? _DataGrid->GetDimLen(1) : -1; }

protected:
  DataGrid* _DataGrid;
	ColorRamp _Ramp;
	Variant _MinVal;
	Variant _MaxVal;
	char* _TexBitmap;
	int _TexWidth;
	int _TexHeight;
	unsigned int _TextureID;
};

#endif
