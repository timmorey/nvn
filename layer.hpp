/**
	 layer.hpp - Created by Timothy Morey on 1/12/2013
 */

#ifndef __LAYER_HPP__
#define __LAYER_HPP__

#include "color-ramp.h"
#include "variant.h"

#define MPICH_SKIP_MPICXX 1
#include <mpi.h>

class Layer
{
public:
	Layer(int width, int height, MPI_Datatype datatype, void* data);
	~Layer();

public:
	ColorRamp& Ramp() { return _Ramp; }

public:
	int Render();

public:
	int GetWidth() const { return _Width; }
	int GetHeight() const { return _Height; }

protected:
	int _Width;
	int _Height;
	MPI_Datatype _DataType;
	void* _Data;
	ColorRamp _Ramp;
	Variant _MinVal;
	Variant _MaxVal;
	char* _TexBitmap;
	int _TexWidth;
	int _TexHeight;
	unsigned int _TextureID;
};

#endif
