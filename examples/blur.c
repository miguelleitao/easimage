
#include <stdio.h>
#include <assert.h>
#include "easimage.h"

int main() {
	init_easimage();
	Image *iKern = imgCreateGaussian(257, 0.);
	//Image *iKern = imgNew(640,480,24);
	assert(iKern);
	printf("kernel created\n");
	imgSavePAM(iKern,"gauss.pam");
printf("viewOpen\n");	
	Viewer *vKern = viewOpen(iKern->width,iKern->height, "Gaussian");
printf("abriu viewer\n");
	assert(vKern);
	printf("Viewer created\n");
	viewDisplayImage(vKern,iKern);
printf("getting key\n");
	getchar();
printf("got key\n");
viewClose(vKern);
imgDestroy(iKern);	
	quit_easimage();
printf("quit\n");
}
