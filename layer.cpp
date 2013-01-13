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
		_Ramp(DefaultColorRamp)
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

int Layer::Render() const
{
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity( );
		gluLookAt(_Width / 2.0, _Height / 2.0, 1000.0,
							_Width / 2.0, _Height / 2.0, 0.0,
							0.0,          1.0,           0.0);

		glBegin(GL_QUADS);

		glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
		glVertex3f(0.0f,          0.0f,           0.0f);
		glVertex3f((float)_Width, 0.0f,           0.0f);
		glVertex3f((float)_Width, (float)_Height, 0.0f);
		glVertex3f(0.0f,          (float)_Height, 0.0f);

		glEnd();

		glBegin(GL_LINES);

		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

		glVertex3f(0.0f,          0.0f,           0.0f);
		glVertex3f((float)_Width, 0.0f,           0.0f);

		glVertex3f((float)_Width, 0.0f,           0.0f);
		glVertex3f((float)_Width, (float)_Height, 0.0f);

		glVertex3f((float)_Width, (float)_Height, 0.0f);
		glVertex3f(0.0f,          (float)_Height, 0.0f);

		glVertex3f(0.0f,          (float)_Height, 0.0f);
		glVertex3f(0.0f,          0.0f,           0.0f);

		glEnd();
}
