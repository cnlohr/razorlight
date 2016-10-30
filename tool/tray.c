//Copyright 2011, 2016 <>< Charles Lohr - File may be licensed under MIT/x11 or New BSD You choose.

//Platform-indepenedent system tray tool.

#include "tray.h"
#include "os_generic.h"
#include <string.h>

#ifdef WIN32


//Windows code mostly from http://ivoronline.com/Coding/Languages/C/Tutorials/C%20-%20WinAPI%20-%20Window%20-%20With%20Tray%20Icon%20-%20With%20Popup%20Menu.php

#include <stdio.h>
#include <windows.h>

#define CLASSNAME "trayClass"
#define WINDOWNAME "windowName"

static const GUID myGUID = {0xf91eb8ff, 0x6d9f, 0x48f9, {0x9d, 0xcf, 0xdc, 0x33, 0xf6, 0x0b, 0xf5, 0x3c}};
static WNDCLASSEX wclx; 
static HICON icon;
static char apptitle[256];

static void (*tcCb)( void *, int );
static void * tcId;

struct TrayEntry
{
	int isCheck;
	int checked;
	char name[256];
	void (*cb)( void *, int );
	void * id; //user ID
	int lid; //local ID
};

static struct TrayEntry tr[MAX_TRAY_MENU];
int trayEntries;

BOOL ShowPopupMenu( HWND hWnd, POINT *curpos, int wDefaultItem ) {
	HMENU hPop = CreatePopupMenu();
	int i;

	//XXX Special: /0/ is a special "cancel" number.
	for( i = 1; i <= trayEntries; i++ )
	{
		struct TrayEntry * t = &tr[i];
		InsertMenu( hPop, i, MF_BYPOSITION | MF_STRING | (t->checked?MF_CHECKED:0), i, t->name );		
	}

	SetMenuDefaultItem( hPop, 0, FALSE );
	SetFocus          ( hWnd );
	SendMessage       ( hWnd, WM_INITMENUPOPUP, (WPARAM)hPop, 0 );

	{
		POINT pt;
		if (!curpos) {
			GetCursorPos( &pt );
			curpos = &pt;
		}

		{
			WORD cmd = TrackPopupMenu( hPop, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, curpos->x, curpos->y, 0, hWnd, NULL );
			SendMessage( hWnd, WM_COMMAND, cmd, 0 );
		}
	}

	DestroyMenu(hPop);

	return 0;
}

UINT m_nWMTaskBarCreated;

NOTIFYICONDATA data = {sizeof(data)};

void TermTray()
{
	int r = Shell_NotifyIcon(NIM_DELETE, &data);
	printf( "Close tray\n" );
}

static LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) { 
	//printf( "UMSG: %04x %04x\n", uMsg, m_nWMTaskBarCreated );
	switch (uMsg) {
	case 0x031f: //Not needed?
		//ShowWindow (hWnd, SW_SHOW);
		printf( "Non-client rendering policy changed.\n" );
		return 0;
	case WM_CREATE:
	{
		data.uFlags = NIF_TIP | NIF_MESSAGE | NIF_GUID | NIF_ICON;
		data.guidItem = myGUID;//__uuidof(NotifyIconGuid);
		data.hWnd = hWnd;
		data.uID = 1;
		data.uCallbackMessage = WM_APP;
		data.hIcon = icon ;
		strcpy( data.szTip, apptitle );
		data.uVersion = NOTIFYICON_VERSION;
		int r = Shell_NotifyIcon(NIM_ADD, &data);
		printf( "Shell: %d\n", r );
		return 0;
	}
	case WM_CLOSE:
		//TODO: Remove icon?
		PostQuitMessage(0);
		return DefWindowProc( hWnd, uMsg, wParam, lParam );
	case WM_COMMAND:
	{
		//printf( "%04x %04x %04x %04x\n", hWnd, uMsg, wParam, lParam );
		struct TrayEntry * t = &tr[wParam];
		if( t->isCheck ) t->checked = ! t->checked;
		if( t->cb ) t->cb( t->id, wParam );
		return 0;
	}
	case WM_APP:
		switch (lParam) {
			case WM_LBUTTONDOWN:
				if( tcCb ) tcCb( tcId, -1 );
				return 0;
			case WM_RBUTTONUP:
				SetForegroundWindow( hWnd );
				ShowPopupMenu(hWnd, NULL, -1 );
				PostMessage( hWnd, WM_APP + 1, 0, 0 );
			return 0;
		}
		return 0;
	}

	return DefWindowProc( hWnd, uMsg, wParam, lParam );
}


void TrayInitialize( int * argc, char *** argv )
{
	{
	//REGISTER WINDOW.--------------------------------------------------------------------------
		memset(&wclx, 0, sizeof(wclx));

		wclx.cbSize         = sizeof( wclx );
		wclx.style          = 0;
		wclx.lpfnWndProc    = &WndProc;
		wclx.cbClsExtra     = 0;
		wclx.cbWndExtra     = 0;
		wclx.hInstance      = GetModuleHandle( 0 );
		wclx.hCursor        = LoadCursor( NULL, IDC_ARROW );
		wclx.hbrBackground  = (HBRUSH)( COLOR_BTNFACE + 1 );   
				                                        
		wclx.lpszMenuName   = NULL;
		wclx.lpszClassName  = CLASSNAME;
		RegisterClassEx( &wclx );
	}
}

int TrayAddItem( const char * name, int is_checkbox, void (*cb)( void *, int ), void * id )
{
	trayEntries++;
	struct TrayEntry * t = &tr[trayEntries];
	t->isCheck = is_checkbox;
	t->checked = 0;
	t->cb = cb;
	t->id = id;
	t->lid = trayEntries;
	strncpy( t->name, name, sizeof( t->name ) - 1 );
	return trayEntries;

}

void SetTrayClickCallback( void (*cb)( void *, int ), void * id )
{
	tcCb = cb;
	tcId = id;
}

void TrayLaunch( const char * appname, uint8_t * pixeldata, int w, int h )
{
	strncpy( apptitle, appname, 254 );
	HDC dc = CreateCompatibleDC(NULL);
	HBITMAP hbm = CreateBitmap( w, h, 1, 32, pixeldata ); 
	ICONINFO ii = {0};
	ii.fIcon    = TRUE;
	ii.hbmColor = hbm;
	ii.hbmMask  = hbm;
	icon = CreateIconIndirect(&ii);
	DeleteObject(hbm);

	int wnd = CreateWindow( CLASSNAME, WINDOWNAME, WS_OVERLAPPEDWINDOW, 100,
		100, 250, 250, NULL, NULL, GetModuleHandle(0), NULL );

	printf( "WND: %d\n", wnd );
	m_nWMTaskBarCreated = RegisterWindowMessage("TaskbarCreated");

	while(1)
	{ 
		MSG msg;
		int ret;
		while (ret = GetMessage ( &msg, NULL, 0, 0 ) ) {
			//printf( "ret: %d\n", ret );
			TranslateMessage( &msg );
			DispatchMessage ( &msg );
		}
		printf( "Dispatch failed.\n" );
    }
}




//Additional features
void TraySetChecked( int itemID, int set )
{
	tr[itemID].checked = set;
}

int TrayGetChecked( int itemID )
{
	return tr[itemID].checked;
}

void TrayChangeItemText( int itemID, const char * newLabel )
{
	strncpy( tr[itemID].name, newLabel, sizeof( tr[itemID].name ) - 1 );
}



#else

#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdint.h>

//GTK Tray
//Strongly based off of: http://blog.sacaluta.com/2007/08/gtk-system-tray-icon-example.html
//Also off of: http://www.gtk.org/tutorial1.2/gtk_tut-13.html
//And.. http://developer.gnome.org/gtk-tutorial/2.90/c1494.html

static GtkWidget * m;
static GtkStatusIcon *tray_icon;
static int SuppressCheck = 0;

void tray_icon_on_menu(GtkStatusIcon *status_icon, guint button, 
                       guint activate_time, gpointer user_data)
{
	gtk_menu_popup (GTK_MENU(m), NULL, NULL, NULL, NULL, button, activate_time);
}

static void (*tcCb)( void *, int );
static void * tcId;

void tray_icon_on_click(GtkStatusIcon *status_icon, 
                        gpointer user_data)
{
	if( SuppressCheck ) return;
	if( tcCb ) tcCb( tcId, -1 );
}

struct TrayEntry
{
	GtkWidget * item;
	int isCheck;
	
	void (*cb)( void *, int );
	void * id; //user ID
	int lid; //local ID
};

struct TrayEntry te[MAX_TRAY_MENU];
int trayEntries;


static void menuitem_response (struct TrayEntry * s)
{
	if( SuppressCheck ) return;
	if( s->cb ) s->cb( s->id, s->lid );
}

void TrayInitialize( int * argc, char *** argv )
{
	GtkStatusIcon *tray_icon;
	gtk_init(argc, argv);
	m = gtk_menu_new();
}

int TrayAddItem( const char * name, int is_checkbox, void (*cb)( void *, int ), void * id )
{
	struct TrayEntry * t = &te[trayEntries];

	if( is_checkbox )
	{
		t->item = gtk_check_menu_item_new_with_label(name);
		t->isCheck = 1;
	}
	else
	{
		t->item = gtk_menu_item_new_with_label(name);
		t->isCheck = 0;
	}
	t->cb = cb;
	t->id = id;
	t->lid = trayEntries;

	gtk_menu_shell_append (GTK_MENU_SHELL (m), t->item);
	g_signal_connect_swapped (t->item, "activate", G_CALLBACK (menuitem_response), (gpointer)t);
	gtk_widget_show (t->item);

	return trayEntries++;
}

void SetTrayClickCallback( void (*cb)( void *, int ), void * id )
{
	tcCb = cb;
	tcId = id;
}

void TrayLaunch( const char * tooltip, uint8_t * colordata, int w, int h )
{
	tray_icon = gtk_status_icon_new();
	g_signal_connect(G_OBJECT(tray_icon), "activate", G_CALLBACK(tray_icon_on_click), NULL);
	g_signal_connect(G_OBJECT(tray_icon), "popup-menu", G_CALLBACK(tray_icon_on_menu), NULL);

	GdkPixbuf * pixbuf = gdk_pixbuf_new_from_data (colordata, /*COLORSPACE_RGB*/0, 1, 8, w, h, w*4, 0, 0 );
	gtk_status_icon_set_from_pixbuf(tray_icon, pixbuf);
	gtk_status_icon_set_tooltip(tray_icon, tooltip);
	gtk_status_icon_set_visible(tray_icon, TRUE);
	gtk_main();
}


void TraySetChecked( int itemID, int set )
{
	SuppressCheck = 1;
	gtk_check_menu_item_set_active( (GtkCheckMenuItem*)te[itemID].item, set); 
	SuppressCheck = 0;
}

int TrayGetChecked( int itemID )
{
	int ret;
	ret = gtk_check_menu_item_get_active((GtkCheckMenuItem*)te[itemID].item); 
}

void TrayChangeItemText( int itemID, const char * newLabel )
{
	gtk_menu_item_set_label( (GtkMenuItem*)te[itemID].item, newLabel ); 
}

void TermTray()
{
	//No need to do this in Linux.
}

#endif

