
#include "SDL.h"

int main() {
    //The images
    SDL_Surface* hello = NULL;
    SDL_Surface* screen = NULL;

    //Start SDL
    SDL_Init( SDL_INIT_EVERYTHING );

    //Set up screen
    screen = SDL_SetVideoMode( 257, 257, 24, SDL_SWSURFACE );

    //Load image
    hello = SDL_LoadBMP( "hello.bmp" );


    //Apply image to screen
    SDL_BlitSurface( hello, NULL, screen, NULL );

    //Update Screen
    SDL_Flip( screen );

    //Pause
    SDL_Delay( 8000 );

    //Free the loaded image
    SDL_FreeSurface( hello );

    //Quit SDL
    SDL_Quit();

    return 0;
}
