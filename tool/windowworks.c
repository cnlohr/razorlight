#include "windowworks.h"
#include "parameters.h"
#include <string.h>
#include <stdio.h>

int focusWindowWidth;
int focusWindowHeight;
char focusName[256];
int focusWindowAvailable = 0;

uint32_t PixelBuffer[MAX_PIXEL_BUFFER];

#ifdef WIN32

#include <windows.h>

HWND focusWindow;

void StartPolling()
{
	RECT r, cr;
	focusWindow = GetForegroundWindow();
	if( focusWindow )
	{
		GetClientRect( focusWindow, &r );
		focusWindowWidth = r.right - r.left;
		focusWindowHeight = r.bottom - r.top;
		focusName[0] = 0;
		GetWindowText( focusWindow, focusName, sizeof( focusName ) );
		focusWindowAvailable = 1;
	}
	else
		focusWindowAvailable = 0;
}

HWND focusWindowLast = 0;
static HDC screen;
static HDC target;
static HBITMAP bmp;
static lastw = 0;
static lasth = 0;

int GetPixelRange( int x, int y, int w, int h )
{
	if( w * h > MAX_PIXEL_BUFFER || !focusWindow ) return;

	if( !target || !screen )
	{
	}
	if( focusWindowLast != focusWindow || lastw != focusWindowWidth || lasth != focusWindowHeight )
	{
		if( target ) DeleteDC( target );
		screen = GetDC(focusWindow);
		target = CreateCompatibleDC(screen);

		if( bmp )
		{
			DeleteObject( bmp );
		}

		bmp = CreateCompatibleBitmap(screen, focusWindowWidth, focusWindowHeight);

		SelectObject(target, bmp);

		focusWindowLast = focusWindow;
		lastw = focusWindowWidth;
		lasth = focusWindowHeight;
	}
//	SIZE dim;
//	GetBitmapDimensionEx( bmp, &dim );
//	printf( "%d %d %d %d,%d %d,%d\n", focusWindow, focusWindowWidth, focusWindowHeight, x, y, w, h );

	int r = BitBlt(target, 0, 0, w, h, screen, x, y, SRCCOPY );// | CAPTUREBLT); WHY Does CaptureBLT make mouse flicker 
	if( r == 0 ) return -1;

    BITMAPINFO bminfo;
    bminfo.bmiHeader.biBitCount = 32;
    bminfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bminfo.bmiHeader.biCompression = BI_RGB;
    bminfo.bmiHeader.biPlanes = 1;
    bminfo.bmiHeader.biWidth = w;
    bminfo.bmiHeader.biHeight = h;
    bminfo.bmiHeader.biSizeImage = w * 4 * h; // must be DWORD aligned
    bminfo.bmiHeader.biXPelsPerMeter = 1;
    bminfo.bmiHeader.biYPelsPerMeter = 1;
    bminfo.bmiHeader.biClrUsed = 0;
    bminfo.bmiHeader.biClrImportant = 0;

	int s = GetDIBits(target, bmp, 0, h, PixelBuffer, &bminfo, DIB_RGB_COLORS);
//	printf( "Focus Name: %s  %d %d %d %d %06x\n", focusName, w, h, r, s, PixelBuffer[100] );

	if( s == 0 ) return -2;
//	printf( "%3d %3d  %3d %3d  %d %d -> %06x %s\n",r,s,x,y,w,h,PixelBuffer[0],focusName );

	return 0;
}


#else


#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xatom.h>

Display *display = 0;
Screen * screen = 0;
Window rootWindow = 0;
Window focusWindow = 0;

static int pass = 1;

static int handler(Display * d, XErrorEvent *e	)
{
	pass = 0;
	printf( "FAIL: %p\n", d ) ;
}

void StartPolling()
{
	pass = 1;
	static int did_set_handler;

	if( !did_set_handler )
	{
		XSetErrorHandler( handler );
	}

	Window foo;
	Window win = 0;
	int bar;
	unsigned int i;

	if( !display )
	{
		display = XOpenDisplay( 0 );
	   	screen=XDefaultScreenOfDisplay(display);
		rootWindow = DefaultRootWindow(display);
	}

	int revert_to;
	XGetInputFocus(display, &win, &revert_to);
	if( !win || !pass )
	{
		focusWindowAvailable = 0;
		focusWindow = 0;
		return;
	}

	Window w = win;
	Window parent = win;
	Window root = None;
	Window *children;
	unsigned int nchildren;
	Status s;
	char * window_return_name = 0;
	char * chain_good_name = 0;

	while (parent != root) {
		w = parent;
		s = XQueryTree(display, w, &root, &parent, &children, &nchildren); // see man
		if (children) XFree(children);
		int r = XFetchName(display, w, &window_return_name);


		if( window_return_name )
		{
			if( chain_good_name ) XFree( chain_good_name );
			win = w;
			chain_good_name = window_return_name;
		}
	}
	if( !pass ) { focusWindow = 0; }

	focusWindow = w;

	XWindowAttributes xwa;
	XGetWindowAttributes(display, focusWindow, &xwa);

	if( chain_good_name )
	{
		strncpy( focusName, chain_good_name, sizeof( focusName ) );
		XFree( chain_good_name );
	}

	focusWindowAvailable = 1;
	focusWindowWidth = xwa.width;
	focusWindowHeight = xwa.height;
}


int GetPixelRange( int x, int y, int w, int h )
{
	int lx, ly;
	if( !focusWindow ) return -7;
	pass = 1;
	XImage * m = XGetImage( display, focusWindow, x, y, w, h, AllPlanes, XYPixmap);
	if( !pass )
	{
		return -2;
	}
	if( m->bitmap_pad != 32 ) return -1;
	uint32_t * pb = PixelBuffer;
	for( ly = 0; ly < h; ly++ )
	{
		for( lx = 0; lx < w; lx++ )
		{
			uint32_t px = XGetPixel( m, lx, ly );
			(*(pb++)) = (px>>16) | (px&0xff00) | ((px&0xff)<<16);
		}
	}
	XDestroyImage( m );
	return 0;
}

#endif


