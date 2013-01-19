/**
	 layer.cpp - Created by Timothy Morey on 1/12/2013
*/

#include "color-ramp.h"
#include "layer.hpp"
#include "nvn.h"
#include "variant.h"

#include "DataGrid.hpp"

#define MPICH_SKIP_MPICXX 1
#include <mpi.h>

#include <float.h>

#include <GL/gl.h>
#include <GL/glu.h>

Layer::Layer(DataGrid* grid)
	: _DataGrid(grid),
		_Ramp(DefaultColorRamp),
		_TexBitmap(0),
    _TexWidth(0),
    _TexHeight(0),
    _TextureID(-1)
{
  if(_DataGrid)
  {
    Variant value;
    MPI_Offset pos[MAX_DIMS];
    _MinVal = MaxVariant(_DataGrid->GetType());
    _MaxVal = MinVariant(_DataGrid->GetType());
    for(pos[0] = 0; pos[0] < this->GetWidth(); pos[0]++)
    {
      for(pos[1] = 0; pos[1] < this->GetHeight(); pos[1]++)
      {
        if(_DataGrid->HasData(pos))
        {
          _DataGrid->GetElemAsVariant(pos, &value);
          if(0 > VariantCompare(value, _MinVal)) _MinVal = value;
          if(0 < VariantCompare(value, _MaxVal)) _MaxVal = value;
        }
      }
    }

    printf("min=%f, max=%f\n", 
           VariantValueAsDouble(_MinVal), VariantValueAsDouble(_MaxVal));
  }
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
  int datawidth = this->GetWidth();
  int dataheight = this->GetHeight();

	if(0 == _TexBitmap)
	{
		_TexWidth = 1;
		_TexHeight = 1;

		while(_TexWidth < datawidth)
			_TexWidth *= 2;

		while(_TexHeight < dataheight)
			_TexHeight *= 2;

		_TexBitmap = (char*)malloc(_TexWidth * _TexHeight * 4);
		Variant value;
    MPI_Offset pos[MAX_DIMS];
		for(pos[0] = 0; pos[0] < datawidth; pos[0]++)
		{
			for(pos[1] = 0; pos[1] < dataheight; pos[1]++)
			{
        if(_DataGrid->HasData(pos))
        {
          _DataGrid->GetElemAsVariant(pos, &value);
          ((int*)_TexBitmap)[pos[1] * _TexWidth + pos[0]] = 
            GetColor(_Ramp, value, _MinVal, _MaxVal);
        }
        else
        {
          ((int*)_TexBitmap)[pos[1] * _TexWidth + pos[0]] = 0x00000000;
        }
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

    // TODO: we're done with _TexBitmap now, and might as well free it, but then
    // we'd need a different flag telling us if we need to create the texture...
	}
	
	glBindTexture(GL_TEXTURE_2D, _TextureID);
	glEnable(GL_TEXTURE_2D);
	
	glBegin(GL_QUADS);
  {
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    
    glTexCoord2f((float)datawidth / (float)_TexWidth, 0.0f);
    glVertex3f((float)datawidth, 0.0f, 0.0f);

    glTexCoord2f((float)datawidth / (float)_TexWidth, 
							 (float)dataheight / (float)_TexHeight);
    glVertex3f((float)datawidth, (float)dataheight, 0.0f);
    
    glTexCoord2f(0.0f, (float)dataheight / (float)_TexHeight);
    glVertex3f(0.0f, (float)dataheight, 0.0f);
  }
	glEnd();
}
