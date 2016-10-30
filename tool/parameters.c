#include "parameters.h"
#include "util.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct ParamSet PARAMETERLIST[NR_PARAMS];
int nr_parameters = 0;


int ParameterToString( int param, char * str, int max_len )
{
	if( param < 0 ) return -1;
	if( param >= NR_PARAMS ) return -2;
	struct ParamSet * p = &PARAMETERLIST[param];
	switch( p->type )
	{
	case 'I': return snprintf( str, max_len, "%d", *((int*)p->data) );
	case 'F': return snprintf( str, max_len, "%f", *((float*)p->data) );
	case 'S': return snprintf( str, max_len, "%s", (char*)p->data );
	default: return -3;
	}
}

int ParameterFromString( int param, const char * str )
{
	if( param < 0 ) return -1;
	if( param >= NR_PARAMS ) return -2;
	struct ParamSet * p = &PARAMETERLIST[param];

	if( p->FSFunc ) return p->FSFunc( p, str );

	switch( p->type )
	{
	case 'I': *((int*)p->data) = atoi( str ); return 0;
	case 'F': *((float*)p->data) = atof( str ); return 0;
	case 'S': strncpy( p->data, str, MAX_PAR_STRING ); return 0;
	default: return -3;
	}
}

int FindParameter( const char * pname )
{
	int i;
	for( i = 0; i < NR_PARAMS; i++ )
	{
		struct ParamSet * p = &PARAMETERLIST[i];
		if( !p->name ) continue;
		if( strcmp( p->name, pname ) == 0 ) return i;
	}
	return -1;
}

void RegParameter( const char * pname, char type, void * data, int (*FSFunc)( struct ParamSet *, const char * ), void * defaul )
{
	if( nr_parameters == NR_PARAMS ) return;
	struct ParamSet * p = &PARAMETERLIST[nr_parameters++];
	p->name = pname;
	p->type = type;
	p->data = data;
	p->defaul = defaul;
	p->FSFunc = FSFunc;
}




int DoParameterList( char * retdata, int retsize, char delimA, char delimB )
{	
	char * ree = retdata;
	int i;
	for( i = 0; i < NR_PARAMS; i++ )
	{
		char tmp[MAX_PAR_STRING];
		char tmp_buf[MAX_PAR_STRING];
		struct ParamSet * p = &PARAMETERLIST[i];
		int rs = ParameterToString( i, tmp, MAX_PAR_STRING );
		if( rs < 0 ) continue;
		rs = URIEncode( tmp_buf, sizeof( tmp_buf ), tmp, -1 );
		if( rs < 0 ) continue;
		int k = snprintf( ree, retsize - (ree-retdata), "%s%c%s%c", p->name, delimA, tmp_buf, delimB );
		if( k <= 0 ) break;
		ree += k;
	}
	*ree = 0;
	return ree - retdata;
}

int HandleSetParameter( char * psrdata, char * retdata, int retsize, char delimA, char delimB )
{
	//XXX Careful.  be sure to check for retdata==0 and be okay with that case.
	char * cnext = psrdata;
	do
	{
		char * cn = cnext;
		char * cv = NextAndZero( cn, delimA );
		if( !cv ) return 0;
		cnext = NextAndZero( cv, delimB );

		int par = FindParameter( cn );
		if( par >= 0 )
		{
			char ctbuff[MAX_PAR_STRING];
			int r = URIDecode( ctbuff, sizeof( ctbuff ), cv, -1 );
			if( r < 0 ) return 0;
			ParameterFromString( par, ctbuff );		
		} 
	}
	while( cnext );

	return 0;
}

#ifdef WIN32
#include <windows.h>
#endif

void SaveParameters()
{
	char savebuff[16384];
	int len = DoParameterList( savebuff, sizeof( savebuff ), '*', '~' );

#ifdef WIN32
	HKEY hkey;
	DWORD dwDisposition;
	if(RegCreateKeyEx(HKEY_CURRENT_USER, 
		TEXT("Software\\razorlight\\settings"), 
		0, NULL, 0, 
		KEY_WRITE, NULL, 
		&hkey, &dwDisposition) == ERROR_SUCCESS)
	{
		RegSetValueEx(hkey, TEXT("value"), 0, REG_SZ, savebuff, len); // does not create anything
		RegCloseKey(hkey);
	}
	else
	{
		printf( "Could not save settings.\n" );
	}
#else
	FILE * f = fopen( ".razorlight_settings", "w" );
	if( f ) 
	{
		fwrite( savebuff, len, 1, f );
		fclose( f );
	}
	else
	{
		printf( "Could not save.\n" );
	}
#endif
}

//Load from saved parameters.
void RevertParameters()
{
	char loadbuff[16384];
	unsigned long size;

#ifdef WIN32
    size = sizeof(loadbuff)-1;
    ULONG nError;
	DWORD dwDisposition;
	HKEY hkey;
	if(RegCreateKeyEx(HKEY_CURRENT_USER, 
		TEXT("Software\\razorlight\\settings"), 
		0, NULL, 0, 
		KEY_READ, NULL, 
		&hkey, &dwDisposition) == ERROR_SUCCESS)
	{
		nError = RegQueryValueEx(hkey, TEXT("value"), 0, NULL, loadbuff, &size);
		if (ERROR_SUCCESS == nError)
		{
		    //Ok.
		}
		else
		{
			printf( "Could not open registry file.\n" );
			return;
		}
		RegCloseKey(hkey);
	}
	else
	{
		printf( "Could not save settings.\n" );
		return;
	}


#else
	FILE * f = fopen( ".razorlight_settings", "rb" );
	if( f )
	{
		size = fread( loadbuff, 1,  sizeof( loadbuff )-1, f );
		fclose( f );
	}
	else
	{
		printf( "No file found to load.\n" );
		return;
	}
#endif

	loadbuff[size] = 0;
	HandleSetParameter( loadbuff, 0, 0, '*', '~' );
	printf( "Settings Loaded.\n" );
}

void RestoreParameters()
{
	int i;
	for( i = 0; i < NR_PARAMS; i++ )
	{
		struct ParamSet * ps = &PARAMETERLIST[i];
		int size = 0;
		char pss[32];
		char * psz = ps->defaul;
		switch( ps->type )
		{
		case 'I':
			size = 4;
			sprintf( pss, "%d", *((int*)psz) );
			psz = pss;
			break;
		case 'F':
			sprintf( pss, "%f", *((float*)psz) );
			psz = pss;
			size = 4;
			break;
		case 'S':
			size = strlen( psz );
			break;
		default:
			size = -1;
			break;
		}

		if( size < 0 ) continue;

		if( ps->FSFunc )
			ps->FSFunc( ps, psz );
		else
			memcpy( ps->data, ps->defaul, size );
	}
	printf( "Settings restored\n" );
}



