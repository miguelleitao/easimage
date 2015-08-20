/*
	easimage.h
*/

#ifndef _EASIMAGE_H_
#define _EASIMAGE_H_

#include <linux/videodev2.h>
#include <SDL/SDL.h>

#define RGB24 	V4L2_PIX_FMT_RGB24
#define BGR24 	V4L2_PIX_FMT_BGR24
#define YUYV 	V4L2_PIX_FMT_YUYV
#define RGB32 	V4L2_PIX_FMT_RGB32
#define RGBA32   V4L2_PIX_FMT_RGB32
#define RGB48	V4L2_PIX_FMT_RGB48


#define min(a,b) ( a<b ? a : b )
#define max(a,b) ( a>b ? a : b )

// forward declarations of internal types
struct Buffer;


typedef struct {
	unsigned int width;
	unsigned int height;
	unsigned int format;
	char * name;
	int handle;
	struct Buffer * buffers;
	unsigned int n_buffers;
} Camera;


typedef struct {
	unsigned int width;
	unsigned int height;
	unsigned short int depth;
	unsigned int format;
	unsigned char * mem_ptr;
	unsigned char * data;
	char *name;
} Image;


typedef struct {
	unsigned int width;
	unsigned int height;
	SDL_Surface * screen;
} Viewer;



/* Utility operations */
void init_easimage();
void quit_easimage();
void waitTime(unsigned int milliseconds);


/* Webcam operations */
Camera * camOpen(char *dev_name, unsigned int width, unsigned int height, int format);
unsigned int camGetWidth(Camera * cam);
unsigned int camGetHeight(Camera * cam);
Image * camGrabNewImage(Camera * cam);
int camGrabImage(Camera * cam, Image * img);
void camClose(Camera * cam);
int camPrintCaps(Camera *cam);


/* Image operations */
Image * imgNew(unsigned int width, unsigned int height, unsigned short int depth);
Image * imgFromBitmap(const char * filename);
Image * imgFromPPM(const char * filename);
int 	imgSavePPM(Image *img, char *fname);
int     imgSavePAM(Image *img, char *fname);
Image * imgCopy(Image * img);
void 	imgScale(Image *img, unsigned int sfactor);
Image * imgCrop(Image *img, int x1, int y1, int x2, int y2);
int	imgFindPattern(Image *img, Image *pattern, int *x, int *y);
int	imgFindPatternArea(Image *img, Image *pattern, int x1, int y1, int x2, int y2, int *x, int *y);
void 	imgDestroy(Image * img);
void 	imgMakeSymmetricX(Image *img);
void 	imgMakeSymmetricY(Image *img);
void 	imgMakeSymmetric(Image *img);
int 	imgGetSymmetryError( Image *img, int x, int y, int radius );
int 	imgGetPixelDifference(unsigned char *p1, unsigned char *p2);
float 	imgGetMeanArea(	Image *img, 			 // Image to analyze (where to search)
			int x1, int y1, int x2, int y2); // Rectangular area of img to use
float 	imgGetMean(Image *img);

unsigned int imgGetWidth(Image * img);
unsigned int imgGetHeight(Image * img);
void imgSetPixel(Image * img, unsigned int x, unsigned int y, unsigned char r, unsigned char g, unsigned char b);
void imgSetPixelRGB(Image * img, unsigned int x, unsigned int y, unsigned char r, unsigned char g, unsigned char b);
void imgSetPixelRGBA(Image * img, unsigned int x, unsigned int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a);

unsigned char * imgGetPixel(Image * img, unsigned int x, unsigned int y);
unsigned char * imgPixel(Image * img, unsigned int x, unsigned int y);


/* Viewer operations */
Viewer * viewOpen(unsigned int width, unsigned int height, const char * title);
void viewDisplayImage(Viewer * view, Image * img);
void viewClose(Viewer * view);


#endif // _IMGPROC_H_

