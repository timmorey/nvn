/*
 * GLX.hpp
 *
 *  Created on: Mar 2, 2013
 *      Author: Timothy Morey
 */

#ifndef __GLX_HPP__
#define __GLX_HPP__


#include "communication-queue.h"

#include "GLWindow.hpp"

#include <pthread.h>
#include <X11/Xlib.h>

#include <list>


/**
 * The GLX singleton object provides an interface to the X11 Glx system.  It
 * creates a UI thread that is capable of launching and managing multiple
 * GLWindows.
 */
class GLX
{
public:
  static int CreateWindow(const char* title,
                          int x, int y, int width, int height,
                          bool borderless, GLWindow** window);
  static int DestroyWindow(GLWindow* window);
  static Display* GetDisplay();
  static XVisualInfo* GetVisualInfo();
  static int Init();
  static bool IsActive(GLWindow* window);
  static int IsInitialized();
  static int Shutdown();

protected:
  GLX();
  ~GLX();

protected:
  int HandleFocusIn(XEvent event);
  int HandleFocusOut(XEvent event);
  int InitUIThread();
  int RunMessageLoop();
  int ShutdownUIThread();

protected:
  static void* UIThreadEntryPoint(void* arg);

protected:
  static GLX* _Instance;

protected:
  Display* _Display;
  XVisualInfo* _VisualInfo;
  pthread_t _UIThread;
  bool _UIThreadAlive;
  bool _KeepUIThreadAlive;
  CommunicationQueue _UIQueue;
  std::list<GLWindow*> _Windows;
  GLWindow* _FocusWindow;
};

#endif
