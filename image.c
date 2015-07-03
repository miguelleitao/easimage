#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include <SDL/SDL.h>
#include <fcntl.h>
#include <unistd.h>
#include "easimage.h"


Image * imgNew(unsigned int width, unsigned int height, unsigned short depth)
{
	// Allocate for the image container
	Image * img = malloc(sizeof(*img));
	if(img == NULL){
		fprintf(stderr, "Failed to allocate memory for image container\n");
		return NULL;
	}

	// Set the width and height
	img->width = width;
	img->height = height;

	img->depth = depth;
	img->format = 0;

	// allocate for image data, depth/8 byte per pixel,
	// aligned to an 8 byte boundary
	img->mem_ptr = malloc(img->width * img->height * depth/8 + 8);
	if(img->mem_ptr == NULL){
		fprintf(stderr, "Memory allocation of image data failed\n");
		free(img);
		return NULL;
	}

	// make certain it is aligned to 8 bytes
	unsigned int remainder = ((size_t)img->mem_ptr) % 8;
	if(remainder == 0){
		img->data = img->mem_ptr;
	} else {
		img->data = img->mem_ptr + (8 - remainder);
	}

	/*
	// Fill the SDL_Surface container
	img->sdl_surface = SDL_CreateRGBSurfaceFrom(
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
	if(img->sdl_surface == NULL){
		fprintf(stderr, "Failed to initialise RGB surface from pixel data\n");
		SDL_FreeSurface(img->sdl_surface);
		free(img->mem_ptr);
		free(img);
		return NULL;
	}
*/
	// return the image
	return img;
}


Image *imgFromBitmap(const char * filename)
{
	// Load the Bitmap
	SDL_Surface * bitmap = SDL_LoadBMP(filename);

	// Allocate for the image container
	Image * img = malloc(sizeof(*img));
	if(img == NULL){
		fprintf(stderr, "Failed to allocate memory for image container\n");
		return NULL;
	}

	// Set the width and height
	img->width = bitmap->w;
	img->height = bitmap->h;
	img->format = RGB24;
	img->depth = 24;

	// set the data pointer
	img->data = bitmap->pixels;	

	// set the memory pointer to NULL, so we don't cause mayhem trying to free it
	img->mem_ptr = NULL;

	// Free SDL_Surface Container
	free(bitmap);

	// return the new image
	return img;
}

Image *imgFromPPM(const char * filename)
{
	FILE *fimg = fopen(filename, "r");
	if ( fimg==NULL ) {
		fprintf(stderr, "Failed to open image file '%s'\n", filename);
                return NULL;
        }
	char tag1 = fgetc(fimg);
	char tag2 = fgetc(fimg);
	if ( tag1!='P' || tag2!='6' ) {
                fprintf(stderr, "Invalid image format '%s'\n", filename);
		fclose(fimg);
                return NULL;
        }
	int w, h, s;
	int nr = fscanf(fimg," %d %d %d", &w, &h, &s);
	if ( nr != 3 ) {
                fprintf(stderr, "Invalid image format '%s'\n", filename);
                fclose(fimg);
                return NULL;
        }
	if ( w<=0 || w>20000 || h<=0 || h>50000 ) {
                fprintf(stderr, "Invalid image geometry '%s', (%dx%d) \n", filename, w, h);
                fclose(fimg);
                return NULL;
        }
	int pixel_size = ( s>255 ? 6 : 3 );
	Image *img = imgNew(w, h, 24);
	if (img == NULL) {
                return NULL;
        }
	img->format = BGR24;
	fgetc(fimg);	
	int i;
	unsigned char *img_ptr = img->data;
	for( i=0 ; i<w*h ; i++ ) {
		img_ptr[2] = fgetc(fimg);
		//img_ptr[2] =255;
                img_ptr[1] = fgetc(fimg);
                img_ptr[0] = fgetc(fimg);
		img_ptr += pixel_size;
	}
	fclose(fimg);
	return img;
}

void imgScale(Image *img, unsigned int sfactor) 
{
	int p;
	unsigned char *c = img->data;
	for( p=0 ; p<img->width*img->height*img->depth/8 ; p++ ) {
		int nc = 128+(*c-128)*sfactor;
		*c = min(nc,255);
		c++;
	}
}

Image *imgCopy(Image * img)
{
	// Create a new empty image
	Image * copy = imgNew(img->width, img->height, img->depth);

	copy->format = img->format;
	// Copy the data between the images
	copy = memcpy(copy->data, img->data, img->width * img->height * img->depth/8 );

	// return the copy
	return copy;	
}

Image *imgCrop(Image *img, int x1, int y1, int x2, int y2)
{
	unsigned width = x2-x1+1;
	unsigned height = y2-y1+1;

	if ( img==NULL ) {
		fprintf(stderr,"Bad image\n");
		return NULL;
	}
	Image *new_img = imgNew(width, height, img->depth);
	if ( new_img==NULL ) return NULL;
	int y;
	for( y=y1 ; y<=y2 ; y++ )
		memcpy(	new_img->data+(y-y1)*width*img->depth/8, 
			img->data+(y*img->width+x1)*img->depth/8,
			width*img->depth/8 );
	return new_img;
}


int imgGetSymmetryError( Image *img, 
			int x, int y, int radius ) // Square area of img to use 
{
	int error = 0;
	int xi, yi;
	if ( radius>x || radius+x>=img->width  ) return error;
	if ( radius>y || radius+y>=img->height ) return error;

	for( xi=1 ; xi<=radius ; xi++ )
	for( yi=1 ; yi<=radius ; yi++ ) {
		// 4 Quadrants
		unsigned char *pix1 = imgGetPixel(img, x+xi, y+yi);
		unsigned char *pix2 = imgGetPixel(img, x-xi, y+yi);
		unsigned char *pix3 = imgGetPixel(img, x-xi, y-yi);
		unsigned char *pix4 = imgGetPixel(img, x+xi, y-yi);
		error 	+= imgGetPixelDifference(pix1,pix2) +
			   imgGetPixelDifference(pix1,pix3) +
			   imgGetPixelDifference(pix1,pix4) +
			   imgGetPixelDifference(pix2,pix3) +
			   imgGetPixelDifference(pix2,pix4) +
			   imgGetPixelDifference(pix3,pix4) ;
	}
	return error;
}

void imgPatternDifference( Image *img, Image *pat, Image *res,
		int x1, int y1, int x2, int y2)
{
	int x, y;
	for( x=x1 ; x<=x2 ; x++ )
	for( y=y1 ; y<=y2 ; y++ ) {
		int diff[3];
		int xi, yi;
		diff[0] = diff[1] = diff[2] = 0;
		for( xi=0 ; xi<pat->width ; xi++ )
                for( yi=0 ; yi<pat->height ; yi++ ) {
			unsigned char *pix = imgGetPixel(img, 
				x+xi-pat->width/2, 
				y+yi-pat->height/2);
			unsigned char *pat_pix = imgGetPixel(pat, xi, yi);
			diff[0] += abs(pix[0]-(int)(pat_pix[0]));
			diff[1] += abs(pix[1]-(int)(pat_pix[1]));	
			diff[2] += abs(pix[2]-(int)(pat_pix[2]));
		}
		imgSetPixel(res, x-x1, y-y1, diff[0], diff[1], diff[2]); 
	}
}


int imgFindPatternArea(	Image *img, 			// Image to analyze (where to search)
			Image *pat, 			// Pattern Image to find (what to search)
			int x1, int y1, int x2, int y2, // Rectangular area of img to use
			int *best_x, int *best_y) 	// Pixel location in img with best match
{
	int best_val = 99999;
	*best_x = *best_y = -1;
	int x, y;
	for( x=x1 ; x<=x2 ; x++ )
	for( y=y1 ; y<=y2 ; y++ ) {
		unsigned char * pix;
		int diff = 0;
		int xi, yi;
		float scalef = 1.0;
		unsigned char * pat_pix;
#ifdef AGC_LOCAL
		// Use Local Automaic Gain Control
		int sum = 0;
                for( xi=0 ; xi<pat->width ; xi++ )
                for( yi=0 ; yi<pat->height ; yi++ ) {
                        pix = imgGetPixel(img, x+xi-pat->width/2, y+yi-pat->height/2);
                        sum += pix[0] + pix[1] + pix[2];
                }
		scalef = 384.*pat->width*pat->height/(float)sum;
#endif
		for( xi=0 ; xi<pat->width ; xi++ )
                for( yi=0 ; yi<pat->height ; yi++ ) {
			pix = imgGetPixel(img, 
				x+xi-pat->width/2, 
				y+yi-pat->height/2);
			pat_pix = imgGetPixel(pat, xi, yi);
			diff += abs(pix[0]-(int)(pat_pix[0]*scalef));
			diff += abs(pix[1]-(int)(pat_pix[1]*scalef));	
			diff += abs(pix[2]-(int)(pat_pix[2]*scalef));
		}
		if (diff<best_val) {
			best_val = diff;
			*best_x = x;
			*best_y = y;
		}
	}
	// Return minimum difference found
	return best_val;
}

int imgFindPattern(Image *img, Image *pat, int *best_x, int *best_y)
{
	return imgFindPatternArea(img, pat, 
		pat->width/2, pat->height/2, 
		img->width-pat->width/2, img->height-pat->height/2,
		best_x, best_y);
}


void imgMakeSymmetricX(Image *img)
{
	int x, y, r;
	for( y=0 ; y<img->height ; y++ ) 
		for( x=0, r=img->width-1 ; x<r ; x++, r-- ) {
			unsigned char *p1, *p2;
			p1 = imgGetPixel(img, x, y);
			p2 = imgGetPixel(img, r, y);
			p1[0] = p2[0] = (p1[0]+p2[0])/2;
			p1[1] = p2[1] = (p1[1]+p2[1])/2;
			p1[2] = p2[2] = (p1[2]+p2[2])/2;
		}
}

void imgMakeSymmetricY(Image *img)
{
        int x, y, r;
        for( y=0 , r=img->height-1 ; y<r ; y++, r-- )
                for( x=0 ; x<img->width ; x++ ) {
                        unsigned char *p1, *p2;
                        p1 = imgGetPixel(img, x, y);
                        p2 = imgGetPixel(img, x, r);
                        p1[0] = p2[0] = (p1[0]+p2[0])/2;
                        p1[1] = p2[1] = (p1[1]+p2[1])/2;
                        p1[2] = p2[2] = (p1[2]+p2[2])/2;
                }
}

void imgMakeSymmetric(Image *img)
{
	imgMakeSymmetricX(img);
	imgMakeSymmetricY(img);
}

unsigned int imgGetWidth(Image * img)
{
	return img->width;
}


unsigned int imgGetHeight(Image * img)
{
	return img->height;
}


void imgSetPixel(Image * img, unsigned int x, unsigned int y, char r, char g, char b)
{
    if ( img->depth==24 ) {
	// calculate the offset into the image array
	uint32_t offset = 3 * (x + (y * img->width));
	// set the rgb value
	img->data[offset + 2] = b;
	img->data[offset + 1] = g;
	img->data[offset + 0] = r;
    }
    else {	// 8 bit/pixel
	uint32_t offset = x + (y * img->width);
	img->data[offset] = (r+g+b)/3;
    }
}


// returns a pointer to the rgb tuple
unsigned char *imgGetPixel(Image * img, unsigned int x, unsigned int y)
{
	uint32_t offset = 3 * (x + (y * img->width));
	return (unsigned char *)(img->data + offset);
}

int imgGetPixelDifference(unsigned char *p1, unsigned char *p2)
{
	return abs(p1[0]-p2[0]) + abs(p1[1]-p2[1]) + abs(p1[2]-p2[2]);
}

// Destroys the image
void imgDestroy(Image * img)
{
	// Free the SDL surface
	//SDL_FreeSurface(img->sdl_surface);
	if(img->mem_ptr != NULL){
		free(img->mem_ptr);
	}
	// Free the image container, Python will handle the memoryview + buffer
	free(img);
}


int imgSaveRAW(Image *img, char *fname) {
    printf("escrevendo frame com %d bytes, para '%s'\n",img->depth/8,fname);
    int outfd = open(fname, O_WRONLY);
    if ( outfd==-1 ) {
	perror(strcat("Opening file ",fname));
        return 1;
    }
    unsigned int image_len = img->width*img->height*img->depth/8;
    int res = write(outfd, img->data, image_len);
    if ( res<1 ) {
	perror("Writting data");
    }
    close(outfd);
 
    return 0;
}

int imgSavePPM(Image *img, char *fname) {
    if ( img->depth != 24 ) {
	fprintf(stderr, "SavePPM is only avilable for 24 bit depth images\n");
	return 1;
    }
    printf("escrevendo frame com %d bytes/pix, para '%s'\n",img->depth/8,fname);
    FILE *outfd = fopen(fname, "w");
    if ( outfd==NULL ) {
	perror(strcat("Opening file ",fname));
        return 1;
    }
    
    fprintf(outfd, "P6\n%d %d\n255\n", img->width, img->height);
    unsigned int i;
    unsigned int image_len = img->width * img->height;
    unsigned char *img_ptr = img->data;
    for( i=0 ; i<image_len ; i++ ) {
	fputc(img_ptr[2], outfd);
	fputc(img_ptr[1], outfd);
	fputc(img_ptr[0], outfd);
	img_ptr +=3;
    }
    fclose(outfd);
    return 0;
}
