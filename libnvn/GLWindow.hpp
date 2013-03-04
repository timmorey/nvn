/*
 * GLWindow.hpp
 *
 *  Created on: Mar 4, 2013
 *      Author: Timothy Morey
 */

#ifndef __GLWINDOW_HPP__
#define __GLWINDOW_HPP__


#include <GL/glx.h>
#include <X11/Xlib.h>


class Model;

class GLWindow
{
public:
  GLWindow(const char* title, int x, int y, int width, int height,
           bool borderless);
  ~GLWindow();

public:
  int AsyncRefresh();
  int ResetView();
  int ShowModel(Model* model);

public:
  float GetPixelsPerModelUnit() const;
  int GetX() const { return _X; }
  int GetY() const { return _Y; }
  int GetWidth() const { return _Width; }
  int GetHeight() const { return _Height; }
  Atom GetWMDeleteMessage() const { return _WMDeleteMessage; }
  bool IsBorderless() const { return _Borderless; }
  bool IsDirty() const { return _Dirty; }
  bool Matches(Window xwin) const { return xwin == _XWindow; }

public:
  int GetViewParms(float* centerx, float* centery, float* zoomlevel,
                   float* xrotation, float* zrotation) const;
  int SetViewParms(float centerx, float centery, float zoomlevel,
                   float xrotation, float zrotation);

/**
 * The following public methods must be called only from the UI thread.
 */
public:
  int GetMousePos(int* x, int* y) const;
  int GetMousePosInModel(float *x, float *y) const;
  int HandleXButtonPress(XEvent event);
  int HandleXButtonRelease(XEvent event);
  int HandleXConfigureNotify(XEvent event);
  int HandleXExpose(XEvent event);
  int HandleXKeyPress(XEvent event);
  int HandleXKeyRelease(XEvent event);
  int HandleXMotionNotify(XEvent event);
  int RenderModel();

protected:
  int _X, _Y, _Width, _Height;
  bool _Borderless;
  char _Title[256];
  Window _XWindow;
  GLXContext _GLXContext;
  Atom _WMDeleteMessage;

  Model* _Model;
  float _CenterX, _CenterY;
  float _ZoomLevel, _ZoomFactor;
  float _XRotation, _ZRotation;

  bool _LeftMouseDown, _CtrlDown, _AltDown;
  int _MouseDownX, _MouseDownY;
  float _MouseDownCenterX, _MouseDownCenterY;
  int _CtrlDownX, _CtrlDownY;
  int _AltDownX, _AltDownY;
  float _CtrlDownXRotation, _AltDownZRotation;

  bool _Dirty;
};

#endif
