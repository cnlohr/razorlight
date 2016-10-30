#ifndef _UTIL_H
#define _UTIL_H

char    byte1hexl( unsigned char v ); //0 to 16 to char

int URIEncode( char * dest_string, int dest_len, const char * data_in, int data_in_length );
int URIDecode( char * dest_string, int dest_len, const char * data_in, int data_in_length );

char * NextAndZero( char * cur, char delim );

#endif

