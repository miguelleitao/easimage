/**
 * @file 	util.c
 *
 * @author	Miguel Leitao
 *
 *
 */

#include <SDL/SDL.h>

// initialise
void init_easimage()
{
	SDL_Init(SDL_INIT_VIDEO);
}


// delay for a given number of milliseconds
void waitTime(unsigned int msec)
{
	SDL_Delay(msec);
}


// quit
void quit_easimage()
{
	SDL_Quit();
}

