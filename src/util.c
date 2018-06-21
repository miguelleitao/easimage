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
    #ifdef _SDL_H
    int res = SDL_Init(SDL_INIT_VIDEO);
    if ( res!=0 ) {
        fprintf(stderr, "Unable to initialize SDL:  %s\n", SDL_GetError());
        exit(1);
    }
    #endif
}


// delay for a given number of milliseconds
void waitTime(unsigned int msec)
{
    #ifdef _SDL_H
	SDL_Delay(msec);
    #else
	usleep(msec*1000);
    #endif
}


// quit
void quit_easimage()
{
    #ifdef _SDH_H
	SDL_Quit();
    #endif
}

