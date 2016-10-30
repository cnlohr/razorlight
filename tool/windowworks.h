#ifndef _WINDOWPOLLING_H
#define _WINDOWPOLLING_H


#include <stdint.h>

#define MAX_PIXEL_BUFFER 8192

extern int top_leds;
extern int left_leds;
extern float fade_ratio; //percentage of current "Colors".  0.0 = full "Alternative"

//1 if fading in or full window.  0 if fading out or 100% faded.
extern int valid_window;

//Whether or not a window is available at all, regardless of if it's big enough or anything.
extern int focusWindowAvailable;
extern int focusWindowWidth;
extern int focusWindowHeight;
extern char focusName[256];

void InitWindow();
void StartPolling();
int GetPixelRange( int x, int y, int w, int h );


#endif

