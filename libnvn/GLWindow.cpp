/*
 * GLWindow.cpp
 *
 *  Created on: Mar 4, 2013
 *      Author: Timothy Morey
 */


#include "nvn.h"

#include "GLWindow.hpp"
#include "GLX.hpp"
#include "Model.hpp"
#include "ReferenceFrameLayer.hpp"

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>


/******************************************************************************
 * Local Definitions
 ******************************************************************************/

/**
  The Hints structure is based on the Motif hints structure and is used to make
  a borderless window.  Based on code found here:
    http://tonyobryan.com/index.php?article=9
 */
typedef struct
{
  unsigned long Flags;
  unsigned long Functions;
  unsigned long Decorations;
  long InputMode;
  unsigned long status;
} Hints;


/******************************************************************************
 * Constructors
 ******************************************************************************/

GLWindow::GLWindow(const char* title, int x, int y, int width, int height,
                   bool borderless)
: _X(x),
  _Y(y),
  _Width(width),
  _Height(height),
  _Borderless(borderless),
  _LeftMouseDown(false),
  _CtrlDown(false),
  _AltDown(false),
  _CenterX(0.0f), _CenterY(0.0f),
  _ZoomLevel(1.0), _ZoomFactor(1.1),
  _XRotation(0.0f), _ZRotation(0.0f),
  _Dirty(true),
  _Model(0)
{
  XSetWindowAttributes windowAttrs;
  Colormap colormap;
  Display* display = GLX::GetDisplay();
  XVisualInfo* visualInfo = GLX::GetVisualInfo();

  printf("Creating window...\n");

  _GLXContext = glXCreateContext(display, visualInfo, 0, 1);
  if(! _GLXContext)
  {
    fprintf(stderr, "Unable to create a GLX context.");
    return;
  }

  colormap = XCreateColormap(display,
      RootWindow(display, visualInfo->screen),
      visualInfo->visual,
      AllocNone);

  windowAttrs.colormap = colormap;
  windowAttrs.border_pixel = 0;
  windowAttrs.event_mask =
      ExposureMask           |
      VisibilityChangeMask   |
      KeyPressMask           |
      KeyReleaseMask         |
      ButtonPressMask        |
      ButtonReleaseMask      |
      PointerMotionMask      |
      StructureNotifyMask    |
      SubstructureNotifyMask |
      FocusChangeMask;
  windowAttrs.override_redirect = 1;

  _XWindow = XCreateWindow(display,
      RootWindow(display, visualInfo->screen),
      _X, _Y, _Width, _Height,
      0,
      visualInfo->depth,
      InputOutput,
      visualInfo->visual,
      // CWOverrideRedirect can help when spanning multiple monitors...
      //CWBorderPixel | CWColormap | CWEventMask | CWOverrideRedirect,
      CWBorderPixel | CWColormap | CWEventMask,
      &windowAttrs);

  XSetStandardProperties(display,
                         _XWindow,
                         _Title,
                         _Title,
                         None,
                         0,
                         0,
                         0);

  if(_Borderless)
  {
    Hints hints;
    Atom prop;
    hints.Flags = 2;
    hints.Decorations = 0;
    prop = XInternAtom(display, "_MOTIF_WM_HINTS", True);
    XChangeProperty(display,
                    _XWindow,
                    prop,
                    prop,
                    32,
                    PropModeReplace,
                    (unsigned char *)&hints,
                    5);
  }

  // These next two lines allow us to intercept the message when the user
  // clicks the 'X' to close the window, so that we can close the window
  // cleanly and avoid errors.
  _WMDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", false);
  XSetWMProtocols(display, _XWindow, &_WMDeleteMessage, 1);

  glXMakeCurrent(display, _XWindow, _GLXContext);
  XMapWindow(display, _XWindow);
  glClearColor(0.0, 0.0, 0.0, 1.0);
}

GLWindow::~GLWindow()
{
  XDestroyWindow(GLX::GetDisplay(), _XWindow);
}


/******************************************************************************
 * Public Members
 ******************************************************************************/

int GLWindow::AsyncRefresh()
{
  int retval = NVN_NOERR;

  _Dirty = true;

  return retval;
}

float GLWindow::GetPixelsPerModelUnit() const
{
  float scale = 0.0f;  // pixels per model unit

  if(_Model)
  {
    NVN_BBox mbounds = _Model->GetBounds();
    float mwidth = mbounds.Max[XDIM] - mbounds.Min[XDIM];
    float mheight = mbounds.Max[YDIM] - mbounds.Min[YDIM];
    float viewAspect = (float)_Width / (float)_Height;
    float dataAspect = mwidth / mheight;

    if(viewAspect > dataAspect)
      scale = (float)_Height / (mheight / _ZoomLevel);
    else
      scale = (float)_Width / (mwidth / _ZoomLevel);
  }

  return scale;
}

int GLWindow::ResetView()
{
  int retval = NVN_NOERR;

  if(_Model)
  {
    NVN_BBox bounds = _Model->GetBounds();
    float width = bounds.Max[XDIM] - bounds.Min[XDIM];
    float height = bounds.Max[YDIM] - bounds.Min[YDIM];

    _CenterX = bounds.Min[XDIM] + width / 2.0f;
    _CenterY = bounds.Min[YDIM] + height / 2.0f;
    _ZoomLevel = 0.98f;  // back off just a bit to make sure the edges are visible
    _XRotation = 0.0f;
    _ZRotation = 0.0f;
    this->AsyncRefresh();
  }

  return retval;
}

int GLWindow::ShowModel(Model* model)
{
  int retval = NVN_NOERR;

  _Model = model;
  this->ResetView();
  this->AsyncRefresh();

  return retval;
}

int GLWindow::GetViewParms(float* centerx, float* centery, float* zoomlevel,
                           float* xrotation, float* zrotation) const
{
  int retval = NVN_NOERR;

  if(centerx)
    *centerx = _CenterX;

  if(centery)
    *centery = _CenterY;

  if(zoomlevel)
    *zoomlevel = _ZoomLevel;

  if(xrotation)
    *xrotation = _XRotation;

  if(zrotation)
    *zrotation = _ZRotation;

  return retval;
}

int GLWindow::SetViewParms(float centerx, float centery, float zoomlevel,
                           float xrotation, float zrotation)
{
  int retval = NVN_NOERR;

  _CenterX = centerx;
  _CenterY = centery;
  _ZoomLevel = zoomlevel;
  _XRotation = xrotation;
  _ZRotation = zrotation;
  this->AsyncRefresh();

  return retval;
}


/******************************************************************************
 * Protected Members
 ******************************************************************************/

int GLWindow::GetMousePos(int* x, int* y) const
{
  int retval = NVN_NOERR;

  Window root, child;
  int rootx, rooty, winx, winy;
  unsigned int mask;
  if(XQueryPointer(GLX::GetDisplay(), _XWindow, &root, &child,
      &rootx, &rooty, &winx, &winy, &mask))
  {
    *x = winx;
    *y = winy;
  }
  else
  {
    retval = NVN_EXWINFAIL;
    fprintf(stderr, "XQueryPointer returned false.\n");
  }

  return retval;
}

int GLWindow::GetMousePosInModel(float* x, float* y) const
{
  int retval = NVN_NOERR;

  int pixx, pixy;
  float scale;

  scale = GetPixelsPerModelUnit();
  retval = GetMousePos(&pixx, &pixy);

  float cosx = cos(_XRotation * DEG2RADF);
  float cosz = cos(_ZRotation * DEG2RADF);
  float sinz = sin(_ZRotation * DEG2RADF);

  *x = _CenterX -
      (_Width / 2.0f - pixx) / scale * cosz +
      (_Height / 2.0f - pixy) / scale / cosx * sinz;
  *y = _CenterY -
      (_Width / 2.0f - pixx) / scale * sinz -
      (_Height / 2.0f - pixy) / scale / cosx * cosz;

  return retval;
}

int GLWindow::HandleXButtonPress(XEvent event)
{
  float x1, y1, x2, y2;
  switch(event.xbutton.button)
  {
  case Button1:
    _LeftMouseDown = true;
    _MouseDownX = event.xbutton.x;
    _MouseDownY = event.xbutton.y;
    _MouseDownCenterX = _CenterX;
    _MouseDownCenterY = _CenterY;
    break;

  case Button4:
    GetMousePosInModel(&x1, &y1);
    _ZoomLevel *= _ZoomFactor;
    GetMousePosInModel(&x2, &y2);
    _CenterX -= x2 - x1;
    _CenterY += y2 - y1;
    this->AsyncRefresh();
    break;

  case Button5:
    GetMousePosInModel(&x1, &y1);
    _ZoomLevel /= _ZoomFactor;
    GetMousePosInModel(&x2, &y2);
    _CenterX -= x2 - x1;
    _CenterY += y2 - y1;
    this->AsyncRefresh();
    break;
  }
}

int GLWindow::HandleXButtonRelease(XEvent event)
{
  switch(event.xbutton.button)
  {
  case Button1:
    _LeftMouseDown = false;
    break;
  }
}

int GLWindow::HandleXConfigureNotify(XEvent event)
{
  _X = event.xconfigure.x;
  _Y = event.xconfigure.y;
  _Width = event.xconfigure.width;
  _Height = event.xconfigure.height;
}

int GLWindow::HandleXExpose(XEvent event)
{
  this->AsyncRefresh();
}

int GLWindow::HandleXKeyPress(XEvent event)
{
  int retval = NVN_NOERR;

  switch(XLookupKeysym(&event.xkey, 0))
  {
  case XK_Alt_L:
  case XK_Alt_R:
    _AltDown = true;
    _AltDownZRotation = _ZRotation;
    _AltDownX = event.xkey.x;
    _AltDownY = event.xkey.y;
    break;

  case XK_Control_L:
  case XK_Control_R:
    _CtrlDown = true;
    _CtrlDownXRotation = _XRotation;
    _CtrlDownX = event.xkey.x;
    _CtrlDownY = event.xkey.y;
    break;

  case XK_r:
  case XK_R:
    this->ResetView();
    break;
  }

  return retval;
}

int GLWindow::HandleXKeyRelease(XEvent event)
{
  int retval = NVN_NOERR;

  switch(XLookupKeysym(&event.xkey, 0))
  {
  case XK_Alt_L:
  case XK_Alt_R:
    _AltDown = false;
    break;

  case XK_Control_L:
  case XK_Control_R:
    _CtrlDown = false;
    break;
  }

  return retval;
}

int GLWindow::HandleXMotionNotify(XEvent event)
{
  if(_LeftMouseDown)
  {
    float cosx = cos(_XRotation * DEG2RADF);
    float cosz = cos(_ZRotation * DEG2RADF);
    float sinz = sin(_ZRotation * DEG2RADF);
    float scale = this->GetPixelsPerModelUnit();

    _CenterX = _MouseDownCenterX +
        ((_MouseDownX - event.xbutton.x) / scale) * cosz -
        ((_MouseDownY - event.xbutton.y) / scale / cosx * sinz);
    _CenterY = _MouseDownCenterY -
        ((_MouseDownX - event.xbutton.x) / scale) * sinz -
        ((_MouseDownY - event.xbutton.y) / scale / cosx * cosz);

    this->AsyncRefresh();
  }

  if(_CtrlDown)
  {
    _XRotation = _CtrlDownXRotation - (_CtrlDownY - event.xbutton.y);
    this->AsyncRefresh();
  }

  if(_AltDown)
  {
    _ZRotation = _AltDownZRotation + (_AltDownX - event.xbutton.x);
    this->AsyncRefresh();
  }
}

int GLWindow::RenderModel()
{
  glViewport(0, 0, _Width, _Height);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  float scale = this->GetPixelsPerModelUnit();
  float xmin = _CenterX - (_Width / 2.0f) / scale;
  float xmax = _CenterX + (_Width / 2.0f) / scale;
  float ymin = _CenterY - (_Height / 2.0f) / scale;
  float ymax = _CenterY + (_Height / 2.0f) / scale;
  glOrtho(xmin, xmax, ymin, ymax, -10000.0f, 10000.0f);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  if(_Model->GetNDims() <= 2)
  {
    // For models without depth, just do full ambient lighting
    float ambcolor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glEnable(GL_LIGHTING);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambcolor);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT);
  }
  else
  {
    // If we've got depth, then do a combination of ambient and diffuse
    // lighting to show off the depth.

    float lightpos[] = { _CenterX, _CenterY, 10000.0f, 1.0f };
    float diffcolor[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    float ambcolor[] = { 0.5f, 0.5f, 0.5f, 1.0f };

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambcolor);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffcolor);
    glLightfv(GL_LIGHT0, GL_POSITION, lightpos);

    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
  }

  glTranslatef(_CenterX, _CenterY, 0.0f);
  glRotatef(_XRotation, 1.0f, 0.0f, 0.0f);
  glRotatef(_ZRotation, 0.0f, 0.0f, 1.0f);
  glTranslatef(-_CenterX,-_CenterY, 0.0f);

  if(_Model)
    _Model->Render();

  ReferenceFrameLayer frame(_Model->GetCRS(), _Model->GetBounds());
  frame.Render();

  glXSwapBuffers(GLX::GetDisplay(), _XWindow);

  _Dirty = false;
}
