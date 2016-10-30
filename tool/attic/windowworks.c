#error DO NOT USE THIS TOOL

#include "windowworks.h"
#include <stdio.h>

static void StartPolling();

static void WindowCallback( void * lid, const char * name, const char * altname )
{
	printf( "%p = %s %s\n", lid, name, altname );
}


void PollForWindows()
{
	StartPolling();
}


#ifdef WIN32

static void StartPolling()
{}


#else


#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xatom.h>

Display *display = 0;
Screen * screen = 0;
Window rootWindow = 0;

static void StartPolling()
{
	unsigned int i;
	if( !display )
	{
		display = XOpenDisplay( 0 );
	   	screen=XDefaultScreenOfDisplay(display);
		rootWindow = DefaultRootWindow(display);
	}

	Window root_return;
	Window parent_return;
	Window * children_return;
	Atom prop = XInternAtom(display,"WM_NAME",False);
	unsigned int nch = 0;

	Status r = XQueryTree(display, rootWindow, &root_return, &parent_return, &children_return, &nch);
	for( i = 0; i < nch; i++ )
	{
		Window c = children_return[i];
		if(  (c & 0xfffff ) != 0x00001  ) continue;
		char * window_return_name, * alternate_name;
		int r = XFetchName(display, c, &window_return_name);
		if( !r ) continue;

		Atom type;
		int form;
		unsigned long remain, len;
		unsigned char *list;
		if (XGetWindowProperty(display,c,prop,0,1024,False,XA_STRING,
				&type,&form,&len,&remain,&alternate_name) != Success)
		{
				alternate_name = 0;
		}

		WindowCallback( c, window_return_name, alternate_name );
		if( window_return_name ) XFree( window_return_name );
		if( alternate_name ) XFree( alternate_name );
	}
	printf( "%d %p\n", nch, children_return );
	if( children_return ) XFree( children_return );
}

#endif


