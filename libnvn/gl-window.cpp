/**
	 gl-window.cpp - Created by Timothy Morey on 1/12/2013
 */


#include "nvn.h"

#include "communication-queue.h"
#include "gl-window.hpp"
#include "Layer.hpp"
#include "Model.hpp"
#include "ReferenceFrameLayer.hpp"

#define MPICH_SKIP_MPICXX 1
#include <mpi.h>

#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>


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
 * Static Member Initialization
 ******************************************************************************/

GLWindow* GLWindow::_Window = 0;
CommunicationQueue GLWindow::_UIQueue;
pthread_t GLWindow::_UIThread;
bool GLWindow::_UIThreadActive = false;
bool GLWindow::_KeepUIThreadActive = false;
Display* GLWindow::_Display = 0;
XVisualInfo* GLWindow::_VisualInfo = 0;


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
  if(! _UIThreadActive)
  {
    printf("Starting UI thread...\n");

    _Window = 0;
    _KeepUIThreadActive = true;
    InitQueue(&_UIQueue);

    if(0 != pthread_create(&_UIThread, 0, UIThreadEntryPoint, 0))
    {
      fprintf(stderr, "Failed to create UI thread.\n");
    }
  }

  if(pthread_equal(_UIThread, pthread_self()))
  {
    // We are already on the UI thread, so create the window directly.
    this->CreateWindow();
  }
  else
  {
    Message msg;
    int handled = 0;

    InitMessage(&msg, "CreateWindow");
    msg.Arguments[0] = this;
    msg.Handled = &handled;
    Push(&_UIQueue, msg);

    // Block until the window is created
    while(! handled)
      pthread_yield();
  }
}

GLWindow::~GLWindow()
{
  if(this->IsActive())
    this->CloseWindow();

  // If all windows are closed, then shutdown the UI thread.
  if(0 == _Window)
  {
    _KeepUIThreadActive = false;
    pthread_join(_UIThread, 0);
  }
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

int GLWindow::CloseWindow()
{
  if(pthread_equal(_UIThread, pthread_self()))
  {
    // We are already on the UI thread, so destroy the window directly
    this->DestroyWindow();
  }
  else
  {
    Message msg;
    int handled = 0;

    InitMessage(&msg, "DestroyWindow");
    msg.Arguments[0] = this;
    msg.Handled = &handled;
    Push(&_UIQueue, msg);

    // Block until the window is created
    while(! handled)
      pthread_yield();
  }
}

int GLWindow::ResetView()
{
  int retval = NVN_NOERR;

  if(_Model)
  {
    NVN_BBox bounds = _Model->GetBounds();
    float width = bounds.Max[XDIM] - bounds.Min[XDIM];
    float height = bounds.Max[YDIM] - bounds.Min[YDIM];

    _CenterX =  width / 2.0f;
    _CenterY = height / 2.0f;
    _ZoomLevel = 1.0f;
    _XRotation = 0.0f;
    _ZRotation = 0.0f;
    this->AsyncRefresh();
  }

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

bool GLWindow::IsActive() const
{
  return _Window != 0;
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
 * 
 * NOTE: It is assumed that these functions are always run on the UI thread.
 ******************************************************************************/

int GLWindow::CreateWindow()
{
  int retval = NVN_NOERR;

  XSetWindowAttributes windowAttrs;
  Colormap colormap;

  printf("Creating window...\n");

  _GLXContext = glXCreateContext(_Display, _VisualInfo, 0, 1);
  if(! _GLXContext)
  {
    this->DestroyWindow();
    retval = NVN_EGLXFAIL;
    fprintf(stderr, "Unable to create a GLX context.");
  }

  if(NVN_NOERR == retval)
  {
    colormap = XCreateColormap(_Display,
        RootWindow(_Display, _VisualInfo->screen),
        _VisualInfo->visual,
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

    _XWindow = XCreateWindow(_Display,
        RootWindow(_Display, _VisualInfo->screen),
        _X, _Y, _Width, _Height,
        0,
        _VisualInfo->depth,
        InputOutput,
        _VisualInfo->visual,
        // CWOverrideRedirect can help when spanning multiple monitors...
        //CWBorderPixel | CWColormap | CWEventMask | CWOverrideRedirect,
        CWBorderPixel | CWColormap | CWEventMask,
        &windowAttrs);

    XSetStandardProperties(_Display,
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
      prop = XInternAtom(_Display, "_MOTIF_WM_HINTS", True);
      XChangeProperty(_Display,
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
    _WMDeleteMessage = XInternAtom(_Display, "WM_DELETE_WINDOW", false);
    XSetWMProtocols(_Display, _XWindow, &_WMDeleteMessage, 1);
  }

  if(NVN_NOERR == retval)
  {
    glXMakeCurrent(_Display, _XWindow, _GLXContext);
    XMapWindow(_Display, _XWindow);
    glClearColor(0.0, 0.0, 0.0, 1.0);

    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    _Window = this;
  }

  return retval;
}

int GLWindow::DestroyWindow()
{
  printf("Destroying window...\n");

  XDestroyWindow(_Display, _XWindow);
  _Window = 0;
}

int GLWindow::GetMousePos(int* x, int* y) const
{
  int retval = NVN_NOERR;

  Window root, child;
  int rootx, rooty, winx, winy;
  unsigned int mask;
  if(XQueryPointer(_Display, _XWindow, &root, &child,
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
  }
  else
  {
    // If we've got depth, then do a combination of ambient and diffuse
    // lighting to show off the depth.

    float lightpos[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    float diffcolor[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    float ambcolor[] = { 0.5f, 0.5f, 0.5f, 1.0f };

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambcolor);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffcolor);
    glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
  }

  glTranslatef(_CenterX, _CenterY, 0.0f);
  glRotatef(_XRotation, 1.0f, 0.0f, 0.0f);
  glRotatef(_ZRotation, 0.0f, 0.0f, 1.0f);
  glTranslatef(-_CenterX,-_CenterY, 0.0f);

  if(_Model)
    _Model->Render();

  ReferenceFrameLayer frame(_Model->GetCRS(), _Model->GetBounds());
  frame.Render();

  glXSwapBuffers(_Display, _XWindow);

  _Dirty = false;
}

/******************************************************************************
 * Static Members:
 ******************************************************************************/

int GLWindow::InitUIThread()
{
  int retval = NVN_NOERR;

  int doubleBufferVisual[] =  /* A description of the GLX visual we want. */
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

int GLWindow::RunMessageLoop()
{
  int retval = NVN_NOERR;

  XEvent event;
  Message msg;
  int valid;
  GLWindow* win;
  double renderStart;
  double renderElapsed = 0.0;
  int renderCount = 0;
  bool userModified = false;

  int rank, commsize;
  MPI_Comm_size(MPI_COMM_WORLD, &commsize);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  float sendbuf[6];
  float* recvbuf = (float*)malloc(6 * commsize);

  _UIThreadActive = true;

  while(_KeepUIThreadActive)
  {
    if(_Window && XPending(_Display)) // Give X11 messages highest priority, and process
    {                      // all if there are any, to avoid a long queue.
      float prevCX = _Window->_CenterX;
      float prevCY = _Window->_CenterY;
      float prevZoom = _Window->_ZoomLevel;
      float prevRX = _Window->_XRotation;
      float prevRZ = _Window->_ZRotation;

      while(XPending(_Display))
      {
        XNextEvent(_Display, &event);
        switch(event.type)
        {
        case ButtonPress:
          _Window->HandleXButtonPress(event);
          break;

        case ButtonRelease:
          _Window->HandleXButtonRelease(event);
          break;

        case ClientMessage:
          if(event.xclient.data.l[0] == _Window->_WMDeleteMessage)
            _Window->DestroyWindow();
          break;

        case ConfigureNotify:
          _Window->HandleXConfigureNotify(event);
          break;

        case Expose:
          _Window->HandleXExpose(event);
          break;

        case KeyPress:
          _Window->HandleXKeyPress(event);
          break;

        case KeyRelease:
          _Window->HandleXKeyRelease(event);
          break;

        case MotionNotify:
          _Window->HandleXMotionNotify(event);
          break;
        }
      }

      if(_Window->_CenterX != prevCX ||
         _Window->_CenterY != prevCY ||
         _Window->_ZoomLevel != prevZoom ||
         _Window->_XRotation != prevRX ||
         _Window->_ZRotation != prevRZ)
      {
        userModified = true;
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
          ret = ((GLWindow*)msg.Arguments[0])->CreateWindow();

          if(msg.Handled)
            *msg.Handled = 1;
        }
        else if(0 == strcmp("DestroyWindow", msg.Message))
        {
          ret = ((GLWindow*)msg.Arguments[0])->DestroyWindow();

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
      // Sync center and zoom before drawing, so all render the same scene
      sendbuf[0] = userModified ? 1.0f : 0.0f;
      sendbuf[1] = _Window->_CenterX;
      sendbuf[2] = _Window->_CenterY;
      sendbuf[3] = _Window->_ZoomLevel;
      sendbuf[4] = _Window->_XRotation;
      sendbuf[5] = _Window->_ZRotation;
      MPI_Allgather(sendbuf, 6, MPI_FLOAT, recvbuf, 6, MPI_FLOAT, MPI_COMM_WORLD);

      if(! userModified)
      {
        for(int i = 0; i < commsize; i++)
        {
          if(recvbuf[i*6] > 0.0f)
          {
            _Window->_CenterX = recvbuf[i*6 + 1];
            _Window->_CenterY = recvbuf[i*6 + 2];
            _Window->_ZoomLevel = recvbuf[i*6 + 3];
            _Window->_XRotation = recvbuf[i*6 + 4];
            _Window->_ZRotation = recvbuf[i*6 + 5];
            _Window->_Dirty = true;
          }
        }
      }

      userModified = false;

      if(_Window->_Dirty)
      {
        renderStart = MPI_Wtime();

        _Window->RenderModel();

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

  _UIThreadActive = false;

  return retval;
}

int GLWindow::ShutdownUIThread()
{
  int retval = NVN_NOERR;

  if(_Window)
  {
    _Window->DestroyWindow();
    _Window = 0;
  }

  if(_Display)
  {
    XCloseDisplay(_Display);
    _Display = 0;
  }
}

void* GLWindow::UIThreadEntryPoint(void* arg)
{
  if(NVN_NOERR == InitUIThread())
  {
    RunMessageLoop();
    ShutdownUIThread();
  }
}
