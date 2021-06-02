/**
 * @file	camera.c
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include <getopt.h>             /* getopt_long() */

#include <fcntl.h>             /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <asm/types.h>          /* for videodev2.h */


#include "easimage.h"

// Should be moved to .h file
struct Buffer {
	struct v4l2_buffer buf;
	unsigned char *start;
};

static void errno_exit(const char *s)
{
        fprintf (stderr, "%s error %d, %s\n",
			s, errno, strerror (errno));

        exit (EXIT_FAILURE);
}


static int xioctl(Camera * cam, int request, void *arg)
{
        int r;

        do r = ioctl (cam->handle, request, arg);
        while (-1 == r && EINTR == errno);

        return r;
}

int camPrintCaps(Camera *cam)
{
	if ( cam->handle==-1 ) {
		fprintf(stderr,"Camera '%s' not opened\n",cam->name);
		return 1;
	}
	printf("Camera name: %s\n", cam->name);
	printf("Frame Size: %ux%u\n", cam->width,cam->height);
        struct v4l2_capability caps = {};
        if (-1 == xioctl(cam, VIDIOC_QUERYCAP, &caps))
        {
                perror("Querying Capabilities");
                return 1;
        }
 
        printf( "Driver Caps:\n"
                "  Driver: \"%s\"\n"
                "  Card: \"%s\"\n"
                "  Bus: \"%s\"\n"
                "  Version: %d.%d\n"
                "  Capabilities: %08x\n",
                caps.driver,
                caps.card,
                caps.bus_info,
                (caps.version>>16)&&0xff,
                (caps.version>>24)&&0xff,
                caps.capabilities);

  
        struct v4l2_cropcap cropcap = {0};
        cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (-1 == xioctl (cam, VIDIOC_CROPCAP, &cropcap))
        {
                printf("Cropping Capabilities not available.");
         
        } else
	{
            printf( "Camera Cropping:\n"
                "  Bounds: %dx%d+%d+%d\n"
                "  Default: %dx%d+%d+%d\n"
                "  Aspect: %d/%d\n",
                cropcap.bounds.width, cropcap.bounds.height,
		cropcap.bounds.left, cropcap.bounds.top,
                cropcap.defrect.width, cropcap.defrect.height,
		cropcap.defrect.left, cropcap.defrect.top,
                cropcap.pixelaspect.numerator,
		cropcap.pixelaspect.denominator);
 	}
        struct v4l2_fmtdesc fmtdesc = {0};
        fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        char fourcc[5] = {0};
        char c, e;
        printf("  FMT : CE Desc\n--------------------\n");
        while (0 == xioctl(cam, VIDIOC_ENUM_FMT, &fmtdesc))
        {
                strncpy(fourcc, (char *)&fmtdesc.pixelformat, 4);
                c = fmtdesc.flags & 1? 'C' : ' ';
                e = fmtdesc.flags & 2? 'E' : ' ';
                printf("  %s: %c%c %s\n", fourcc, c, e, fmtdesc.description);
                fmtdesc.index++;
        }
 	return 0;
}

// routine to initialise memory mapped i/o on the camera device
static void init_mmap(Camera * cam)
{
	struct v4l2_requestbuffers req;

	memset (&(req), 0, sizeof (req));

	req.count               = 1;
	req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory              = V4L2_MEMORY_MMAP;

	if (-1 == xioctl (cam, VIDIOC_REQBUFS, &req)) {
		if (EINVAL == errno) {
			fprintf (stderr, "%s does not support "
					"memory mapping\n", cam->name);
			exit (EXIT_FAILURE);
		} else {
			errno_exit ("VIDIOC_REQBUFS");
		}
	}

	// allocate memory for the buffers
	cam->buffers = calloc (req.count, sizeof (*(cam->buffers)));

	if (!cam->buffers) {
		fprintf (stderr, "Out of memory\n");
		exit (EXIT_FAILURE);
	}

	for (cam->n_buffers = 0; cam->n_buffers < req.count; cam->n_buffers++) {
		struct v4l2_buffer buffer;

		memset (&(buffer), 0, sizeof (buffer));

		buffer.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buffer.memory      = V4L2_MEMORY_MMAP;
		buffer.index       = cam->n_buffers;

		if (-1 == xioctl (cam, VIDIOC_QUERYBUF, &buffer)){
			errno_exit ("VIDIOC_QUERYBUF");
		}

		// copy the v4l2 buffer into the device buffers
		cam->buffers[cam->n_buffers].buf = buffer;
		// memory map the device buffers
		cam->buffers[cam->n_buffers].start = 
			mmap( NULL, // start anywhere
				  buffer.length,
				  PROT_READ | PROT_WRITE, // required
				  MAP_SHARED, // recommended
				  cam->handle,
				  buffer.m.offset
			);

		if (MAP_FAILED == cam->buffers[cam->n_buffers].start){
			errno_exit ("mmap");
		}
	}
}


// returns an index to the dequeued buffer
static unsigned int camDequeueBuffer(Camera * cam)
{
	while(1){
		fd_set fds;
		struct timeval tv;
		int r;
	
		FD_ZERO (&fds);
		FD_SET (cam->handle, &fds);

		// Timeout.
		tv.tv_sec = 20;
		tv.tv_usec = 0;
		
		r = select (cam->handle + 1, &fds, NULL, NULL, &tv);

		if (-1 == r) {
			if (EINTR == errno){
				printf("Repeting select\n");
				continue;
			}
			errno_exit ("select");
		}

		if (0 == r) {
			fprintf (stderr, "select timeout\n");
			exit (EXIT_FAILURE);
		}

		// read the frame
		struct v4l2_buffer buffer;
		memset (&(buffer), 0, sizeof (buffer));

		buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buffer.memory = V4L2_MEMORY_MMAP;
		
		// dequeue a buffer
		if (-1 == xioctl (cam, VIDIOC_DQBUF, &buffer)) {
			switch (errno) {
				case EAGAIN:
					continue;

				case EIO:
					/* Could ignore EIO, see spec. */
					/* fall through */

				default:
					errno_exit ("VIDIOC_DQBUF");
			}
		}
		assert (buffer.index < cam->n_buffers);

		// return the buffer index handle to the buffer
		return buffer.index;
	}
}


// enqueue a given device buffer to the device
static void camEnqueueBuffer(Camera * cam, unsigned int buffer_id)
{
	// enqueue a given buffer by index
	if(-1 == xioctl(cam, VIDIOC_QBUF, &(cam->buffers[buffer_id].buf) )){
		errno_exit("VIDIOC_QBUF");
	}

	return;
}

char *pixFormatName(unsigned int format, char* name) {
        static char localName[20];
	if ( !name ) name = localName;
	switch( format ) {
		case BGR24:
			strcpy(name,"BGR24");
			break;
                case RGB24:
                        strcpy(name,"RGB24");
                        break;
                case YUYV:
                        strcpy(name,"YUYV");
                        break;
                case MJPEG:
                        strcpy(name,"MJPEG");
                        break;
                default: 
                        strcpy(name,"unknown");
                        break;
	}
	return name;
}


void BGR24_to_RGB24(	unsigned char *in, 
				unsigned char *out,
				unsigned int nPixels )
{
	uint32_t i;
	if ( in==out ) {
		// iterate 1 pixels at a time, so 3 bytes for RGB or GBR
		for(i = 0 ; i < nPixels ; i++ ) {
			unsigned char temp = in[2];
			in[2] = in[0];
			in[0] = temp;
			in += 3;
		}
		return;
	}
	// iterate 1 pixels at a time, so 3 bytes for RGB or GBR
	for(i = 0 ; i < nPixels ; i++ ) {
		out[0] = in[2];
		out[1] = in[1];
		out[2] = in[0];
		in += 3;
		out += 3;
	}
}

void YUYV_to_RGB24(unsigned char *buffer_ptr, unsigned char *img_ptr, unsigned int nPixels) {

	int nBytes = nPixels * 2;
	// iterate 2 pixels at a time, so 4 bytes for YUV and 6 bytes for RGB
	uint32_t i, j;
	for(i = 0, j = 0; i < nBytes; i+=4, j+=6){
		// jml, May 2015
		// Add unsigned
		unsigned char * buffer_pos = buffer_ptr + i;
		unsigned char * img_pos = img_ptr + j;
		
		// YCbCr to RGB conversion (from: http://www.equasys.de/colorconversion.html);
		int y0 = buffer_pos[0];
		int cb = buffer_pos[1];
		int y1 = buffer_pos[2];
		int cr = buffer_pos[3];
		int r;
		int g;
		int b;

		// first RGB
		r = y0 + ((357 * cr) >> 8) - 179;

		g = y0 - (( 87 * cb) >> 8) +  44 - ((181 * cr) >> 8) + 91;
		b = y0 + ((450 * cb) >> 8) - 226;
		// clamp to 0 to 255
		img_pos[2] = r > 254 ? 255 : (r < 0 ? 0 : r);
		img_pos[1] = g > 254 ? 255 : (g < 0 ? 0 : g);
		img_pos[0] = b > 254 ? 255 : (b < 0 ? 0 : b);
		
		// second RGB
		r = y1 + ((357 * cr) >> 8) - 179;
		g = y1 - (( 87 * cb) >> 8) +  44 - ((181 * cr) >> 8) + 91;
		b = y1 + ((450 * cb) >> 8) - 226;
		img_pos[5] = r > 254 ? 255 : (r < 0 ? 0 : r);
		img_pos[4] = g > 254 ? 255 : (g < 0 ? 0 : g);
		img_pos[3] = b > 254 ? 255 : (b < 0 ? 0 : b);
	}
}

void YUYV_to_BGR24(unsigned char *in, unsigned char *out,
			unsigned int size) {
	YUYV_to_RGB24(in,out,size);
	BGR24_to_RGB24(out,out,size);
}

int camGrabImage(Camera * cam, Image *img)
{
	// dequeue a buffer
	unsigned int buffer_id = camDequeueBuffer(cam);

	// Copy data across, converting to RGB along the way
	unsigned char * buffer_ptr = cam->buffers[buffer_id].start;
	unsigned char * img_ptr = img->data;
	unsigned int img_size = img->width * img->height;

	if ( cam->format == img->format ) {
	    memcpy(img->data, buffer_ptr, 
		img->width * img->height * img->depth/8);
	}
	else if ( (cam->format==RGB24 && img->format==BGR24) ||
		  (cam->format==BGR24 && img->format==RGB24) ) 
	    BGR24_to_RGB24( (unsigned char *)buffer_ptr, img_ptr, img_size );
	else if ( cam->format==YUYV && img->format==RGB24 )
	    YUYV_to_RGB24( (unsigned char *)buffer_ptr, img_ptr, img_size );
	else if ( cam->format==YUYV && img->format==BGR24 )
	    YUYV_to_BGR24( (unsigned char *)buffer_ptr, img_ptr, img_size );
	else {
	    fprintf(stderr,"camGrabImage() error: %s (%u->%u)\n",
		"The requested Pixel format conversion is not supported",cam->format,img->format); 
	    printf("Cam format is %s (%u)\n", pixFormatName(cam->format, NULL), cam->format);
	    printf("Img format is %s (%u)\n", pixFormatName(img->format, NULL), img->format);
	    
	}
	// requeue the buffer
	camEnqueueBuffer(cam, buffer_id);

	// return the image
	return 0;
}

Image * camGrabNewImage(Camera *cam) {
	int depth = 24;
	int format = cam->format;
	if ( format==YUYV ) depth = 16;
	Image *img = imgNew(cam->width, cam->height, depth);
	img->format = format;
	camGrabImage(cam, img);
	return img;
}


static void camSetFormat(Camera *cam, unsigned int width, unsigned int height, int format)
{
	printf("Setting device format\n");

	struct v4l2_format fmt;
	unsigned int min;

	if ( format==0 ) format = YUYV;

	memset (&(fmt), 0, sizeof (fmt));
	
	fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       = width; 
	fmt.fmt.pix.height      = height;
	fmt.fmt.pix.pixelformat = format;
	fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
	if (-1 == xioctl (cam, VIDIOC_S_FMT, &fmt)){
		errno_exit ("VIDIOC_S_FMT");
	}

    	// Note VIDIOC_S_FMT may change width and height.
	
	// Buggy driver paranoia.
	min = fmt.fmt.pix.width * 2;
	if (fmt.fmt.pix.bytesperline < min)
		fmt.fmt.pix.bytesperline = min;
	min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	if (fmt.fmt.pix.sizeimage < min)
		fmt.fmt.pix.sizeimage = min;

	// ONLY CHANGES IN IMAGE SIZE ARE HANDLED ATM
	// set device image size to the returned width and height.
	cam->width = fmt.fmt.pix.width;
	cam->height = fmt.fmt.pix.height;
	cam->format = fmt.fmt.pix.pixelformat;

	// initialise for memory mapped io
	init_mmap (cam);
	
	// initialise streaming for capture
	enum v4l2_buf_type type;
	
	// queue buffers ready for capture
	unsigned int i;
	for ( i = 0; i < cam->n_buffers; ++i) {
		// buffers are initialised, so just call the enqueue function
		camEnqueueBuffer(cam, i);		
	}
	
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	// turn on streaming
	if (-1 == xioctl (cam, VIDIOC_STREAMON, &type)){
		if(errno == EINVAL){
			fprintf(stderr, "buffer type not supported, or no buffers allocated or mapped\n");
			exit(EXIT_FAILURE);
		} else if(errno == EPIPE){
			fprintf(stderr, "The driver implements pad-level format configuration and the pipeline configuration is invalid.\n");
			exit(EXIT_FAILURE);
		} else {
			errno_exit ("VIDIOC_STREAMON");
		}
	}

}


// close video capture device
void camClose(Camera * cam)
{
	if ( cam==NULL ) {
		fprintf(stderr,"Invalid camera\n");
		return; 
	}

	//printf("Stopping camera capture\n");

	// stop capturing
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == xioctl (cam, VIDIOC_STREAMOFF, &type)){
		errno_exit ("VIDIOC_STREAMOFF");
	}

	// uninitialise the device
	unsigned int i;
	for (i = 0; i < cam->n_buffers; ++i){
		if(-1 == munmap(cam->buffers[i].start, cam->buffers[i].buf.length)){
			errno_exit("munmap");
		}
	}
	
	// free buffers
	free (cam->buffers);

	// close the device
	if (-1 == close(cam->handle)){
		errno_exit ("close");
	}
	cam->handle = -1;

	free(cam);
}


// Open a video capture device
Camera * camOpen(char *dev_name, unsigned int width, unsigned int height, int format)
{
	// printf("Opening the device\n");
	
	if ( dev_name==NULL )	dev_name = "/dev/video0";
	if ( format==0 )	format = YUYV;

	// initialise the device
	struct stat st; 

	if (-1 == stat (dev_name, &st)) {
		fprintf (stderr, "Cannot identify '%s': %d, %s\n",
			dev_name, errno, strerror (errno));
		exit (EXIT_FAILURE);
	}

	if (!S_ISCHR (st.st_mode)) {
		fprintf (stderr, "%s is no device\n", dev_name);
		exit (EXIT_FAILURE);
	}
	
	// set up the device
	Camera * cam = malloc(sizeof(*cam));
	if(cam == NULL){
		fprintf(stderr, "Could not allocate memory for camera device structure\n");
		exit(EXIT_FAILURE);
	}
	cam->handle = -1;
	cam->name = NULL;

	// open the device
	cam->handle = open(dev_name, O_RDWR | O_NONBLOCK, 0);
	if (-1 == cam->handle) {
		fprintf (stderr, "Cannot open '%s': %d, %s\n",
			 dev_name, errno, strerror (errno));
		exit (EXIT_FAILURE);
	}
	cam->name = dev_name;
	
	// initialise the device
	struct v4l2_capability cap;

	// check capabilities
	if (-1 == xioctl (cam, VIDIOC_QUERYCAP, &cap)) {
		if (EINVAL == errno) {
			fprintf (stderr, "%s is no V4L2 device\n",
					 cam->name);
			exit (EXIT_FAILURE);
		} else {
				errno_exit ("VIDIOC_QUERYCAP");
		}
	}

	// check capture capable
	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		fprintf (stderr, "%s is no video capture device\n",
					 cam->name);
		exit (EXIT_FAILURE);
	}

	// check for memory mapped io
	if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
		fprintf (stderr, "%s does not support streaming i/o\n",
			 cam->name);
		exit (EXIT_FAILURE);
	}

	// Set the Camera's format
	camSetFormat(cam, width, height, format);

	return cam;
}
