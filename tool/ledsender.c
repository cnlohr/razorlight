#include "ledsender.h"
#include "led_manage.h"
#include "parameters.h"
#include <string.h>
#include <stdio.h>
#include "util.h"


#ifdef WIN32

#include <winsock2.h>
#include <windows.h>
#define MSG_NOSIGNAL 0

#else

#include <netdb.h>
#include <sys/types.h>

#endif

#define MAX_LED_CONNECTIONS 2
#define MAX_CONNSET 5

struct LEDConnSet
{
	uint32_t * ledptr;
	int start;
	int end;
};

struct LEDConnection
{
	char host[256];
	int port;
	int offset;
	int sock;

	struct LEDConnSet CS[MAX_CONNSET];
};

struct LEDConnection CONNS[MAX_LED_CONNECTIONS];

#ifdef WIN32
int setup_wsa = 0;
#endif

//Don't worry, these get called at startup.
static int changeaddy( int which, struct ParamSet * ps, const char * strin )
{
	char datax[MAX_PAR_STRING];
	strncpy( datax, strin, MAX_PAR_STRING ); 
	char * dat = datax;

#ifdef WIN32
	if( !setup_wsa )
	{
		WSADATA wsaData;
		WSAStartup(0x202, &wsaData);
		setup_wsa = 1;
	}
#endif

	char * IP = dat;
	char * HPort = NextAndZero( IP, ':' );
	char * HOffset = NextAndZero( HPort, ':' );
	if( !HPort || !HOffset )
	{
		printf( "Invalid change addy.\n" );
		return -1;
	}

	struct LEDConnection * l = &CONNS[which];

	strncpy( l->host, IP, sizeof( l->host ) );
	l->port = atoi( HPort );
	l->offset = atoi( HOffset );
	l->sock = socket (AF_INET, SOCK_DGRAM, 0);
	if( l->sock <= 0 )
	{
		printf( "Can't create socket.\n" );
		l->sock = 0;
	}

	struct hostent * gh = gethostbyname( l->host );
	struct sockaddr_in servAddr;

	if( !gh )
	{
		fprintf( stderr, "Error: Could not resolve host name: \"%s\"\n", l->host );
		l->sock = 0;
	}

	servAddr.sin_family = AF_INET;
	memcpy( &servAddr.sin_addr, gh->h_addr_list[0], gh->h_length );
	servAddr.sin_port = htons( l->port );
	int rc = connect( l->sock, (struct sockaddr *)&servAddr, sizeof(servAddr) );

	if( rc < 0 )
	{
		printf( "Could not connect.\n" );
		l->sock = 0;
	}

	printf( "Connected to: %s:%d on socket %d\n", l->host, l->port, l->sock );
	strncpy( ps->data, strin, MAX_PAR_STRING ); 
	return 0;
}

static int changestat( int which, struct ParamSet * ps, const char * strin )
{
	char datax[MAX_PAR_STRING];
	strncpy( datax, strin, MAX_PAR_STRING ); 
	char * dat = datax;
	char * next;
	int set = 0;
	struct LEDConnSet * CS;
	do
	{
		next = NextAndZero( dat, ':' );
		CS = &CONNS[which].CS[set];

		char * num = dat+1;
		char * endnum = NextAndZero( num, '-' );
		if( !num || !endnum ) break;
		CS->start = atoi( num );
		CS->end = atoi( endnum );
		CS->ledptr = GetLEDsByIndex( dat[0] );
		dat = next;
		set++;
	} while( next && set < MAX_CONNSET);
	CONNS[which].CS[set].ledptr = 0;
	
	strncpy( ps->data, strin, MAX_PAR_STRING ); 
}



static int update_1_add( struct ParamSet * ps, const char * strin ) { return changeaddy( 1, ps, strin ); }
REG_PARAM_S( remote_1_address, "192.168.11.5:7777:0", update_1_add ); 
static int update_0_add( struct ParamSet * ps, const char * strin ) { return changeaddy( 0, ps, strin ); }
REG_PARAM_S( remote_0_address, "192.168.11.4:7777:0", update_0_add );

static int update_0_stat( struct ParamSet * ps, const char * strin ) { return changestat( 0, ps, strin ); }
REG_PARAM_S( remote_0_config,  "B80-0:R80-0", update_0_stat ); 
static int update_1_stat( struct ParamSet * ps, const char * strin ) { return changestat( 1, ps, strin ); }
REG_PARAM_S( remote_1_config,  "B80-0:R80-0", update_1_stat );

void SendLEDs()
{
	#define MAX_LEDS 492

	uint8_t LEDs[MAX_LEDS*3];
	int i, j, k;
	int ls = 0; //LED Stack Position


	for( i = 0; i < MAX_LED_CONNECTIONS; i++ )
	{
		struct LEDConnection *c = &CONNS[i];
		if( !c->sock ) continue;
		for( ls = 0; ls < c->offset; ls++ )
		{
			LEDs[ls] = 0;
		}
		for( j = 0; j < MAX_CONNSET; j++ )
		{
			struct LEDConnSet * s = &c->CS[j];
			if( !s->ledptr ) continue;

			if( s->end < s->start )
			{
				//Go backwards
				for( k = s->start; k >= s->end; k-- )
				{
					if( k >= MAX_LED_PER || k < 0 )
					{
						break;
					}
					uint32_t col = s->ledptr[k];
					LEDs[ls+0] = (col>>8)&0xff;
					LEDs[ls+1] = col&0xff;
					LEDs[ls+2] = (col>>16);
					ls+=3;
					if( ls >= MAX_LEDS*3 ) break;
				}
			}
			else
			{
				//Go forwards
				for( k = s->start; k < s->end; k++ )
				{
					if( k >= MAX_LED_PER || k < 0 )
					{
						break;
					}
					uint32_t col = s->ledptr[k];
					LEDs[ls+0] = (col>>8)&0xff;
					LEDs[ls+1] = col&0xff;
					LEDs[ls+2] = (col>>16);
					ls+=3;
					if( ls >= MAX_LEDS*3 ) break;
				}
			}
		}


		send( c->sock, LEDs, ls, MSG_NOSIGNAL );
	}
}

