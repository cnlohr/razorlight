#include <string.h>

static unsigned char hex1bytel( char c ) //From hex to byte
{
	if( c >= '0' && c <= '9' ) return c - '0';
	if( c >= 'a' && c <= 'f' ) return c - 'a' + 10;
	if( c >= 'A' && c <= 'F' ) return c - 'A' + 10;
	return 0;
}
char    byte1hexl( unsigned char v ) //0 to 16 to char
{
	if( v < 10 ) return '0' + v;
	if( v < 16 ) return 'a' - 10 + v;
	return '0'; 
}

int URIEncode( char * dest_string, int dest_len, const char * data_in, int data_in_length )
{
	int i;
	int o = 0;
	if( data_in_length == -1 ) data_in_length = strlen( data_in );
	for( i = 0; i < data_in_length; i++ )
	{
		if( o >= dest_len-2 )
			return -o;

		char c = data_in[i];
		int force = 0;
		switch( c )
		{
		case '%': case '*': case '-': case '.': case '+': case '~': force = 1;
		default:
			if( c <= 32 || c > 126 || force ) 
			{
				force = 0;
				if( o + 4 >= dest_len ) { return -o; }
				dest_string[o++] = '%';
				dest_string[o++] = byte1hexl( c >> 4 );
				dest_string[o++] = byte1hexl( c & 0x0f );
			}
			else
			{
				dest_string[o++] = c;
			}
		}			
	}
done:
	dest_string[o] = 0;
	return o;
}

int URIDecode( char * dest_string, int dest_len, const char * data_in, int data_in_length )
{
	int t;
	int i;
	int o = 0;
	if( data_in_length == -1 ) data_in_length = strlen( data_in );
	for( i = 0; i < data_in_length; i++ )
	{
		if( o >= dest_len-2)
			return -o;

		char c = data_in[i];
		switch( c )
		{
		case '+': dest_string[o++] = ' '; break;
		case '%':
			if( i + 2 >= data_in_length ) return o;
			dest_string[o++] = ( hex1bytel( data_in[i+1] ) << 4 ) | ( hex1bytel( data_in[i+2] ) );
			i+=2;
			break; 
		default: dest_string[o++] = c; break;
		}
	}
	dest_string[o] = 0;
	return o;
}


char * NextAndZero( char * cur, char delim )
{
	if( !cur ) return 0;
	char * c = strchr( cur, delim );
	if( c ) { *c = 0; c++; }
	return c;
}

