//Copyright 2011, 2016 <>< Charles Lohr - File may be licensed under MIT/x11 or New BSD You choose.

#ifndef _TRAY_H
#define _TRAY_H

#include <stdint.h>

//Platform-indepenedent system tray tool.

#define MAX_TRAY_MENU 20

//Call this first
void TrayInitialize( int * argc, char *** argv );

//Add your items
int TrayAddItem( const char * name, int is_checkbox, void (*cb)( void *, int ), void * id );

//Actually attach a callback to a tray click.
void SetTrayClickCallback( void (*cb)( void *, int ), void * id );

//Actually launch the tray.  This is a blocking call.  Expects data in RGBA
void TrayLaunch( const char * appname, uint8_t * pixeldata, int w, int h );


//Additional features
void TraySetChecked( int itemID, int set );
int TrayGetChecked( int itemID );
void TrayChangeItemText( int itemID, const char * newLabel );

void TermTray();

#endif

