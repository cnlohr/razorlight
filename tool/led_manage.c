#include "led_manage.h"
#include "windowworks.h"
#include "parameters.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "util.h"

#ifdef WIN32
#include <windows.h>
#endif

uint32_t   LeftColors[MAX_LED_PER];
uint32_t  RightColors[MAX_LED_PER];
uint32_t    TopColors[MAX_LED_PER];
uint32_t BottomColors[MAX_LED_PER];

uint32_t   LeftAlternate[MAX_LED_PER];
uint32_t  RightAlternate[MAX_LED_PER];
uint32_t    TopAlternate[MAX_LED_PER];
uint32_t BottomAlternate[MAX_LED_PER];

uint32_t   LeftLEDs[MAX_LED_PER];
uint32_t  RightLEDs[MAX_LED_PER];
uint32_t    TopLEDs[MAX_LED_PER];
uint32_t BottomLEDs[MAX_LED_PER];

uint32_t * GetLEDsByIndex( int idx )
{
	switch( idx )
	{
	case 'T': case 't': case 0: return TopLEDs;
	case 'R': case 'r': case 1: return RightLEDs;
	case 'B': case 'b': case 2: return BottomLEDs;
	case 'L': case 'l': case 3: return LeftLEDs;
	default: return 0;
	}
}

int set_led_left( struct ParamSet * p, const char * st )
{
	int pst = atoi( st );
	if( pst >= MAX_LED_PER ) pst = MAX_LED_PER-1;
	if( pst < 0 ) pst = 0;
	*((int*)p->data) = pst;
}

int set_led_top( struct ParamSet * p, const char * st )
{
	int pst = atoi( st );
	if( pst >= MAX_LED_PER ) pst = MAX_LED_PER-1;
	if( pst < 0 ) pst = 0;
	*((int*)p->data) = pst;
}

REG_PARAM_I( min_window_width, 64, 0 );
REG_PARAM_I( min_window_height, 48, 0 );
REG_PARAM_F( fade_no_window_timeout, 1.0, 0 );
REG_PARAM_F( fade_no_window_fade, 1.0, 0 );
REG_PARAM_F( fade_window_fade, 0.2, 0 );
REG_PARAM_I( top_leds, 192, set_led_top );
REG_PARAM_I( left_leds, 108, set_led_left );
REG_PARAM_I( bottom_leds, 192, set_led_top );
REG_PARAM_I( right_leds, 108, set_led_left );
REG_PARAM_I( start_edge, 5, 0 );
REG_PARAM_I( start_left, 5, 0 );
REG_PARAM_I( start_right, 5, 0 );
REG_PARAM_I( start_top, 5, 0 );
REG_PARAM_I( start_bottom, 5, 0 );
REG_PARAM_I( idle_mode, 1, 0 );
REG_PARAM_F( gamma_l, 2.2, 0 );
REG_PARAM_F( bright_l, 0.15, 0 );
REG_PARAM_F( adj_r, 1.0, 0 );
REG_PARAM_F( adj_g, 1.0, 0 );
REG_PARAM_F( adj_b, 1.0, 0 );

float time_since_fade = 0.0;
int valid_window = 0;
float fade_ratio = 0.0;


uint32_t PixelBuffer[MAX_PIXEL_BUFFER];


void PollForWindows()
{
	StartPolling();
}

int DoLEDReturn( int which, char * psrdata, char * retdata, int retsize )
{
	uint32_t * buff;
	int led_ct;
	int i;
	char * offset = NextAndZero( psrdata, '\t' );
	if( !offset ) return 0;
	char * nrl = NextAndZero( offset, '\t' );
	if( !nrl ) return 0;

	int iOff = atoi( offset );
	int iNrl = atoi( nrl );

	switch( which )
	{
		case 0: buff = TopLEDs; led_ct = top_leds; break;
		case 1: buff = RightLEDs; led_ct = right_leds; break;
		case 2: buff = BottomLEDs; led_ct = bottom_leds; break;
		case 3: buff = LeftLEDs; led_ct = left_leds; break;
		default: return 0;
	}

	if( iOff >= led_ct ) iOff = led_ct-1;
	if( iNrl * 6 + 21 > retsize ) iNrl = (retsize-21)/6;
	if( iNrl + iOff > led_ct ) iNrl = led_ct - iOff;

	int retpt = 0;

	char * rr = retdata;
	rr += sprintf( rr, "L%d\t%d\t%d\t", which, iOff, iNrl );
	for( i = 0; i < iNrl; i++ )
	{
		int r = buff[i] & 0xff;
		int g = (buff[i]>>8) & 0xff;
		int b = (buff[i]>>16) & 0xff;

		(*(rr++)) = byte1hexl( r >> 4 );
		(*(rr++)) = byte1hexl( r & 0xf );
		(*(rr++)) = byte1hexl( g >> 4 );
		(*(rr++)) = byte1hexl( g & 0xf );
		(*(rr++)) = byte1hexl( b >> 4 );
		(*(rr++)) = byte1hexl( b & 0xf );
	}
	*rr = 0;
	return rr - retdata;
}

static int CapToLEDs( uint32_t * colors, int nrled, int sx, int sy, int w, int h, int nrr )
{
	int i;
	float pitch = (float)nrr / (nrled+1);
	float pl = pitch / 0.5;
//printf( "%d %d %d %d\n", sx, sy, w, h );
	int r = GetPixelRange( sx, sy, w, h );

	if( r ) return r;

#if QUICKBLIT
	//We don't do this. It does not average the leds.
	for( i = 0; i < nrled; i++ )
	{
#ifdef WIN32
		uint8_t r = PixelBuffer[(int)pl] >> 16;
		uint8_t g = PixelBuffer[(int)pl] >> 8;
		uint8_t b = PixelBuffer[(int)pl];
		colors[i] = (r) | (g<<8) | (b<<16);
#else
		colors[i] = PixelBuffer[(int)pl];
#endif
		pl += pitch;
	}
#else
	//This is what we use.
	int ar = 0, ag = 0, ab = 0;
	int ac = 0;
	int led = 0;

	for( i = 0; i < nrr; i++ )
	{
#ifdef WIN32
		uint8_t r = PixelBuffer[i] >> 16;
		uint8_t g = PixelBuffer[i] >> 8;
		uint8_t b = PixelBuffer[i];
#else
		uint8_t b = PixelBuffer[i] >> 16;
		uint8_t g = PixelBuffer[i] >> 8;
		uint8_t r = PixelBuffer[i];
#endif
		ar += r;
		ag += g;
		ab += b;
		ac++;


		int il = ((nrled+1)*i) / nrr;
	//	/printf( "%d/%d/%d/%d\n", nrled,i,nrr,led );
		if( il != led )
		{
			r = ar/ac;
			g = ag/ac;
			b = ab/ac;
			colors[led] = (r) | (g<<8) | (b<<16);
			ar = 0;
			ag = 0;
			ab = 0;
			ac = 0;
			led = il;
		}
	}


#endif

	return 0;
}

void MixLEDs( uint32_t * out, uint32_t * b, uint32_t * a, float ratio, int count )
{
	int i;
	for( i = 0; i < count; i++ )
	{
		int ra = a[i]&0xff;
		int ga = (a[i]>>8)&0xff;
		int ba = (a[i]>>16)&0xff;

		int rb = b[i]&0xff;
		int gb = (b[i]>>8)&0xff;
		int bb = (b[i]>>16)&0xff;

		float or = ((ra * ratio + rb * (1.-ratio)) / 255.0);
		float og = ((ga * ratio + gb * (1.-ratio)) / 255.0);
		float ob = ((ba * ratio + bb * (1.-ratio)) / 255.0);

		or = powf( or, gamma_l ) * bright_l * adj_r;
		og = powf( og, gamma_l ) * bright_l * adj_g;
		ob = powf( ob, gamma_l ) * bright_l * adj_b;
		
		int kr = or * 255.8 + 0.1;
		int kg = og * 255.8 + 0.1;
		int kb = ob * 255.8 + 0.1;

		out[i] = (kr<<0) | (kg<<8) | (kb<<16);
	}
}

double MotionTime;

unsigned long HSVtoHEX( float hue, float sat, float value )
{
	float pr = 0;
	float pg = 0;
	float pb = 0;

	short ora = 0;
	short og = 0;
	short ob = 0;

	float ro = fmod( hue * 6, 6. );

	float avg = 0;

	ro = fmod( ro + 6 + 1, 6 ); //Hue was 60* off...

	if( ro < 1 ) //yellow->red
	{
		pr = 1;
		pg = 1. - ro;
	} else if( ro < 2 )
	{
		pr = 1;
		pb = ro - 1.;
	} else if( ro < 3 )
	{
		pr = 3. - ro;
		pb = 1;
	} else if( ro < 4 )
	{
		pb = 1;
		pg = ro - 3;
	} else if( ro < 5 )
	{
		pb = 5 - ro;
		pg = 1;
	} else
	{
		pg = 1;
		pr = ro - 5;
	}

	//Actually, above math is backwards, oops!
	pr *= value;
	pg *= value;
	pb *= value;

	avg += pr;
	avg += pg;
	avg += pb;

	pr = pr * sat + avg * (1.-sat);
	pg = pg * sat + avg * (1.-sat);
	pb = pb * sat + avg * (1.-sat);

	ora = pr * 255;
	og = pb * 255;
	ob = pg * 255;

	if( ora < 0 ) ora = 0;
	if( ora > 255 ) ora = 255;
	if( og < 0 ) og = 0;
	if( og > 255 ) og = 255;
	if( ob < 0 ) ob = 0;
	if( ob > 255 ) ob = 255;

	return (ob<<16) | (og<<8) | ora;
}

uint32_t syscolor;

void LDOSetup()
{
#ifdef WIN32
	syscolor = GetSysColor( 1 );
#else
	syscolor = 0;
#endif

}

uint32_t LDO( int ledno, int edg )
{
	switch( idle_mode )
	{
	case 1:
		return HSVtoHEX( ledno*0.1 + MotionTime*.1, 1.0, 1.0 );
	case 2:
		return 0x0f0f0f;
	case 3:
		return 0xffffff;
	case 4:
		return 0x000000;
	case 5:
		return 0xff0000;
	case 6:
		return 0x00ff00;
	case 7:
		return 0x000ff;
	case 8:
		return (edg>2)?0xff:(edg>1)?0xff00:(edg>0)?0xff0000:0xff00ff;
	default:
		return syscolor;
	}
}

void UpdateAlternates( double dTime )
{
	int i;
	int ledno = 0;
	MotionTime += dTime;
	LDOSetup();
	for( i = 0; i < top_leds; i++, ledno++ )
	{
		TopAlternate[i] = LDO( ledno, 0 );
	}
	for( i = 0; i < right_leds; i++, ledno++ )
	{
		RightAlternate[i] = LDO( ledno, 1 );
	}
	for( i = bottom_leds-1; i >= 0; i--, ledno++ )
	{
		BottomAlternate[i] = LDO( ledno, 2 );
	}
	for( i = left_leds-1; i >= 0; i--, ledno++ )
	{
		LeftAlternate[i] = LDO( ledno, 3 );
	}
}

void UpdateLEDs( double dTime )
{
	int i;
	float fade_coefficient;
	valid_window  = focusWindowWidth > min_window_width && focusWindowHeight > min_window_height;

retry:
	if( valid_window )
	{
		fade_coefficient = fade_window_fade;
		if( time_since_fade > fade_window_fade ) time_since_fade = fade_window_fade;
		time_since_fade -= dTime;
		if( time_since_fade < 0 ) time_since_fade = 0;
		fade_ratio = time_since_fade / fade_window_fade;

		int capwid = focusWindowWidth - start_left - start_right;
		int caphei = focusWindowHeight - start_top - start_bottom;
		int r;

		r =  CapToLEDs( TopColors,    top_leds,    start_left, start_edge,                           capwid, 1, capwid );
		r |= CapToLEDs( RightColors,  right_leds,  focusWindowWidth - start_edge - 1, start_top,  1, caphei, caphei );
		r |= CapToLEDs( BottomColors, bottom_leds, start_left, focusWindowHeight - start_edge - 1,   capwid, 1, capwid );
		r |= CapToLEDs( LeftColors,   left_leds,   start_edge, start_top,                         1, caphei, caphei );
		if( r ) { fprintf( stderr, "Error: R: %d\n", r ); valid_window = 0; goto retry; }
	}
	else
	{
		time_since_fade += dTime;
		fade_coefficient = fade_no_window_timeout + fade_no_window_fade;
		if( time_since_fade > fade_coefficient )
		{
			time_since_fade = fade_coefficient;
		}
		fade_ratio = time_since_fade / fade_no_window_fade;
		if( fade_ratio > 1.0 ) fade_ratio = 1.0;
	}

	MixLEDs( TopLEDs, TopColors, TopAlternate, fade_ratio, top_leds );
	MixLEDs( RightLEDs, RightColors, RightAlternate, fade_ratio, right_leds );
	MixLEDs( BottomLEDs, BottomColors, BottomAlternate, fade_ratio, bottom_leds );
	MixLEDs( LeftLEDs, LeftColors, LeftAlternate, fade_ratio, left_leds );
}


