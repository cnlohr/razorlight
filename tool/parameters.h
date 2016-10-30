#ifndef _PARAMETERS_H
#define _PARAMETERS_H

//For strlen
#include <string.h>

#define MAX_PAR_STRING 256
#define NR_PARAMS 30

struct ParamSet
{
	const char * 	name;
	char 			type;
	void * 			data;
	void *			defaul;
	int (*FSFunc)( struct ParamSet * ps, const char * strin );
};
extern int nr_parameters;
extern struct ParamSet PARAMETERLIST[NR_PARAMS];

//returns length if ok. Negative if bad.
int ParameterToString( int param, char * str, int max_len );

//returns 0 if OK, nonzero otherwise.
int ParameterFromString( int param, const char * str );

int FindParameter( const char * pname );

void RegParameter( const char * pname, char type, void * data, int (*FSFunc)( struct ParamSet *, const char * ), void * defaul );

void SaveParameters();
void RevertParameters();
void RestoreParameters();

#define REG_PARAM_I( pname, defaul, vf ) \
	int pname = defaul; \
	int pname##d = defaul; \
	void __attribute__((constructor)) reg##pname() { RegParameter( #pname, 'I', (void*)&pname, vf, (void*)&(pname##d) ); }

#define REG_PARAM_F( pname, defaul, vf ) \
	float pname = defaul; \
	float pname##d = defaul; \
	void __attribute__((constructor)) reg##pname() { RegParameter( #pname, 'F', (void*)&pname, vf, (void*)&(pname##d) ); }

#define REG_PARAM_S( pname, defaul, vf ) \
	char pname[MAX_PAR_STRING]; \
	const char * pname##d = defaul; \
	void __attribute__((constructor)) reg##pname() { RegParameter( #pname, 'S', (void*)pname, vf, (void*)&(pname##d) ); if( defaul ) strcpy( pname, defaul ); else pname[0] = 0; }



int DoParameterList( char * retdata, int retsize, char delimA, char delimB );
int HandleSetParameter( char * psrdata, char * retdata, int retsize, char delimA, char delimB );


#endif

