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

        viewDisplayImage(view, img);

	GetTime();

	//printf("Entering main loop\n");
	while( ! easimageAppEnd )
		appProcEvents();

	printf("Image presented for %.1f seconds.\n\n", 
		GetTime()/1000. );

	// now we will free the memory for the various objects
	imgDestroy(img);
	viewClose(view);

	// finally we unintialise the library
	quit_easimage();
	printf("\nExiting...\n");

	return 0;
}

