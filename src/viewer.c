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
	Uint32 r_mask = 0x000000ff;
	Uint32 g_mask = 0x0000ff00;
	Uint32 b_mask = 0x00ff0000;
	Uint32 a_mask = 0x00000000;

	if ( img==NULL ) {
		fprintf(stderr,"No image to display\n");
		return;
	}
	if ( img->format == YUYV ) {
		fprintf(stderr,"Display of YUYV image is not implemented\n");
		return;
	}

	if ( img->format == BGR24 )  {
		r_mask = 0xff0000;
		b_mask = 0x0000ff;
	}
	if ( img->format == RGBA32 ) 
		a_mask = 0xff000000;
        if ( view==NULL ) {
                view = viewOpen(img->width,img->height,img->name);
        }

	SDL_Surface *surf;
	// Fill the SDL_Surface container
	surf = SDL_CreateRGBSurfaceFrom(
				img->data,
				img->width,
				img->height,
				img->depth, 
				img->width * img->depth/8,
				r_mask,
				g_mask,
				b_mask,
				a_mask
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