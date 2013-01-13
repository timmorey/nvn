/**
	 layer.cpp - Created by Timothy Morey on 1/12/2013
*/

#include "color-ramp.h"
#include "layer.hpp"

#define MPICH_SKIP_MPICXX 1
#include <mpi.h>

#include <float.h>

#include <GL/gl.h>
#include <GL/glu.h>

Layer::Layer(int width, int height, MPI_Datatype datatype, void* data)
	: _Width(width),
		_Height(height),
		_DataType(datatype),
		_Data(data),
		_Ramp(DefaultColorRamp),
		_TexBitmap(0)
{
	double min = DBL_MAX;
	double max = DBL_MIN;
	for(int x = 0; x < _Width; x++)
	{
		for(int y = 0; y < _Height; y++)
		{
			double pivot = ((double*)_Data)[y * _Width + x];
			if(pivot < min) min = pivot;
			if(pivot > max) max = pivot;
		}
	}

	printf("width=%d, height=%d, min=%f, max=%f\n", _Width, _Height, min, max);

	_MinVal.Type = VariantTypeDouble;
	_MinVal.Value.DoubleVal = min;

	_MaxVal.Type = VariantTypeDouble;
	_MaxVal.Value.DoubleVal = max;
}

Layer::~Layer()
{
	if(_TexBitmap)
	{
		free(_TexBitmap);
		_TexBitmap = 0;
	}
}

int Layer::Render()
{
	// Render Method 1: Render each grid point as a colored square:
	//
	// glMatrixMode( GL_MODELVIEW );
	// glLoadIdentity( );
	// gluLookAt(_Width / 2.0, _Height / 2.0, 1000.0,
	//  					_Width / 2.0, _Height / 2.0, 0.0,
	//  					0.0,          1.0,           0.0);

	// glBegin(GL_QUADS);
	// {
	// 	Variant value;
	// 	value.Type = VariantTypeDouble;
	// 	for(int x = 0; x < _Width; x++)
	// 	{
	// 		for(int y = 0; y < _Height; y++)
	// 		{
	// 			value.Value.DoubleVal = ((double*)_Data)[y * _Width + x];
	// 			int color = GetColor(_Ramp, value, _MinVal, _MaxVal);
	// 			glColor4ub(GetR(color), GetG(color), GetB(color), GetA(color));
	// 			glVertex3f((float)x, (float)y, 0.0);
	// 			glVertex3f((float)x + 1.0, (float)y, 0.0);
	// 			glVertex3f((float)x + 1.0, (float)y + 1.0, 0.0);
	// 			glVertex3f((float)x, (float)y + 1.0, 0.0);
	// 		}
	// 	}
	// }
	// glEnd();


	// Render Method 2: Render the grid into a texture and then draw it onto a
	//                  single rectangle.

	if(0 == _TexBitmap)
	{
		_TexWidth = 1;
		_TexHeight = 1;

		while(_TexWidth < _Width)
			_TexWidth *= 2;

		while(_TexHeight < _Height)
			_TexHeight *= 2;

		_TexBitmap = (char*)malloc(_TexWidth * _TexHeight * 4);
		Variant value;
		value.Type = VariantTypeDouble;
		for(int x = 0; x < _Width; x++)
		{
			for(int y = 0; y < _Height; y++)
			{
				value.Value.DoubleVal = ((double*)_Data)[y * _Width + x];
				((int*)_TexBitmap)[y * _TexWidth + x] = 
					GetColor(_Ramp, value, _MinVal, _MaxVal);
			}
		}

		glGenTextures(1, &_TextureID);
		glBindTexture(GL_TEXTURE_2D, _TextureID);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, 4, 
								 _TexWidth, _TexHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, _TexBitmap);
	}
	
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );
	gluLookAt(_Width / 2.0, _Height / 2.0, 1000.0,
	 					_Width / 2.0, _Height / 2.0, 0.0,
	 					0.0,          1.0,           0.0);

	glBindTexture(GL_TEXTURE_2D, _TextureID);
	glEnable(GL_TEXTURE_2D);
	
	glBegin(GL_QUADS);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);

	glTexCoord2f((float)_Width / (float)_TexWidth, 0.0f);
	glVertex3f((float)_Width, 0.0f, 0.0f);

	glTexCoord2f((float)_Width / (float)_TexWidth, 
							 (float)_Height / (float)_TexHeight);
	glVertex3f((float)_Width, (float)_Height, 0.0f);

	glTexCoord2f(0.0f, (float)_Height / (float)_TexHeight);
	glVertex3f(0.0f, (float)_Height, 0.0f);

	glEnd();
}
