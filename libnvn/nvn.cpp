/**
 * nvn.cpp - Created by Timothy Morey on 2/6/2013
 */


#include "gl-window.hpp"
#include "nvn.h"

#include <list>
using namespace std;


#define MAX_WINDOWS 256


extern "C" NVN_Err NVN_CreateWindow(const char* title,
                         int x, int y,
                         int width, int height,
                         int borderless,
                         NVN_Window* window)
{
  NVN_Err retval = NVN_NOERR;

  if(window)
  {
    *window = (NVN_Window)new GLWindow(title, x, y,
        width, height, (bool)borderless);
  }
  else
  {
    retval = NVN_EINVARGS;
  }

  return retval;
}

extern "C" NVN_Err NVN_DestroyWindow(NVN_Window window)
{
  NVN_Err retval = NVN_NOERR;

  if(window)
  {
    GLWindow* w = (GLWindow*)window;
    w->CloseWindow();
    delete w;
  }

  return retval;
}
