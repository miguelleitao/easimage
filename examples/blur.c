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

	Image *iKern = imgCreateGaussian(7, 0.);
	assert(iKern);
	imgSavePAM(iKern, "gauss.pam");
	Viewer *vKern = viewOpen(iKern->width,iKern->height, "Gaussian");
	assert(vKern);
	viewDisplayImage(vKern, iKern);
	float mean = imgGetMean(iKern);
	printf("Kernel mean: %f\n", mean);
	waitTime(2000);

	Image *iImag = imgFromBitmap("default.bmp");
	assert(iImag);
	Viewer *vImag = viewOpen(iImag->width, iImag->height, "Input image");
	assert(vImag);
	viewDisplayImage(vImag, iImag);
	waitTime(2000);

	Image* iOut = imgConvolution(iImag, iKern, NULL);
	assert(iOut);
	Viewer *vOut = viewOpen(iOut->width, iOut->height, "Output image");
	assert(vOut);
	viewDisplayImage(vOut, iOut);

	
	waitTime(40000);

	viewClose(vOut);
	imgDestroy(iOut);

	viewClose(vImag);
	imgDestroy(iImag);

	viewClose(vKern);
	imgDestroy(iKern);

	quit_easimage();
	exit(0);
}
