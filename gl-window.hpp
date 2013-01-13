/**
	 gl-window.hpp - Created by Timothy Morey on 1/12/2013
 */

#ifndef __GL_WINDOW_HPP__
#define __GL_WINDOW_HPP__


#include "communication-queue.h"
#include "model.hpp"

#include <list>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include <GL/gl.h>
#include <GL/glx.h>


class GLWindow
{
public:
	GLWindow(const char* title, int x, int y, int width, int height, 
						bool borderless);
	~GLWindow();

public:
	int AddLayer(Layer* layer);
	int CloseWindow();
	int Refresh();

public:
	bool IsActive() const;
	int GetX() const { return _X; }
	int GetY() const { return _Y; }
	int GetWidth() const { return _Width; }
	int GetHeight() const { return _Height; }
	int IsBorderless() const { return _Borderless; }

protected:
	int CreateWindow();
	int DestroyWindow();

	int HandleXConfigureNotify(XEvent event);
	int HandleXExpose(XEvent event);

protected:
	int _X, _Y, _Width, _Height;
	bool _Borderless;
	char _Title[256];
	Window _XWindow;
	GLXContext _GLXContext;
	Atom _WMDeleteMessage;
	Model _Model;

protected:
	static std::list<GLWindow*> _Windows;
	static pthread_t _UIThread;
	static bool _UIThreadActive;
	static bool _KeepUIThreadActive;
	static CommunicationQueue _UIQueue;
	static Display* _Display;
	static XVisualInfo* _VisualInfo;

	static GLWindow* FindWindow(Window xwindow);
	static int InitUIThread();
	static int RunMessageLoop();
	static int ShutdownUIThread();
	static void* UIThreadEntryPoint(void* arg);
};

#endif
