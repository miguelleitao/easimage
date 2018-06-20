/**
 * @file 	util.c
 *
 * @author	Miguel Leitao
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>

// initialise
void init_easimage()
{
    int res = SDL_Init(SDL_INIT_VIDEO);
    if ( res!=0 ) {
        fprintf(stderr, "Unable to initialize SDL:  %s\n", SDL_GetError());
        exit(1);
    }
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

