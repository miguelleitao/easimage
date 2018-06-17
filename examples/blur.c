
#include <stdio.h>
#include <assert.h>
#include "easimage.h"

int main() {
	init_easimage();
	Image *iKern = imgCreateKernelGaussianBlur(65, 0.);
	//Image *iKern = imgNew(640,480,24);
	assert(iKern);
	printf("kernel created\n");
	imgSavePAM(iKern,"gauss.pam");
	/*
	Viewer *vKern = viewOpen(iKern->width,iKern->height, "Gaussian");
	assert(vKern);
	printf("Viewer created\n");
	viewDisplayImage(vKern,iKern);
	*/
	quit_easimage();
}
