/*
 * sdlimgview.c
 *
 * Minimal SDL image viewer.
 * Provided as an example in the easimage package:
 * 	http://miguelleitao.github.io/easimage/
 * 
 */

#include <SDL.h>

int main(int argc, char **argv) {
    //The images
    SDL_Surface* image = NULL;
    SDL_Surface* screen = NULL;

    //Start SDL
    SDL_Init( SDL_INIT_VIDEO );

    //Load image
    if ( argc>1 && argv[1][0] )
	image = SDL_LoadBMP( argv[1] );
    else
        image = SDL_LoadBMP( "default.bmp" );

    if ( ! image ) {
	fprintf(stderr, "Image not loaded.\n");
	SDL_Quit();
	exit(1);
    }

    //Set up screen
    screen = SDL_SetVideoMode( image->w, image->h, 24, SDL_SWSURFACE );

    //Apply image to screen
    SDL_BlitSurface( image, NULL, screen, NULL );

    //Update Screen
    SDL_Flip( screen );

    // Wait for event
    SDL_Event Event;
    int Done = 0;
    while ( ! Done ) {
	while (SDL_PollEvent(&Event)) {
            switch (Event.type) {
	        case SDL_QUIT:		// Quit program
		    Done = 1;
		    break;
		case SDL_KEYDOWN: 	// Quit program
		    if (Event.key.keysym.sym == SDLK_q) 
			Done = 1;
		    break;
	    }
        }
	SDL_Delay(20);
    }

    //Free the loaded image
    SDL_FreeSurface( image );
    SDL_FreeSurface( screen );

    //Quit SDL
    SDL_Quit();

    return 0;
}
