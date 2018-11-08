[![Build Status](https://travis-ci.org/miguelleitao/easimage.svg?branch=master "Build Status")](https://travis-ci.org/miguelleitao/easimage)
[![Documentation](https://codedocs.xyz/miguelleitao/easimage.svg)](https://codedocs.xyz/miguelleitao/easimage/)
# easimage
Simple and light image processing library

### Description
**easimage** is a simple and light image processing library aimed to tutorial environments and/or
small computing devices.

**easimage** was developed in C and organized into 4 main packets:

* camera.c: functions related to camara handling (open, close and image capture).
* image.c:  functions to handle image structures.
* viewer.c: window creation and image display.
* util.c:   other funcionalities that could not be fit elsewhere.

### Building **easimage**

    git clone https://github.com/miguelleitao/easimage.git
    cd easimage
    make
    sudo make install

### Documentation
**easimage** home page is located on: http://miguelleitao.github.io/easimage/  
**easimage** documentation is available from https://codedocs.xyz/miguelleitao/easimage/  
Latest **easimage** source code is maintained at: https://github.com/miguelleitao/easimage/

### Authors
**easimage** is maintained by Miguel Leitao.  
**easimage** includes parts from the Basic Image Processing library (imgproc) available from:
https://www.cl.cam.ac.uk/projects/raspberrypi/tutorials/robot/resources/imgproc.zip
and credited to Olly Andrade, Ed Jones and Alex Lee.

### License
**easimage** is licensed under the terms of the MIT license.
A copy of the MIT lecense can be found at:
http://opensource.org/licenses/MIT

