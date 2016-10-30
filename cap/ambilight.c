#include <stdio.h>
#include <windows.h>
#include "os_generic.h"
#include <d3d9.h>
#include <stdint.h>

HWND wnd = 0;
HDC screen;
HDC target;
uint32_t width;
uint32_t height;
HBITMAP bmp;


BOOL CALLBACK EnumWindowsProc( HWND hwnd, LONG lParam )
{
     CHAR buffer[512];
    SendMessage(hwnd,WM_GETTEXT,sizeof(buffer), (LPARAM)(void*)buffer);
    printf( "%s\n", buffer );
	if( strncmp( buffer, "The W", 5 ) == 0 ) wnd = hwnd;
    return TRUE;
}

void setup()
{
	EnumWindows(EnumWindowsProc, (LPARAM) 0);
	printf( "WND: %d\n", wnd );
	screen = GetDC(wnd);
	target = CreateCompatibleDC(screen);

	width = 10;//GetSystemMetrics(SM_CXSCREEN);
	height = 10;//GetSystemMetrics(SM_CYSCREEN);

	bmp = CreateCompatibleBitmap(screen, width, height);

	SelectObject(target, bmp);

	//DeleteObject( bmp );

}
uint32_t * bmpBuffer = 0;

unsigned int takeScreenShot(int startx, int starty,int endx,int endy, int skipPx ){
    int ret = 0;

    ret = BitBlt(target, 0, 0, width, height, screen, 50, 50, SRCCOPY | CAPTUREBLT);
printf( "RR1: %d\n", ret );
    BITMAPINFO bminfo;
    bminfo.bmiHeader.biBitCount = 32;
    bminfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bminfo.bmiHeader.biCompression = BI_RGB;
    bminfo.bmiHeader.biPlanes = 1;
    bminfo.bmiHeader.biWidth = width;
    bminfo.bmiHeader.biHeight = height;
    bminfo.bmiHeader.biSizeImage = width * 4 * height; // must be DWORD aligned
    bminfo.bmiHeader.biXPelsPerMeter = 0;
    bminfo.bmiHeader.biYPelsPerMeter = 0;
    bminfo.bmiHeader.biClrUsed = 0;
    bminfo.bmiHeader.biClrImportant = 0;
 
    // get bitmap info from the bitmap
    //ret = GetDIBits(mTarget, mBmp, 0, mHeight, NULL, (BITMAPINFO *) &bminfo, DIB_RGB_COLORS);
    //assert(ret);
     
    if( ! bmpBuffer ) bmpBuffer = malloc(bminfo.bmiHeader.biSizeImage);
 
    // real capture
    ret = GetDIBits(target, bmp, 0, height, bmpBuffer, &bminfo, DIB_RGB_COLORS);
printf( "RR2: %d\n", ret );
    printf( "%p ", bmpBuffer[0] );

//	free( bmpBuffer );


}
int main()
{
	COLORREF color=0xaaaaaaaa;
	int iter = 0;
	int i;
	setup();
    //HDC dc = GetDC(NULL);
	printf( "Starting...\n" );
	double st = OGGetAbsoluteTime();
/*	for( iter = 0; iter < 10; iter++ )
	{
		for( i = 0; i < 100; i++ )
		{
			color = GetPixel(screen, 0, i);
		}
	}*/
	double delt = OGGetAbsoluteTime() - st;
	printf( "Timing 1: %f\n", delt );
	st = OGGetAbsoluteTime();
	for( iter = 0; iter < 10; iter++ )
	{
		takeScreenShot(0, 0,1,1000, 0 );
	}
	double deltb = OGGetAbsoluteTime() - st;
	printf( "Timings: %08x -> %f %f\n", color, delt, deltb );
    ReleaseDC(NULL, screen);
	return 0;
}


