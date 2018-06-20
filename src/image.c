/**
 * @file 	image.c
 *
 * @author	Miguel Leitao
 *
 *
 */


#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include <SDL/SDL.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include "easimage.h"

/**  
 *  \addtogroup image
 *  @{
 */

//! Creates a new image
/*!
 *  Allocates a memory block to store an Image structure.
 *  Image is created with undefined colors.
 *  Image can then be released by calling imgDestroy() function.
 *  @param width the number of columns
 *  @param height the number of rows
 *  @param depth the pixel size in bits
 *  @return The address of the new allocated Image
 */
Image * imgNew(unsigned int width, unsigned int height, unsigned short depth)
{
	// Allocate for the image container
	Image * img = malloc(sizeof(Image));
	if(img == NULL){
		fprintf(stderr, "Failed to allocate memory for image container\n");
		return NULL;
	}

	// Set the width and height
	img->width = width;
	img->height = height;

	img->depth = depth;
	img->format = 0;
	img->name = NULL;


	// allocate for image data, depth/8 byte per pixel,
	// aligned to an 8 byte boundary
	img->mem_ptr = malloc(img->width * img->height * depth/8 + 8);
	if(img->mem_ptr == NULL){
		fprintf(stderr, "Memory allocation of image data failed\n");
		free(img);
		return NULL;
	}

	if ( depth==8 ) img->format = V4L2_PIX_FMT_GREY;

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

//! Loads an image from a BMP image file
/*!
 *  Creates a new Image using the data read from the specified BMP image file.
 *  Image can then be released by calling imgDestroy() function.
 *  @param filename the name of the BMP image file
 *  @return The address of the new loaded Image
 */
Image *imgFromBitmap(const char * filename)
{
	// Load the Bitmap
	SDL_Surface * bitmap = SDL_LoadBMP(filename);

	// Allocate for the image container
	Image * img = malloc(sizeof(Image));
	if(img == NULL){
		fprintf(stderr, "Failed to allocate memory for image container\n");
		return NULL;
	}

	// Set the width and height
	img->width = bitmap->w;
	img->height = bitmap->h;
	img->format = RGB24;
	img->depth = 24;
	img->name = strdup(filename);

	// set the data pointer
	img->data = bitmap->pixels;	

	// set the memory pointer to NULL, so we don't cause mayhem trying to free it
	img->mem_ptr = NULL;

	// Free SDL_Surface Container
	free(bitmap);

	// return the new image
	return img;
}
//! Loads an image from a PPM image file
/*!
 *  Creates a new Image using the data read from the specified PPM image file.
 *  Image can then be released by calling imgDestroy() function.
 *  @param filename the name of the PPM image file
 *  @return The address of the new loaded Image
 */
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
        img->name = strdup(filename);
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

//! Scales an image
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

//! Creates a copy of an Image
/*!
 *  Creates a new image storing a copy of an original Image.
 *  Created Image can then be released by calling the imgDestroy() function.
 *
 *  @param img the source Image (Image to crop)
 *  @return The address of the new copied Image
 */

Image *imgCopy(Image * img)
{
	// Create a new empty image
	Image * copy = imgNew(img->width, img->height, img->depth);

	copy->format = img->format;
	
	// Copy the data between the images
	memcpy(copy->data, img->data, img->width * img->height * img->depth/8 );

	// return the copy
	return copy;	
}

//! Crops an image
/*!
 *  Copies a rectangular area to a new Image.
 *  Image is created with undefined colors.
 *  Image can then be released by calling the imgDestroy() function.
 *  @param img the source Image (Image to crop)
 *  @param x1 the column number of first vertex
 *  @param y1 the row number of first vertex
 *  @param x2 the column number of second vertex
 *  @param y2 the row number of second vertex
 *  @return The address of the new cropped Image
 */
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

//! Evaluates simmetry error at location
/*!
 *  Evaluates the simmetry error of an image's square area.
 *  Computed error will be zero if the squre area is fully simmetric along the X and Y axis.
 *  
 *  @param img Image to be processed
 *  @param x the column number of the image area to be evaluated
 *  @param y the row number of the image area to be evaluated
 *  @param radius the half side dimension of the square area
 *  @return The total simmetry error
 */
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

//! Searches image for a pattern 
/*!
 *  Searches full specified area from Image @p img for ocurrences of pattern Image @p pat.
 *  Produces resulting Image @p res with calculated differences.
 *
 *  @param img Image to be searched.
 *  @param pat Image pattern to search for.
 *  @param res Previously allocated Image where resulting differences will be stored.
 *  	       If @p res equals NULL, a new Image is created.
 *  @param x1 the column number of the top left corner of the image area to be searched
 *  @param y1 the row number of the top left corner of the image area to be searched
 *  @param x2 the column number of the bottom right corner of the image area
 *  @param y2 the row number of the bottom right corner of the image area to be searched
 *  @return the address of the resulting Image
 */
Image *imgPatternDifference( Image *img, Image *pat, Image *res,
		int x1, int y1, int x2, int y2)
{
	// res = abs(img-pat)
	// res must have size: x2-x1+, y2-y1+1
	if ( ! res ) res = imgNew(x2-x1,y2-y1,24);
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
		imgSetPixelRGB(res, x-x1, y-y1, diff[0], diff[1], diff[2]); 
	}
	return res;
}


//! Evaluates the addition of pixel components.
/*!
 *  Evaluates the total addition of all (color) components' values from all pixels within the specified area of the Image @p img.
 *
 *  @param img Image to be processed.
 *  @param x1 the column number of the top left corner of the image area to be processed
 *  @param y1 the row number of the top left corner of the image area to be processed
 *  @param x2 the column number of the bottom right corner of the image area to be processed
 *  @param y2 the row number of the bottom right corner of the image area to be processed
 *  @return The calculated total value as an int 
 */
int imgGetSumArea(   Image *img,                     // Image to analyze (where to search)
                        int x1, int y1, int x2, int y2) // Rectangular area of img to use
{
        int x, y, p;
        int total = 0;
        int comp = img->depth/8;
        for( x=x1 ; x<=x2 ; x++ )
        for( y=y1 ; y<=y2 ; y++ ) {
                unsigned char * pix = imgGetPixel(img,x,y);
                for( p=0 ; p<comp ; p++ )
                        total += pix[p];
        }
        return total;
}

//! Evaluates the pixel component mean value.
/*!
 *  Evaluates the mean value from all (color) components and all pixel within the specified area of the Image @p img.
 *
 *  @param img Image to be processed.
 *  @param x1 the column number of the top left corner of the image area to be processed
 *  @param y1 the row number of the top left corner of the image area to be processed
 *  @param x2 the column number of the bottom right corner of the image area to be processed
 *  @param y2 the row number of the bottom right corner of the image area to be processed
 *  @return the calculated mean value as a float
 */
float imgGetMeanArea(	Image *img, 			// Image to analyze (where to search)
			int x1, int y1, int x2, int y2) // Rectangular area of img to use
{
	int total = imgGetSumArea(img,x1,y1,x2,y2);
	int comp = img->depth/8;
	return (float)total / (float)( (x2-x1+1) * (y2-y1+1) * comp );
}

//! Evaluates the pixel component mean value.
/*!
 *  Evaluates the mean value from all (color) components and all pixels within the Image @p img.
 *
 *  @param img Image to be processed.
 *  @return the calculated mean value as a float
 */
float imgGetMean(Image *img) {
	return imgGetMeanArea(img, 0, 0, img->width, img->height);
}

//! Searches image area for a pattern
/*!
 *  Searches the full specified area from Image @p img for ocurrences of pattern Image @p pat.
 *  Selectes and informs the location of the best matching pattern.
 *
 *  @param img Image to be searched.
 *  @param pat Image pattern to search for.
 *  @param x1 the column number of the top left corner of the image area to be searched
 *  @param y1 the row number of the top left corner of the image area to be searched
 *  @param x2 the column number of the bottom right corner of the image area
 *  @param y2 the row number of the bottom right corner of the image area to be searched
 *  @param best_x location to store the column number of the selected location.
 *  @param best_y location to store the row number of the selected location.
 *  @return The matching error for the selected location.
 */
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

//! Searches image for a pattern
/*!
 *  Searches the full specified area from Image @p img for ocurrences of pattern Image @p pat.
 *  Selectes and informs the location of the best matching pattern.
 *
 *  @param img Image to be searched.
 *  @param pat Image pattern to search for.
 *  @param best_x location to store the column number of the selected location.
 *  @param best_y location to store the row number of the selected location.
 *  @return The matching error for the selected location.
 */
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

int imgGetSymmetricError(Image *img, int x1, int x2, int y1, int y2)
{
	int error = 0;
	int xi1,yi1,xi2,yi2;
	for( xi1=x1, xi2=x2 ; xi1<xi2 ; xi1++, xi2-- )
	for( yi1=y1, yi2=y2 ; yi1<yi2 ; yi1++, yi2-- ) {
		unsigned char *p1, *p2;
                p1 = imgGetPixel(img, xi1, yi1);
                p2 = imgGetPixel(img, xi2, yi2);
		error += abs( p1[0]-p2[0] ) + abs( p1[1]-p2[1] ) + abs( p1[2]-p2[2] );
	}
	return error;
}

unsigned int imgGetWidth(Image * img)
{
	return img->width;
}


unsigned int imgGetHeight(Image * img)
{
	return img->height;
}


void imgSetPixel(Image * img, unsigned int x, unsigned int y, unsigned char *pdata)
{
    // calculate the offset into the image array
    uint32_t offset = img->depth/8 * (x + (y * img->width));
    register int c;
    for( c=0 ; c<img->depth/8 ; c++ )
	img->data[offset+c] = pdata[c];
}


void imgSetPixelRGB(Image * img, unsigned int x, unsigned int y, unsigned char r, unsigned char g, unsigned char b)
{
    // calculate the offset into the image array
    uint32_t offset = img->depth/8 * (x + (y * img->width));
    if ( img->depth>=24 ) {
	// set the rgb value
	if ( img->format==BGR24 ) {
		img->data[offset + 0] = b;
		img->data[offset + 1] = g;
		img->data[offset + 2] = r;
	}
	else {
		img->data[offset + 0] = r;
		img->data[offset + 1] = g;
		img->data[offset + 2] = b;
	}
    }
    else {	// 8 bit/pixel
	img->data[offset] = (r+g+b)/3;
    }
}

void imgSetPixelRGBA(Image * img, unsigned int x, unsigned int y, 
		unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    // calculate the offset into the image array
    uint32_t offset = img->depth/8 * (x + (y * img->width));
    if ( img->depth>=24 ) {
	// set the rgb value
	switch ( img->format ) {
	    case BGR24:
		img->data[offset + 0] = b;
		img->data[offset + 1] = g;
		img->data[offset + 2] = r;
		break;
	    case RGB24:
		img->data[offset + 0] = r;
		img->data[offset + 1] = g;
		img->data[offset + 2] = b;
		break;
	    case RGBA32:
		img->data[offset + 0] = r;
		img->data[offset + 1] = g;
		img->data[offset + 2] = b;
		img->data[offset + 3] = a;
		break;
	    default:
		fprintf(stderr,"SetPixelRGBA not implemented for this format\n");
	}
    }
    else {	// 8 bit/pixel
	uint32_t offset = x + (y * img->width);
	img->data[offset] = (r+g+b)/3;
    }
}

//! Gets pixel data
/*! Returns a pointer to the pixel data
 *  @param img Image structure location
 *  @param x pixel column number
 *  @param y pixel row number
 *  @return A pointer to the pixel data array
 */
unsigned char *imgGetPixel(Image * img, unsigned int x, unsigned int y)
{
	uint32_t offset = img->depth/8 * (x + (y * img->width));
	return (unsigned char *)(img->data + offset);
}

int imgGetPixelDifference(unsigned char *p1, unsigned char *p2)
{
	return abs(p1[0]-p2[0]) + abs(p1[1]-p2[1]) + abs(p1[2]-p2[2]);
}

//! Image convolution
/*! Performs a convolution beteween Image @p img1 and Image @p img2.
 *  Convolution result is stored in preallocated Image @p res. 
 *  Image @p res should have dimensions and depth matching Image @p img1.
 *  If @p res equals NULL, a new Image is allocated.
 *  Implementation is optimized for @p img1 larger than @p img2.
 *  This is usually adequated for processing Image @p img1 through kernel Image @p img2.
 *  Image @p img2 should have odd dimensions.
 *  Image @p img2 must use a pixel depth of 8 bits or the same pixel depth as Image @p img1.
 *  @param img Image structur location
 *  @param x pixel column number
 *  @param y pixel row number
 *  @return The convolution resulting Image.
 */
Image * imgConvolution(Image *img1, Image *img2, Image *res)
{
	unsigned long fscale = 255 * img2->width * img2->height;
	if ( ! res ) res = imgNew(img1->width,img1->height,img1->depth);
	int x1, y1, x2, y2;
	const int xc = img2->width / 2;
	const int yc = img2->height / 2;
	for( x1=0 ; x1<img1->width ; x1++ )
	for( y1=0 ; y1<img1->height ; y1++ ) {
	    unsigned long acc[img1->depth/8];
	    register int c;
	    for( c=0 ; c<img1->depth/8 ; c++ )
		acc[c] = 0L;
	    for( x2=0 ; x2<img2->width ; x2++ ) {
		register int xx = x1+x2-xc;
		if ( xx<0 || xx>=img1->width ) continue;
	    	for( y2=0 ; y2<img2->height ; y2++ ) {
		    register int yy = y1+y2-yc;
		    if ( yy<0 || yy>=img1->height ) continue;
		    unsigned char *pix1 = imgGetPixel(img1,xx,yy);
		    unsigned char *pix2 = imgGetPixel(img2,x2,y2);
	    	    for( c=0 ; c<img1->depth/8 ; c++ ) 
			if ( img2->depth <= 8 )
			    acc[c] += pix1[c] * pix2[0];
		    	else
		    	    acc[c] += pix1[c] * pix2[c];
	    	}
	    }
	    unsigned char rpix[img1->depth/8];
	    for( c=0 ; c<img1->depth/8 ; c++ )
                rpix[c] = acc[c]/fscale;
	    imgSetPixel(res,x1,y1,rpix);
	}
	return res;
}

Image *imgCreateGaussian(int dim, float sig)
{
    //const float sig = 40.;
    if ( sig<=0.f ) sig = (float)dim/5.;
    Image *k = imgNew(dim,dim,8);
    //k->format = RGB24;
    k->format = GREY;
    int x,y;
    const int dc = dim/2;
    for( x=0 ; x<=dc ; x++ )
    for( y=0 ; y<=dc ; y++ ) {
	float xf = x-dc;
	float yf = y-dc;
	float kf = exp(-(xf*xf+yf*yf)/(2.*sig*sig));
	unsigned char kv = kf*255.;
	//printf("%d %d: %u\n",x,y,kv);
	//unsigned char kvv[3] = { kv, kv, kv };
	unsigned char kvv[3] = { kv };
	imgSetPixel(k,x,y,kvv);
	imgSetPixel(k,x,dim-y-1,kvv);
	imgSetPixel(k,dim-x-1,y,kvv);
	imgSetPixel(k,dim-x-1,dim-y-1,kvv);
    }
    return k;
}

//! Destroys the image
void imgDestroy(Image * img)
{
	if ( img==NULL ) {
		fprintf(stderr,"Cannot destroy NULL image\n");
		return;
	}
	// Free the SDL surface
	//SDL_FreeSurface(img->sdl_surface);
	if(img->mem_ptr != NULL){
		free(img->mem_ptr);
	}
	// Free the image container
	free(img);
}


int imgSaveRAW(Image *img, char *fname) {
    printf("writing frame with %d bytes, into file '%s'\n",img->depth/8,fname);
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
    printf("writting frame with %d bytes/pix, into '%s'\n",img->depth/8,fname);
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

int imgSavePAM(Image *img, char *fname) {
    printf("writting frame with %d bytes/pix, into '%s'\n",img->depth/8,fname);
    FILE *outfd = fopen(fname, "w");
    if ( outfd==NULL ) {
        perror(strcat("Opening file ",fname));
        return 1;
    }

    fprintf(outfd, "P7\nWIDTH %d\nHEIGHT %d\nDEPTH %d\nMAXVAL 255\n", img->width, img->height, img->depth/8);
    if ( img->depth>24 ) 	fprintf(outfd, "TUPLTYPE RGB_ALPHA\n");
    else if ( img->depth<=8 )	fprintf(outfd, "TUPLTYPE GRAYSCALE\n");
    else 			fprintf(outfd, "TUPLTYPE RGB\n");
    fprintf(outfd, "ENDHDR\n");
    unsigned int i;
    unsigned int image_len = img->width * img->height;
    unsigned char *img_ptr = img->data;
    switch ( img->format ) {
	case BGR24:
	    for( i=0 ; i<image_len ; i++ ) {
		fputc(img_ptr[2], outfd);
		fputc(img_ptr[1], outfd);
		fputc(img_ptr[0], outfd);
		img_ptr += 3;
	    }
	    break;
	case RGB24:
	    for( i=0 ; i<image_len ; i++ ) {
		fputc(img_ptr[0], outfd);
		fputc(img_ptr[1], outfd);
		fputc(img_ptr[2], outfd);
		img_ptr += 3;
	    }
	    break;
	case RGBA32:
	    for( i=0 ; i<image_len ; i++ ) {
		fputc(img_ptr[0], outfd);
		fputc(img_ptr[1], outfd);
		fputc(img_ptr[2], outfd);
		fputc(img_ptr[3], outfd);
		img_ptr += 4;
	    }
	    break;
	case GREY:
	    for( i=0 ; i<image_len ; i++ ) {
		fputc(img_ptr[i], outfd);
	    }
	    break;
    }		
    fclose(outfd);
    return 0;
}

/**
 *  @}
 */
