/*
 * blur.c
 *
 * Example application and test utility for easimage.
 * Uses imgCreateGaussian(), imgSavePAM, viewDisplayImage()
 * 
 */
#include <stdio.h>
#include <assert.h>
#include "easimage.h"

int main() {
	init_easimage();

	Image *iKern = imgCreateGaussian(257, 0.);
	assert(iKern);
	imgSavePAM(iKern,"gauss.pam");
	Viewer *vKern = viewOpen(iKern->width,iKern->height, "Gaussian");
	assert(vKern);
	viewDisplayImage(vKern,iKern);
	waitTime(8000);
	viewClose(vKern);
	imgDestroy(iKern);	
	quit_easimage();
	exit(0);
}
