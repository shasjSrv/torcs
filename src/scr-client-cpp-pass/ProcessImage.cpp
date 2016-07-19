#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "opencv/cvaux.h"
#include "opencv/cxmisc.h"

#include <stdio.h>

IplImage* captureCV=NULL;

void createImage(int width, int height)
{
	if(captureCV!=NULL)
		cvReleaseImage(&captureCV);

	//creat image
	captureCV = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
}

void releaseImage()
{
  	// release the image
  	if(captureCV!=NULL)
  		cvReleaseImage(&captureCV);
}

void processImage(const char camID, const unsigned char *imgBuf, int width, int height)
{
	if(captureCV!=NULL){
		for (int i = (height-1), j=0; i >=0 ; i--,j++) {
			memcpy((unsigned char*)(captureCV->imageData + i * captureCV->widthStep), imgBuf + j * width * 3, width * 3);
		}

		//save image to disk
		char imgFileName[20];
		sprintf(imgFileName,"/tmp/receive%c.png",camID);
		//cvSaveImage(imgFileName,captureCV);
	
//The channels of the colors that we use. In this case are 3, our
// data layout of a color image is: b0, g0, r0, b1, g1, r1 ... We have
// to realize that is different in OpenCV is BGR and is not RGB as we
// are get used.

		//process image
		//...
	}
}
