#include "urlopen.h"
#ifdef WIN32
#include <windows.h>
#else
#include <stdio.h>
#include <stdlib.h>
#endif

//Expects preceeding http://
void URLOpen( const char * url )
{
#ifdef WIN32
	ShellExecute(0, "open", url, 0, 0 , SW_SHOWNORMAL );
#else
	char shelln[8192];
	sprintf( shelln, "xdg-open %s", url );
	int r = system( shelln );
#endif

}

