#include <stdlib.h>
#include <stdio.h>
#include "easimage.h"
//include <unistd.h>
//include <sys/wait.h>
//include <termios.h>
//include <fcntl.h>

int Verbose=0;

void Help() {
    printf("\nAvailable commands:\n");
    printf("\th     This help\n");
    printf("\tq     Quit\n");
}


void Usage(char *pname) {
    printf("Usage: %s [options]\n  Options:\n", pname);
    printf("\t-v    Increase verbosity (debug mode)\n");
}


int GetParams(int argc, char **argv) {
        argc--;
	char **aval = argv + 1;
        while ( argc>0 && aval[0][0]=='-' ) {
                switch( aval[0][1] ) {
		    case 'v':
			Verbose = 1;
			break;
                    default:
                        Usage(argv[0]);
                        return -1;
            }
            argc--;
            aval++;
        }
        return argc;
}


int main(int argc, char *argv[])
{
	printf("\neasimgview %s\n\n", STR(VERSION) );

	argc = GetParams(argc,argv);

	// initialise the library
	init_easimage();

	Image *img;
	if ( argc>0 )  	img = imgFromFile(argv[1]);
	else 		img = imgFromPPM("default.ppm");

        if ( ! img ) {
	    Usage(argv[0]);
	    quit_easimage();
	    exit(1);
	}

	Viewer *view = NULL;
        // create a new viewer of the used resolution with a caption
        view = viewOpen(img->width, img->height, "Easimgview");

	int i;
//	int end = 0;

        viewDisplayImage(view, img);

	GetTime();

	//printf("Entering main loop\n");
	for ( i=0 ; i<100000 && ! easimageAppEnd ; i++ ) {
		if ( kbhit() ) {
		    // Tecla pressionada
		    int c = getchar();
			    switch (c) {
				case 'q':
				case 'Q':
				case 27:  // Esc
				    easimageAppEnd = 1;
				    break;
				case 'h':
				    Help();
				    break;
				default:
				    printf("key %d=%c\n",c,c);
			    	    break;
			    }
		}
		SDL_Event Event;
		while (viewPollEvent(&Event)) {
	            switch (Event.type) {
		        case SDL_QUIT:		// Quit program
			    easimageAppEnd = 1;
			    break;
			case SDL_KEYDOWN: 	// Quit program
			    if (Event.key.keysym.sym == SDLK_q) 
				easimageAppEnd = 1;
			    break;
	    	    }
        	}
		waitTime(100);
		    
	}
	printf("%d images processed in %.1f seconds. %.2f img/sec\n\n", 
		i, GetTime()/1000., i*1000./GetTime() );

	// now we will free the memory for the various objects
	imgDestroy(img);
	viewClose(view);

	// finally we unintialise the library
	quit_easimage();
	printf("\nExiting...\n");

	return 0;
}

