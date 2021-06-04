#include <stdlib.h>
#include <stdio.h>
#include "easimage.h"
#include <unistd.h>
#include <sys/wait.h>
#include <termios.h>
#include <fcntl.h>

int Verbose=0;

Image *img;


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
        while ( argc>0 ) {
            if ( aval[0][0]=='-' ) {
                switch( aval[0][1] ) {
		    case 'v':
			Verbose = 1;
			break;
                    default:
                        Usage(argv[0]);
                        return -1;
                }
            }
            else {
                Usage("./harpa");
                return -1;
            }
            argc--;
            aval++;
        }
        return argc;
}


int main(int argc, char * argv[])
{
	printf("\ncamview %s\n\n", STR(VERSION) );

	GetParams(argc,argv);

	// initialise the library
	init_easimage();

	// open the webcam, with a capture resolution of width 640 and height 480
	Camera * cam = camOpen("/dev/video0", 640, 480, YUYV); //BGR24);
	//cam->format = RGB24;
	if ( ! cam ) {
	        fprintf(stderr, "camOpen failed.\n");
		exit(1);
	}

	Viewer *view = NULL;
        // create a new viewer of the used resolution with a caption
        view = viewOpen(cam->width, cam->height, "camview");


	Image * img = imgNew(cam->width, cam->height, 24);
	img->format = RGB24;

	int i;
	GetTime();
	for ( i=0 ; i<100000 && ! easimageAppEnd ; i++ ) {

                appProcEvents();

	        // capture an image from the webcam
	        if (camGrabImage(cam,img)) {
		    fprintf(stderr, "GrabImage: Image not got\n");
	            break;
	    	}
	
		viewDisplayImage(view, img);
		    
	}
	printf("%d images processed in %.1f seconds. %.2f img/sec\n\n", 
		i, GetTime()/1000., i*1000./GetTime() );

	// now we will free the memory for the various objects
	imgDestroy(img);
	viewClose(view);
	camClose(cam);

	// finally we unintialise the library
	quit_easimage();
	printf("\nExiting...\n");

	return 0;
}

