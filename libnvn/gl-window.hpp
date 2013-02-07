/**
	 gl-window.hpp - Created by Timothy Morey on 1/12/2013
 */

#ifndef __GL_WINDOW_HPP__
#define __GL_WINDOW_HPP__


#include "communication-queue.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include <GL/gl.h>
#include <GL/glx.h>


class Model;

class GLWindow
{
public:
  GLWindow(const char* title, int x, int y, int width, int height,
           bool borderless);
  ~GLWindow();

public:
  int AsyncRefresh();
  int CloseWindow();
  int ResetView();
  int ShowModel(Model* model);

public:
  bool IsActive() const;
  float GetPixelsPerModelUnit() const;
  int GetX() const { return _X; }
  int GetY() const { return _Y; }
  int GetWidth() const { return _Width; }
  int GetHeight() const { return _Height; }
  int IsBorderless() const { return _Borderless; }

protected:
  int CreateWindow();
  int DestroyWindow();
  int GetMousePos(int* x, int* y) const;
  int GetMousePosInModel(float *x, float *y) const;
  int RenderModel();

protected:
  int HandleXButtonPress(XEvent event);
  int HandleXButtonRelease(XEvent event);
  int HandleXConfigureNotify(XEvent event);
  int HandleXExpose(XEvent event);
  int HandleXKeyPress(XEvent event);
  int HandleXKeyRelease(XEvent event);
  int HandleXMotionNotify(XEvent event);

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

protected:
  static GLWindow* _Window;
  static pthread_t _UIThread;
  static bool _UIThreadActive;
  static bool _KeepUIThreadActive;
  static CommunicationQueue _UIQueue;
  static Display* _Display;
  static XVisualInfo* _VisualInfo;

  static int InitUIThread();
  static int RunMessageLoop();
  static int ShutdownUIThread();
  static void* UIThreadEntryPoint(void* arg);
};

#endif
