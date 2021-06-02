/**
 * @file 	easimage.h
 *
 * @author	Miguel Leitao
 *
 * Easimage headerfile
 *
 */

#ifndef _EASIMAGE_H_
#define _EASIMAGE_H_

#include <linux/videodev2.h>
#include <SDL/SDL.h>

#define QUOTE(name) #name
#define STR(macro) QUOTE(macro)

/** \defgroup format Image storing formats 
 *  \addtogroup format
 *  @{
 *  Image storing formats
 */
#define RGB24 	V4L2_PIX_FMT_RGB24
#define BGR24 	V4L2_PIX_FMT_BGR24
#define YUYV 	V4L2_PIX_FMT_YUYV
#define RGB32 	V4L2_PIX_FMT_RGB32
#define RGBA32  V4L2_PIX_FMT_RGB32
#define RGB48	V4L2_PIX_FMT_RGB48
#define GREY	V4L2_PIX_FMT_GREY
#define MJPEG   V4L2_PIX_FMT_MJPEG
/** @}*/

#define min(a,b) ( a<b ? a : b )
#define max(a,b) ( a>b ? a : b )

//! Memory block to store image data
//forward declarations of internal types
struct Buffer;

//! Represents an image capturing device
typedef struct {
	unsigned int width;	 	///< The width of the camera frame (Number of columns)
	unsigned int height;	 	///< The height of the camera frame  (Number of rows)
	unsigned int format;		///< Format identifier
	char *name;	 	 	///< The name of the device
	int handle;		 	///< The stream handle
	struct Buffer *buffers;  	///< location of image buffers
	unsigned int n_buffers;	 	///< Number of allocated buffers
} Camera;

//! Stores an image
typedef struct {
	unsigned int width;		///< The width of the image (Number of columns)
	unsigned int height;		///< The height of the image (Number of rows)
	unsigned short int depth;	///< The depth of the image (number of bit per pixel)
	unsigned int format;		///< Format identifier
	unsigned char *mem_ptr;		///< location of image buffers
	unsigned char *data;		///< location of pixel data
	char *name;	 	 	///< The name of the image
} Image;

//! Represents an image presenting device
typedef struct {
	unsigned int width;		///< The width of the image (Number of columns)
	unsigned int height;		///< The height of the image (Number of rows)
	SDL_Surface *screen;		///< Pointer to screen surface
} Viewer;


/** \defgroup util Utility functions 
 *  \addtogroup util
 *  @{
 *  Utility functions
 */
/** @brief Init easimage */
void init_easimage();
/** @brief Quit easimage */
void quit_easimage();
void waitTime(unsigned int milliseconds);
/** @}*/

/** \defgroup camera Camera operations 
 *  \addtogroup camera
 *  @{
 *  Functions to get info and image data from a connected camera
 */
Camera * camOpen(char *dev_name, unsigned int width, unsigned int height, int format);
unsigned int camGetWidth(Camera * cam);
unsigned int camGetHeight(Camera * cam);
Image * camGrabNewImage(Camera * cam);
int camGrabImage(Camera * cam, Image * img);
void camClose(Camera * cam);
int camPrintCaps(Camera *cam);
/** @}*/

/** \defgroup image Image operations 
 *  \addtogroup image
 *  @{
 *  Functions to create and process image structures
 */	
Image  *imgNew(unsigned int width, unsigned int height, unsigned short depth);
Image  *imgFromBitmap(const char *filename);
Image  *imgFromPPM(const char *filename);
Image  *imgFromFile(const char *filename);
int 	imgSavePPM(Image *img, char *fname);
int     imgSavePAM(Image *img, char *fname);
int	imgSaveRAW(Image *img, char *fname);
Image  *imgCopy(Image * img);
void 	imgScale(Image *img, unsigned int sfactor);
Image  *imgCrop(Image *img, int x1, int y1, int x2, int y2);
Image  *imgCreateGaussian(int dim, float sig);
Image  *imgConvolution(Image *img1, Image *img2, Image *res);
int	imgFindPattern(Image *img, Image *pattern, int *x, int *y);
int	imgFindPatternArea(Image *img, Image *pattern, int x1, int y1, int x2, int y2, int *x, int *y);
void 	imgDestroy(Image * img);
void 	imgMakeSymmetricX(Image *img);
void 	imgMakeSymmetricY(Image *img);
void 	imgMakeSymmetric(Image *img);
int 	imgGetSymmetryError(Image *img, int x, int y, int radius);
Image  *imgPatternDifference( Image *img, Image *pat, Image *res, int x1, int y1, int x2, int y2);
int 	imgGetPixelDifference(unsigned char *p1, unsigned char *p2);

int     imgGetSumArea( Image *img,                      // Image to analyze 
                        int x1, int y1, int x2, int y2);
float 	imgGetMeanArea(	Image *img, 			 // Image to analyze (where to search)
			int x1, int y1, int x2, int y2); // Rectangular area of img to use
float 	imgGetMean(Image *img);

unsigned int imgGetWidth(Image * img);
unsigned int imgGetHeight(Image * img);
void 	 imgSetPixel(Image * img, unsigned int x, unsigned int y, unsigned char *pdata);
void 	 imgSetPixelRGB(Image * img, unsigned int x, unsigned int y, unsigned char r, unsigned char g, unsigned char b);
void 	 imgSetPixelRGBA(Image * img, unsigned int x, unsigned int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a);

unsigned char * imgGetPixel(Image * img, unsigned int x, unsigned int y);
unsigned char * imgPixel(Image * img, unsigned int x, unsigned int y);
/** @}*/

/* Viewer operations */
/** \defgroup view Viewer operations 
 *  \addtogroup view
 *  @{
 *  Functions to display images
 */	
Viewer * viewOpen(unsigned int width, unsigned int height, const char * title);
//! Displays an Image
/*! 
 *  Image @p img is presented on viewer window @p view.
 *  Viewer window must be previously created using @c viewOpen()
 */
void viewDisplayImage(Viewer *view, Image *img);
//! Terminates the viewer
/*!
 *  Viwer @p view is terminated and window is closed.
 *  Viewer window must be previously created using @c viewOpen()
 */
void viewClose(Viewer *view);
//! Polls for currently pending events
/*!
 *  Polls for currently pending events, and returns 1 if there are any pending events, or 0 if there are none available.
 */
int  viewPollEvent(SDL_Event *event);
/** @}*/

#endif // _EASIMAGE_H_

