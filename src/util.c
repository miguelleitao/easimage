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
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#include "easimage.h"

int easimageAppEnd = 0;

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


//! Delay for a given number of milliseconds
/*!
 *  @msec is the number of milisecond to wait.
 */
void waitTime(unsigned int msec)
{
    while( msec>100 ) {
	#ifdef _SDL_H
            SDL_Delay(100);
        #else
            usleep(100*1000);
	#endif
	msec -= 100;
	if ( easimageAppEnd ) return;
    }
    #ifdef _SDL_H
	SDL_Delay(msec);
    #else
	usleep(msec*1000);
    #endif
}


//! Quit
void quit_easimage()
{
    easimageAppEnd = 1;
    #ifdef _SDH_H
	SDL_Quit();
    #endif
}

//! GetTime
/*!
 *  Returns the current time in miliseconds.
 *  Current time is mesure as the elapsed time from the first call to GetTime().
 *  First call to GetTime() returns 0L
 */
long int GetTime()
{
   //return elapsed time in milisec;
   static time_t init_t = -1L;
   struct timeval t;
   gettimeofday(&t,NULL);
   if ( init_t == -1L ) {
        init_t = t.tv_sec;
//        printf("Timer reseted at %ld.%06ldsec\n",t.tv_sec,t.tv_usec);
   }
   long int res = 1000L*(t.tv_sec-init_t)+t.tv_usec/1000L;
   //printf("GetTime res=%ld\n",res);
   return res;
}

int kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

  ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);

  if(ch != EOF) {
    ungetc(ch, stdin);
    return 1;
  }
  return 0;
}

int appProcEvents() {
               if ( kbhit() ) {
                    // Tecla pressionada
                    int c = getchar();
                    switch (c) {
                       case 'q':
                       case 'Q':
                       case 27:  // Esc
                           easimageAppEnd = 1;
                           break;
                       default:
                           printf("key %d=%c\n",c,c);
                           break;
                    }
                }
                SDL_Event Event;
                while (viewPollEvent(&Event)) {
                    switch (Event.type) {
                        case SDL_QUIT:          // Quit program
                            easimageAppEnd = 1;
                            break;
                        case SDL_KEYDOWN:       // Quit program
                            if (Event.key.keysym.sym == SDLK_q)
                                easimageAppEnd = 1;
                            if (Event.key.keysym.sym == SDLK_ESCAPE)
                                easimageAppEnd = 1;
                            break;
                    }
                }
                waitTime(100);
		return easimageAppEnd;
}

