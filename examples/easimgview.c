#include <stdlib.h>
#include <stdio.h>
#include "easimage.h"
#include <unistd.h>
#include <sys/wait.h>
#include <termios.h>
#include <fcntl.h>

int Verbose=0;

Image *img;


long int GetTime()
{
   //return elapsed time in milisec;
   static time_t init_t = -1;
   struct timeval t;
   gettimeofday(&t,NULL);
   if ( init_t == -1L ) {
        init_t = t.tv_sec;
        printf("Timer reseted at %ld.%06ldsec\n",t.tv_sec,t.tv_usec);
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

void MarkImagePositionRGB(Image * img, int x, int y, 
	unsigned char r, unsigned char g, unsigned char b) {
    if ( ! img ) return;
    int len=7;
    int i;
    for( i=-len ; i<=len ; i++ ) {
        imgSetPixelRGB(img, x, y+i, r, g, b);
        imgSetPixelRGB(img, x+i, y, r, g, b);
    }
}

void MarkImagePosition(Image * img, int x, int y) {
	MarkImagePositionRGB(img,x,y,0,0,255);
}

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
	printf("\nimgview v1.2.1\n\n");

	GetParams(argc,argv);

	// initialise the library
	init_easimage();

	Viewer *view = NULL;
	// create a new viewer of the used resolution with a caption
        view = viewOpen(640, 400, "Easimgview");
	
	// open the webcam, with a capture resolution of width 640 and height 480
	Camera * cam = camOpen("/dev/video0", 640, 480, YUYV); //BGR24);
	//cam->format = RGB24;

	Image * img = imgNew(cam->width, cam->height, 24);
	img->format = RGB24;
	//Image * img = imgFromPPM("sample.ppm");

	int i;
	int end = 0;
	printf("Entering main loop\n");
	for ( i=0 ; i<100000 && !end ; i++ ) {

		if ( kbhit() ) {
		    // Tecla pressionada
		    int c = getchar();
			    switch (c) {
				case 'q':
				case 'Q':
				case 27:  // Esc
				    end = 1;
				    break;
				case 'h':
				    Help();
				    break;
				default:
				    printf("teclou %d=%c\n",c,c);
			    	    break;
			    }
		}
	        // capture an image from the webcam
	        if (camGrabImage(cam,img)) {
		    fprintf(stderr, "Image not got\n");
	            break;
	    	}
	
		//Image * img = imgFromBitmap("room4l.bmp");
	
		viewDisplayImage(view, img);
		    
	}
	printf("%d images processed in %.1f seconds. %.2f img/sec\n\n", 
		i, GetTime()/100., i*1000./GetTime() );

	// now we will free the memory for the various objects
	imgDestroy(img);
	viewClose(view);
	camClose(cam);

	// finally we unintialise the library
	quit_easimage();
	printf("\nExiting...\n");

	return 0;
}

