/**
	 ShadedSurfaceLayer.cpp - Created by Timothy Morey on 1/12/2013
 */

#include "color-ramp.h"
#include "nvn.h"
#include "variant.h"

#include "DataGrid.hpp"
#include "ShadedSurfaceLayer.hpp"

#define MPICH_SKIP_MPICXX 1
#include <mpi.h>

#include <float.h>
#include <math.h>

#include <GL/gl.h>
#include <GL/glu.h>

ShadedSurfaceLayer::ShadedSurfaceLayer(DataGrid* grid)
: Layer(),
  _DataGrid(grid),
  _Ramp(DefaultColorRamp),
  _TexBitmap(0),
  _TexWidth(0),
  _TexHeight(0),
  _TextureID(-1),
  _DisplayList(-1),
  _Compiled(false)
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

ShadedSurfaceLayer::~ShadedSurfaceLayer()
{
  if(_TexBitmap)
  {
    free(_TexBitmap);
    _TexBitmap = 0;
  }
}

int ShadedSurfaceLayer::DrawTriangle(MPI_Offset pt1[], MPI_Offset pt2[], MPI_Offset pt3[]) const
{
  int retval = NVN_NOERR;
  Variant v1, v2, v3;
  int c1, c2, c3;
  float x1, x2, x3;
  float y1, y2, y3;
  float z1, z2, z3;

  _DataGrid->GetElemAsVariant(pt1, &v1);
  _DataGrid->GetElemAsVariant(pt2, &v2);
  _DataGrid->GetElemAsVariant(pt3, &v3);

  c1 = GetColor(_Ramp, v1, _MinVal, _MaxVal);
  c2 = GetColor(_Ramp, v2, _MinVal, _MaxVal);
  c3 = GetColor(_Ramp, v3, _MinVal, _MaxVal);

  x1 = (float)pt1[0];  y1 = (float)pt1[1];  z1 = VariantValueAsFloat(v1) / 100.0f;
  x2 = (float)pt2[0];  y2 = (float)pt2[1];  z2 = VariantValueAsFloat(v2) / 100.0f;
  x3 = (float)pt3[0];  y3 = (float)pt3[1];  z3 = VariantValueAsFloat(v3) / 100.0f;

  glNormal3f(((y2-y1)*(z3-z1)) - ((z2-z1)*(y3-y1)),
             ((z2-z1)*(x3-x1)) - ((x2-x1)*(z3-z1)),
             ((x2-x1)*(y3-y1)) - ((y2-y1)*(x3-x1)));

  glColor4ub(GetR(c1), GetG(c1), GetB(c1), GetA(c1));
  glVertex3f(x1, y1, z1);

  glColor4ub(GetR(c2), GetG(c2), GetB(c2), GetA(c2));
  glVertex3f(x2, y2, z2);

  glColor4ub(GetR(c3), GetG(c3), GetB(c3), GetA(c3));
  glVertex3f(x3, y3, z3);
}

int ShadedSurfaceLayer::Render()
{
  if(! _Compiled)
  {
    int datawidth = this->GetWidth();
    int dataheight = this->GetHeight();
    MPI_Offset nw[MAX_DIMS], ne[MAX_DIMS], se[MAX_DIMS], sw[MAX_DIMS];
    Variant value;
    int color;

    _DisplayList = glGenLists(1);

    glNewList(_DisplayList, GL_COMPILE);
    {
      glBegin(GL_TRIANGLES);
      {
        for(sw[0] = 0, se[0] = 1, ne[0] = 1, nw[0] = 0; 
            se[0] < datawidth && ne[0] < datawidth;
            sw[0]++, se[0]++, ne[0]++, nw[0]++)
        {
          for(sw[1] = 0, se[1] = 0, ne[1] = 1, nw[1] = 1; 
              se[1] < dataheight && ne[1] < dataheight;
              sw[1]++, se[1]++, ne[1]++, nw[1]++)
          {        
            if(_DataGrid->HasData(ne) && 
                _DataGrid->HasData(se) &&
                _DataGrid->HasData(sw) &&
                _DataGrid->HasData(nw))
            {
              Variant vsw, vse, vne, vnw;

              _DataGrid->GetElemAsVariant(sw, &vsw);
              _DataGrid->GetElemAsVariant(se, &vse);
              _DataGrid->GetElemAsVariant(ne, &vne);
              _DataGrid->GetElemAsVariant(nw, &vnw);

              if(fabs(VariantValueAsFloat(vsw) - VariantValueAsFloat(vne)) <
                  fabs(VariantValueAsFloat(vnw) - VariantValueAsFloat(vse)))
              {
                this->DrawTriangle(sw, nw, ne);
                this->DrawTriangle(ne, se, sw);
              }
              else
              {
                this->DrawTriangle(nw, ne, se);
                this->DrawTriangle(se, sw, nw);
              }
            }
            else if(_DataGrid->HasData(ne) && 
                _DataGrid->HasData(se) &&
                _DataGrid->HasData(sw))
            {
              this->DrawTriangle(sw, se, ne);
            }
            else if(_DataGrid->HasData(ne) && 
                _DataGrid->HasData(se) &&
                _DataGrid->HasData(nw))
            {
              this->DrawTriangle(se, ne, nw);
            }
            else if (_DataGrid->HasData(ne) && 
                _DataGrid->HasData(sw) &&
                _DataGrid->HasData(nw))
            {
              this->DrawTriangle(ne, nw, sw);
            }
            else if(_DataGrid->HasData(se) &&
                _DataGrid->HasData(sw) &&
                _DataGrid->HasData(nw))
            {
              this->DrawTriangle(se, sw, nw);
            }
          }
        }
      }
      glEnd();
    }
    glEndList();

    _Compiled = true;
  }

  glCallList(_DisplayList);

  // int datawidth = this->GetWidth();
  // int dataheight = this->GetHeight();

  // if(0 == _TexBitmap)
  // {
  // 	_TexWidth = 1;
  // 	_TexHeight = 1;

  // 	while(_TexWidth < datawidth)
  // 		_TexWidth *= 2;

  // 	while(_TexHeight < dataheight)
  // 		_TexHeight *= 2;

  // 	_TexBitmap = (char*)malloc(_TexWidth * _TexHeight * 4);
  // 	Variant value;
  //   MPI_Offset pos[MAX_DIMS];
  // 	for(pos[0] = 0; pos[0] < datawidth; pos[0]++)
  // 	{
  // 		for(pos[1] = 0; pos[1] < dataheight; pos[1]++)
  // 		{
  //       if(_DataGrid->HasData(pos))
  //       {
  //         _DataGrid->GetElemAsVariant(pos, &value);
  //         ((int*)_TexBitmap)[pos[1] * _TexWidth + pos[0]] = 
  //           GetColor(_Ramp, value, _MinVal, _MaxVal);
  //       }
  //       else
  //       {
  //         ((int*)_TexBitmap)[pos[1] * _TexWidth + pos[0]] = 0x00000000;
  //       }
  // 		}
  // 	}

  // 	glGenTextures(1, &_TextureID);
  // 	glBindTexture(GL_TEXTURE_2D, _TextureID);
  // 	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  // 	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  // 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  // 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  // 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  // 	glTexImage2D(GL_TEXTURE_2D, 0, 4,
  // 							 _TexWidth, _TexHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, _TexBitmap);

  //   // TODO: we're done with _TexBitmap now, and might as well free it, but then
  //   // we'd need a different flag telling us if we need to create the texture...
  // }

  // glBindTexture(GL_TEXTURE_2D, _TextureID);
  // glEnable(GL_TEXTURE_2D);

  // glBegin(GL_QUADS);
  // {
  //   glTexCoord2f(0.0f, 0.0f);
  //   glVertex3f(0.0f, 0.0f, 0.0f);

  //   glTexCoord2f((float)datawidth / (float)_TexWidth, 0.0f);
  //   glVertex3f((float)datawidth, 0.0f, 0.0f);

  //   glTexCoord2f((float)datawidth / (float)_TexWidth, 
  // 						 (float)dataheight / (float)_TexHeight);
  //   glVertex3f((float)datawidth, (float)dataheight, 0.0f);

  //   glTexCoord2f(0.0f, (float)dataheight / (float)_TexHeight);
  //   glVertex3f(0.0f, (float)dataheight, 0.0f);
  // }
  // glEnd();


}
