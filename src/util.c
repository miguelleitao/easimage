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
    #ifdef _SDL_H
	SDL_Delay(msec);
    #else
	usleep(msec*1000);
    #endif
}


//! Quit
void quit_easimage()
{
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

