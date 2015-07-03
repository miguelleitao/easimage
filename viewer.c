#include <malloc.h>

#include <SDL/SDL.h>

#include "easimage.h"


Viewer * viewOpen(unsigned int width, unsigned int height, const char * title)
{	
	// set up the view
	Viewer * view = malloc(sizeof(*view));
	if (view == NULL){
		fprintf(stderr, "Could not allocate memory for view\n");
		return NULL;
	}
	
	// initialise the screen surface
	view->screen = SDL_SetVideoMode(width, height, 24, SDL_SWSURFACE);
	if(view == NULL){
		fprintf(stderr, "Failed to open screen surface\n");
		free(view);
		return NULL;
	}

	// set the window title
	SDL_WM_SetCaption(title, 0);
	
	// return the completed view object
	return view;
}


void viewClose(Viewer * view)
{
	if ( view==NULL ) return;
	// free the screen surface
	SDL_FreeSurface(view->screen);
	// free the view container
	free(view);
}


// take an image and display it on the view
void viewDisplayImage(Viewer * view, Image * img)
{
	if ( view==NULL ) return;
	if ( img->format == YUYV ) {
		fprintf(stderr,"Display of YUYV image is not implemented\n");
		return;
	}
	if ( img->format == RGB24 ) 
		fprintf(stderr,"Display of RGB image\n");
	SDL_Surface *surf;
	// Fill the SDL_Surface container
	surf = SDL_CreateRGBSurfaceFrom(
				img->data,
				img->width,
				img->height,
				24, 
				img->width * 3,
				0xff0000,
				0x00ff00,
				0x0000ff,
				0x000000
	);


	// check the surface was initialised
	if (surf == NULL) {
		perror("Display image");
		return;
	}
	// Blit the image to the window surface
	SDL_BlitSurface(surf, NULL, view->screen, NULL);
	
	// Flip the screen to display the changes
	SDL_Flip(view->screen);
	free(surf);
	
}
