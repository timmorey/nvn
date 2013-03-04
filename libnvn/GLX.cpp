/*
 * GLX.cpp
 *
 *  Created on: Mar 2, 2013
 *      Author: Timothy Morey
 */


#include "communication-queue.h"
#include "nvn.h"

#include "GLWindow.hpp"
#include "GLX.hpp"

#include <GL/glx.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>

#include <algorithm>
#include <list>


/******************************************************************************
 * Static Member Initialization
 ******************************************************************************/


GLX* GLX::_Instance = 0;


/******************************************************************************
 * Public Members:
 ******************************************************************************/

GLX::GLX()
  : _Display(0),
    _VisualInfo(0),
    _UIThreadAlive(false),
    _FocusWindow(0)
{
  InitQueue(&_UIQueue);
  _KeepUIThreadAlive = true;

  if(0 != pthread_create(&_UIThread, 0, UIThreadEntryPoint, 0))
  {
    fprintf(stderr, "Failed to create UI thread.\n");
  }
}

GLX::~GLX()
{
  if(_UIThreadAlive)
  {
    _KeepUIThreadAlive = false;
    pthread_join(_UIThread, 0);
  }
}

int GLX::CreateWindow(const char* title, int x, int y, int width, int height,
                      bool borderless, GLWindow** window)
{
  int retval = NVN_NOERR;

  if(! _Instance)
    GLX::Init();

  if(pthread_equal(_Instance->_UIThread, pthread_self()))
  {
    // We are already on the UI thread, so create the window directly.
    *window = new GLWindow(title, x, y, width, height, borderless);
    _Instance->_Windows.push_back(*window);
  }
  else
  {
    Message msg;
    int handled = 0;

    InitMessage(&msg, "CreateWindow");
    msg.Arguments[0] = &title;
    msg.Arguments[1] = &x;
    msg.Arguments[2] = &y;
    msg.Arguments[3] = &width;
    msg.Arguments[4] = &height;
    msg.Arguments[5] = &borderless;
    msg.Arguments[6] = window;
    msg.Handled = &handled;
    msg.Result = &retval;
    Push(&_Instance->_UIQueue, msg);

    // Block until the window is created
    while(! handled)
      pthread_yield();
  }

  return retval;
}

int GLX::DestroyWindow(GLWindow* window)
{
  int retval = NVN_NOERR;

  if(! _Instance)
    GLX::Init();

  if(pthread_equal(_Instance->_UIThread, pthread_self()))
  {
    if(_Instance->_Windows.end() !=
        std::find(_Instance->_Windows.begin(), _Instance->_Windows.end(), window))
    {
      _Instance->_Windows.remove(window);
      delete window;
    }
  }
  else
  {
    Message msg;
    int handled = 0;

    InitMessage(&msg, "DestroyWindow");
    msg.Arguments[0] = window;
    msg.Handled = &handled;
    msg.Result = &retval;
    Push(&_Instance->_UIQueue, msg);

    // Block until the window is created
    while(! handled)
      pthread_yield();
  }

  return retval;
}

Display* GLX::GetDisplay()
{
  Display* retval = 0;

  if(! _Instance)
    GLX::Init();

  retval = _Instance->_Display;
  return retval;
}

XVisualInfo* GLX::GetVisualInfo()
{
  XVisualInfo* retval = 0;

  if(! _Instance)
    GLX::Init();

  retval = _Instance->_VisualInfo;
  return retval;
}

int GLX::Init()
{
  int retval = NVN_NOERR;

  if(! _Instance)
  {
    _Instance = new GLX();
  }

  return retval;
}

bool GLX::IsActive(GLWindow* window)
{
  return 0 != window &&
      _Instance->_Windows.end() !=
          std::find(_Instance->_Windows.begin(), _Instance->_Windows.end(), window);
}

int GLX::IsInitialized()
{
  return 0 != _Instance;
}

int GLX::Shutdown()
{
  int retval = NVN_NOERR;

  if(_Instance)
  {
    delete _Instance;
    _Instance = 0;
  }

  return retval;
}


/******************************************************************************
 * Protected Members:
 ******************************************************************************/

int GLX::HandleFocusIn(XEvent event)
{
  int retval = NVN_NOERR;

  std::list<GLWindow*>::iterator iter;
  for(iter = _Windows.begin(); iter != _Windows.end(); iter++)
  {
    if((*iter)->Matches(event.xfocus.window))
    {
      _FocusWindow = *iter;
      break;
    }
  }

  return retval;
}

int GLX::HandleFocusOut(XEvent event)
{
  int retval = NVN_NOERR;

  if(_FocusWindow && _FocusWindow->Matches(event.xfocus.window))
    _FocusWindow = 0;

  return retval;
}

int GLX::InitUIThread()
{
  int retval = NVN_NOERR;

  int doubleBufferVisual[] =
      {
          GLX_RGBA,
          GLX_DEPTH_SIZE, 16,
          GLX_DOUBLEBUFFER,
          None
      };

  _Display = XOpenDisplay(0);
  if(! _Display)
  {
    retval = NVN_EXWINFAIL;
    fprintf(stderr, "XOpenDisplay failed.\n");
  }
  else if(! glXQueryExtension(_Display, 0, 0))
  {
    retval = NVN_EGLXFAIL;
    fprintf(stderr, "X server doesn't have the OpenGL GLX extension.");
  }

  if(NVN_NOERR == retval)
  {
    _VisualInfo = glXChooseVisual(_Display,
        DefaultScreen(_Display),
        doubleBufferVisual);

    if(! _VisualInfo)
    {
      retval = NVN_EGLXFAIL;
      fprintf(stderr, "Unable to find an appropriate GLX visual.");
    }
  }

  return retval;
}

int GLX::RunMessageLoop()
{
  int retval = NVN_NOERR;

  XEvent event;
  Message msg;
  int valid;
  GLWindow* win;
  GLWindow* prevFocus;
  double renderStart;
  double renderElapsed = 0.0;
  int renderCount = 0;
  bool userModified = false;
  float prevCX, prevCY, prevRX, prevRZ, prevZoom;
  float newCX, newCY, newRX, newRZ, newZoom;

  int rank, commsize;
  MPI_Comm_size(MPI_COMM_WORLD, &commsize);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  float sendbuf[6];
  float* recvbuf = (float*)malloc(6 * commsize);

  _UIThreadAlive = true;

  while(_KeepUIThreadAlive)
  {
    if(XPending(_Display)) // Give X11 messages highest priority, and process
                           // all if there are any, to avoid a long queue.
    {
      prevFocus = _FocusWindow;
      if(_FocusWindow)
      {
        _FocusWindow->GetViewParms(&prevCX, &prevCY, &prevZoom, &prevRX, &prevRZ);
      }

      while(XPending(_Display))
      {
        XNextEvent(_Display, &event);
        switch(event.type)
        {
        case ButtonPress:
          if(_FocusWindow)
            _FocusWindow->HandleXButtonPress(event);
          break;

        case ButtonRelease:
          if(_FocusWindow)
            _FocusWindow->HandleXButtonRelease(event);
          break;

        case ClientMessage:
          if(_FocusWindow &&
             event.xclient.data.l[0] == _FocusWindow->GetWMDeleteMessage())
          {
            GLX::DestroyWindow(_FocusWindow);
            _FocusWindow = 0;
          }
          break;

        case ConfigureNotify:
          if(_FocusWindow)
            _FocusWindow->HandleXConfigureNotify(event);
          break;

        case Expose:
          if(_FocusWindow)
            _FocusWindow->AsyncRefresh();
          break;

        case FocusIn:
          this->HandleFocusIn(event);
          break;

        case FocusOut:
          this->HandleFocusOut(event);
          break;

        case KeyPress:
          if(_FocusWindow)
            _FocusWindow->HandleXKeyPress(event);
          break;

        case KeyRelease:
          if(_FocusWindow)
            _FocusWindow->HandleXKeyRelease(event);
          break;

        case MotionNotify:
          if(_FocusWindow)
            _FocusWindow->HandleXMotionNotify(event);
          break;
        }
      }

      if(_FocusWindow && _FocusWindow == prevFocus)
      {
        _FocusWindow->GetViewParms(&newCX, &newCY, &newZoom, &newRX, &newRZ);
        if(newCX != prevCX ||
           newCY != prevCY ||
           newZoom != prevZoom ||
           newRX != prevRX ||
           newRZ != prevRZ)
        {
          userModified = true;
        }
      }
    }
    else if(_UIQueue.Size > 0) // Local messages receive a lower priority
    {
      Pop(&_UIQueue, &msg, &valid);
      if(valid)
      {
        int ret = NVN_NOERR;

        if(0 == strcmp("CreateWindow", msg.Message))
        {
          const char* title = *(const char**)msg.Arguments[0];
          int x = *((int*)msg.Arguments[1]);
          int y = *((int*)msg.Arguments[2]);
          int width = *((int*)msg.Arguments[3]);
          int height = *((int*)msg.Arguments[4]);
          bool borderless = *((bool*)msg.Arguments[5]);
          GLWindow** window = ((GLWindow**)msg.Arguments[6]);
          ret = this->CreateWindow(title, x, y, width, height, borderless, window);

          if(msg.Handled)
            *msg.Handled = 1;
        }
        else if(0 == strcmp("DestroyWindow", msg.Message))
        {
          GLWindow* window = ((GLWindow*)msg.Arguments[0]);
          ret = this->DestroyWindow(window);

          if(msg.Handled)
            *msg.Handled = 1;
        }

        if(msg.Result)
          *((int*)msg.Result) = ret;

        if(msg.DestroyArgs)
          DestroyArguments(msg);
      }
    }
    else
    {
      // TODO: Need to rethink the viewport synchronization...
//      // Sync center and zoom before drawing, so all render the same scene
//      sendbuf[0] = userModified ? 1.0f : 0.0f;
//      sendbuf[1] = _FocusWindow->_CenterX;
//      sendbuf[2] = _FocusWindow->_CenterY;
//      sendbuf[3] = _FocusWindow->_ZoomLevel;
//      sendbuf[4] = _FocusWindow->_XRotation;
//      sendbuf[5] = _FocusWindow->_ZRotation;
//      MPI_Allgather(sendbuf, 6, MPI_FLOAT, recvbuf, 6, MPI_FLOAT, MPI_COMM_WORLD);
//
//      if(! userModified)
//      {
//        for(int i = 0; i < commsize; i++)
//        {
//          if(recvbuf[i*6] > 0.0f)
//          {
//            _FocusWindow->_CenterX = recvbuf[i*6 + 1];
//            _FocusWindow->_CenterY = recvbuf[i*6 + 2];
//            _FocusWindow->_ZoomLevel = recvbuf[i*6 + 3];
//            _FocusWindow->_XRotation = recvbuf[i*6 + 4];
//            _FocusWindow->_ZRotation = recvbuf[i*6 + 5];
//            _FocusWindow->_Dirty = true;
//          }
//        }
//      }

      userModified = false;

      std::list<GLWindow*>::iterator iter;
      for(iter = _Windows.begin(); iter != _Windows.end(); iter++)
      {
        if((*iter)->IsDirty())
        {
          renderStart = MPI_Wtime();

          (*iter)->RenderModel();

          renderElapsed += MPI_Wtime() - renderStart;
          renderCount ++;
          if(renderCount >= 100)
          {
            printf("Average render time: %f s\n", renderElapsed / renderCount);
            renderElapsed = 0.0;
            renderCount = 0;
          }
        }
      }
    }
  }

  _UIThreadAlive = false;

  return retval;
}

int GLX::ShutdownUIThread()
{
  int retval = NVN_NOERR;

  if(_Display)
  {
    XCloseDisplay(_Display);
    _Display = 0;
  }
}

void* GLX::UIThreadEntryPoint(void* arg)
{
  if(NVN_NOERR == _Instance->InitUIThread())
  {
    _Instance->RunMessageLoop();
    _Instance->ShutdownUIThread();
  }
}
