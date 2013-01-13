/**
	 gl-window.cpp - Created by Timothy Morey on 1/12/2013
 */

#include "communication-queue.h"
#include "gl-window.hpp"
#include "layer.hpp"
#include "model.hpp"
#include "nvn.h"

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

std::list<GLWindow*> GLWindow::_Windows;
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
		_Borderless(borderless)
{
	if(! _UIThreadActive)
	{
		printf("Starting UI thread...\n");

		_Windows.clear();
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
	if(_Windows.empty())
	{
		_KeepUIThreadActive = false;
		pthread_join(_UIThread, 0);
	}
}


/******************************************************************************
 * Public Members
 ******************************************************************************/

int GLWindow::AddLayer(Layer* layer)
{
	_Model.TheLayer = layer;
	this->Refresh();
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

bool GLWindow::IsActive() const
{
	bool retval = false;
	std::list<GLWindow*>::iterator iter;

	for(iter = _Windows.begin(); iter != _Windows.end(); iter++)
	{
		if(this == *iter)
		{
			retval = true;
			break;
		}
	}

	return retval;
}

int GLWindow::Refresh()
{
	int retval = NVN_NOERR;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if(_Model.TheLayer)
		_Model.TheLayer->Render();
	glXSwapBuffers(_Display, _XWindow);

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
		
		glShadeModel(GL_SMOOTH);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);
		glEnable(GL_CULL_FACE);
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glViewport(0, 0, _Width, _Height);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(60.0, (float)_Width / (float)_Height, 1.0, 1024.0);

		this->Refresh();

		_Windows.push_back(this);
	}

	return retval;
}

int GLWindow::DestroyWindow()
{
	printf("Destroying window...\n");

	XDestroyWindow(_Display, _XWindow);
	_Windows.remove(this);
}

int GLWindow::HandleXConfigureNotify(XEvent event)
{
	printf("Handling XConfigureNotify\n");

	_X = event.xconfigure.x;
	_Y = event.xconfigure.y;
	_Width = event.xconfigure.width;
	_Height = event.xconfigure.height;

	glViewport(0, 0, _Width, _Height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (float)_Width / (float)_Height, 1.0, 1024.0);
}

int GLWindow::HandleXExpose(XEvent event)
{
	printf("Handling XExpose...\n");
	this->Refresh();
}


/******************************************************************************
 * Static Members:
 ******************************************************************************/

GLWindow* GLWindow::FindWindow(Window xwindow)
{
	GLWindow* retval = 0;
	std::list<GLWindow*>::iterator iter;

	for(iter = _Windows.begin(); iter != _Windows.end(); iter++)
	{
		if((*iter)->_XWindow == xwindow)
		{
			retval = *iter;
			break;
		}
	}

	return retval;
}

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

	_UIThreadActive = true;

	while(_KeepUIThreadActive)
	{
		if(_UIQueue.Size > 0) // Local messages get highest priority
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
		else if(XPending(_Display)) // X11 messages get next highest priority
		{
			XNextEvent(_Display, &event);
			switch(event.type)
			{
			case ClientMessage:
				win = FindWindow(event.xclient.window);
				if(win && event.xclient.data.l[0] == win->_WMDeleteMessage)
					win->DestroyWindow();
				break;

			case ConfigureNotify:
				// The window geometry has changed
				win = FindWindow(event.xconfigure.window);
				if(win)
					win->HandleXConfigureNotify(event);
				break;

			case Expose:
				// The window contents needs to be re-drawn
				win = FindWindow(event.xexpose.window);
				if(win)
					win->HandleXExpose(event);
				break;
			}
		}		
	}

	_UIThreadActive = false;

	return retval;
}

int GLWindow::ShutdownUIThread()
{
	int retval = NVN_NOERR;

	std::list<GLWindow*>::iterator iter;
	for(iter = _Windows.begin(); iter != _Windows.end(); iter++)
	{
		(*iter)->DestroyWindow();
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
