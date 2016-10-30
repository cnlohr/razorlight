#include <stdint.h>
#include <stdlib.h>
#include "tray.h"
#include "os_generic.h"
#include <stdio.h>
#include <string.h>
#include "urlopen.h"
#include "parameters.h"
#include "util.h"
#include "led_manage.h"

int httpok=0xff;
int interfaces = 0;
#define MAX_IFACES 16
int interface_elems[MAX_IFACES];
char interface_addresses[MAX_IFACES][32];
#define IFACE_OFFSET 10000

int close_me = 0;

int accept_new_cycle_wait( struct ParamSet * p, const char * st )
{
	int pst = atoi( st );
	if( pst > 1000 ) pst = 1000;
	if( pst < 0 ) pst = 0;
	*((int*)p->data) = pst;
}
REG_PARAM_I( cycle_wait_ms, 16, accept_new_cycle_wait );

REG_PARAM_I( cycles_per_window_poll, 1, 0 );

int accept_new_port( struct ParamSet * p, const char * st )
{
	int pst = atoi( st );
	if( pst > 65535 ) pst = 65535;
	if( pst < 1 ) pst = 1;
	*((int*)p->data) = pst;
}
REG_PARAM_I( gport, 28380, accept_new_port );


int issue_command(char * buffer, int retsize, char *pusrdata, unsigned short len)
{
	char * buffend = buffer;
	pusrdata[len] = 0;

	switch( pusrdata[0] )
	{
	case 'e': case 'E':
		if( retsize > len )
		{
			if( len > 1 )
			{
				write( 0, pusrdata, len );
			}
			memcpy( buffend, pusrdata, len );
			return len;
		}
		else
		{
			return -1;
		}
	case 'w': case 'W':  //This repeatedly comes from the browser.
		buffer[0] = 'w';
		buffer[1] = 'x';
		buffer[2] = '\t';
		buffer[3] = close_me?'1':'0';
		return 4;
	case 'L': case 'l': // LEDs
		switch( pusrdata[1] )
		{
		case '0': return DoLEDReturn( 0, pusrdata, buffer, retsize );
		case '1': return DoLEDReturn( 1, pusrdata, buffer, retsize );
		case '2': return DoLEDReturn( 2, pusrdata, buffer, retsize );
		case '3': return DoLEDReturn( 3, pusrdata, buffer, retsize );
		}
		break;

	case 'C': case 'c': // "Custom"
		switch(pusrdata[1])
		{
		case 'S': case 's':
		{
			switch( pusrdata[2] )
			{
				case 'S': case 's':	SaveParameters();		memcpy( buffer, pusrdata, 4 ); return 3;
				case 'R': case 'r':	RevertParameters();		memcpy( buffer, pusrdata, 4 ); return 3; //Revert from saved
				case 'P': case 'p':	RestoreParameters();	memcpy( buffer, pusrdata, 4 ); return 3;
				default: 
					return 0;
				break;
			}
			break;
		}
		case 'V': case 'v':
			{
			switch( pusrdata[2] )
			{
				case 'R': case 'r':	return DoParameterList( buffer, retsize, '\t', '\n' );
				case 'W': case 'w': return HandleSetParameter( pusrdata+3, buffer, retsize, '\t', '\n' );
				default: 
				break;
			}
			}
			break;
		}
	break;
	default:
		write( 0, pusrdata, len );
		putchar( '\n' );
		break;
	}
	return -1;
}

void Callback( void * iv, int lid )
{
	int i = *((unsigned*)&iv);
	printf( "Tray callback: %d\n",i  );

	switch( i )
	{
		case -1:
		{
			printf( "Click.\n" );
			break;
		}
		case 1: TermTray(); close_me = 1;
		case 2: printf( "Is Checked: %d\n", TrayGetChecked( lid ) ); break;
	}

	if( i >= IFACE_OFFSET && i < interfaces + IFACE_OFFSET || i == -1 )
	{
		char url[256];

		if( i == -1 )
		{
			i = IFACE_OFFSET;
		}
		else
		{
			TraySetChecked( lid, httpok );
		}
		printf( "%d, %d = %d\n", IFACE_OFFSET, i, i-IFACE_OFFSET );
		sprintf( url, "http://%s:%d", interface_addresses[i-IFACE_OFFSET], gport );
		printf( "%s\n", url );
		URLOpen( url );
	}
}


void * TrayRunner( void * v )
{
	int i = 0;

	for( i = 0; i < MAX_IFACES; i++ )
	{
		char cc[128];
		strcpy( cc, "HTTP at " );
		int r = GetMyIP( interface_addresses[i], sizeof(cc)-9, i );
		if( r ) break;
		strcpy( cc + 8, interface_addresses[i] );
		interface_elems[i] = TrayAddItem( cc, 1, Callback, (void*)((unsigned long)(IFACE_OFFSET+i)) );
	}
	interfaces = i;

	TrayAddItem( "Test", 1, Callback, (void*)2 );
	TrayAddItem( "Exit", 0, Callback, (void*)1 );
	SetTrayClickCallback( Callback, (void*)-1 );

	uint8_t colordata[128*128*4];
	int ptr = 0;
	int x, y;
	for( y = 0; y < 128; y++ )
	for( x = 0; x < 128; x++ )
	{
		int g = (x+y)*2-128;
		colordata[ptr++] = x*2;
		colordata[ptr++] = (g>0)?( (g<255)?g:255 ):0;
		colordata[ptr++] = y*2;
		colordata[ptr++] = 255;
	}
	TrayLaunch("Razorlight", colordata, 128, 128);
}


int main( int argc, char ** argv )
{
	int i;
	int rcycle = 0;

	RevertParameters(); //To load from saved settings into RAM.

	RunHTTP( gport );
	TrayInitialize( &argc, &argv );
	OGCreateThread( TrayRunner, 0 );

	double Now = OGGetAbsoluteTime();
	double Last = Now;
	double Delta = 0;

	while(!close_me)
	{
		usleep( cycle_wait_ms*1000 );
		rcycle++;

		if( rcycle > 0 && rcycle >= cycles_per_window_poll )
		{
			rcycle = 0;
			PollForWindows();
		}

		Now = OGGetAbsoluteTime();
		Delta = Now - Last;
		Last = Now;
		UpdateAlternates( Delta );
		UpdateLEDs( Delta );

		SendLEDs();

		i = (TickHTTP() == 0);
		if( i != httpok )
		{
			httpok = i;
			for( i = 0; i < interfaces; i++ )
			{
				TraySetChecked( interface_elems[i], httpok );
			}
		}
	}

	//Allow time for clients to disconnect.
	OGSleep( 2 );
	return 0;
}

