///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005-2009 Focus Robotics. All rights reserved. 
//
// Created by    :  Stefan Assmus
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the 
// Free Software Foundation; either version 2 of the License, or (at your 
// option) any later version.
//
// This program is distributed in the hope that it will be useful, but 
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
// General Public License for more details.
//
///////////////////////////////////////////////////////////////////////////
#include "cv.h"
#include "highgui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <time.h>
#include <ctype.h>

#include "frcam.h"

// point for distance info (using mouse)
CvPoint pt ;


// define mouse callback for pixel highlighting
void on_mouse( int event, int x, int y, int flags, void* param )
{
  switch ( event )
  {
  case CV_EVENT_LBUTTONDOWN:
    {
      pt = cvPoint(x, y);
    }

    break;
  }
}

int main( int argc, char** argv )
{
	FrChannel *channel[3];
	IplImage *frame[3];
	char wndname[3][50];

	int i;

	/* get unconfigured ndepth video channels, device supports up to 4 */
	for(i = 0; i < 2; i++) {
	  channel[i] = frOpenChannel(i,0);
		if (!channel[i]) {
			printf("demo: frOpen failed for device%d\n",i);
			exit(1);			
		}
	}
	

	/* configure the channels as disparity and rt calibrated */
	if(frSetChannelInput(channel[0], FR_CHAN_INPUT_TYPE_DISP)) {
		printf("demo: set INPUT_TYPE failed for device 0\n");
		exit(1);
	}
	if(frSetChannelInput(channel[1], FR_CHAN_INPUT_TYPE_CAL_RT)) {
		printf("demo: set INPUT_TYPE failed for device 1\n");
		exit(1);
	}
	

	/* program calibration remap coords */
	FrCalibMap *map = frOpenCalibMap("/etc/fr/default.calib");
	if(frProgRemapData(channel[0], map)) {
		fprintf(stderr, "RemapCoords failed\n");
		exit(1);
	}
	float focalLength = frGetCameraFocalLength(map);
	float baseline = frGetCameraBaseline(map);
	frCloseCalibMap(map);


	/* create display windows for each channel */
	sprintf(wndname[0], "Disparity");	
	sprintf(wndname[1], "Right Calib");
	cvNamedWindow(wndname[0], CV_WINDOW_AUTOSIZE);
	cvMoveWindow(wndname[0], 0, 0);
	cvNamedWindow(wndname[1], CV_WINDOW_AUTOSIZE);
	cvMoveWindow(wndname[1], 0, 520);     

	/* used for color image */
	IplImage *disp = cvCreateImage(cvSize(752, 480), IPL_DEPTH_8U, 3); // FIXME: don't hardcode image size
	const int maxDisparityValue = 63;	
	char *redLut = (char*)malloc(maxDisparityValue);
	char *greenLut = (char*)malloc(maxDisparityValue);
	char *blueLut = (char*)malloc(maxDisparityValue);
	frCreateColorLUT(maxDisparityValue, (uchar*)redLut, (uchar*)greenLut, (uchar*)blueLut);

	/* add status/control interfaces */
	int keypressed;
	int frameCount = 0;

	FrStatusTool *stat = frOpenStatusTool(channel[0], "Status");

	frAddBinMon(stat);
	frAddExposureMon(stat);
	frAddGainMon(stat);

	frAddDesiredBinTbar(stat);
	frAddManualExpTbar(stat);
	frAddManualGainTbar(stat);
	frAddMatchQualTbar(stat);
	frAddAEGCMon(stat);
	
	frAddFPSMon(stat, &frameCount);

	frAddI2CKeypress(stat);
	

	/* used to show distance of pixel selected by the mouse */
	const int r = 4;
	char dispText[1024];
	float distance;
	int   disparity;
	float numerator = focalLength * baseline;

	CvFont dfontMouse;
	CvScalar color = CV_RGB(0, 0, 0);
	CvScalar color2 = CV_RGB(255, 0, 0);
	int line_type = 8;
	float hscaleMouse = .5f;
	float vscaleMouse = .5f;
	int thicknessMouse =2;

	cvInitFont (&dfontMouse, CV_FONT_VECTOR0, hscaleMouse, vscaleMouse, 0.0f, thicknessMouse, line_type);

	// set MouseCallBack
	cvSetMouseCallback(wndname[1], on_mouse,0);
	cvSetMouseCallback(wndname[2], on_mouse,0);


	// frGrabFrame will start the DMA for each channel 
	for (i = 0; i < 2; i++) {
		if(frGrabFrame(channel[i]))
			goto out;
	}		


	for(;;)	{	
		// frRetreiveFrame will map the DMA buffer to userspace 
		for (i = 0; i < 2; i++) {

			frame[i] = frRetrieveFrame(channel[i]);
			if (NULL == frame) 
				goto out;

			/* add distance-info (mouse-selection) */
			//create distance-text
			cvCircle( frame[i], pt, r, color2, 1, CV_AA, 0);        
			disparity = ((uchar*)(frame[0]->imageData + frame[0]->widthStep*pt.y))[pt.x];
			distance = numerator / disparity;
			sprintf( dispText, "(%d,%d)=%4f m (disp %d)", pt.x, pt.y, distance, disparity);

		}

		// frGrabFrame will start the DMA for each channel 
		for (i = 0; i < 2; i++) {
			if(frGrabFrame(channel[i]))
				goto out;
		}		

		// create a colorized disparity image
		frRemapImageColor(frame[0], disp, redLut, greenLut, blueLut);

		//add distance text to colored image
		cvCircle( disp, pt, r, color2, 1, CV_AA, 0);   
		cvPutText(disp, dispText, cvPoint(pt.x+5 ,pt.y+15), &dfontMouse, color2);

		// display the colorized imag
		cvPutText(frame[1], dispText, cvPoint(pt.x+5 ,pt.y+15), &dfontMouse, color);	
		cvShowImage(wndname[0], disp);

		// display the calibrated image
		cvShowImage(wndname[1], frame[1]);
		
		frameCount++;

		// update monitors every 100 frames
		if(frameCount%10==0) {
			frUpdateMonitors(stat);
		}

		/* check pressed key, if any */
		if(-1 == frCheckKeypress(stat, cvWaitKey(2)))
			break;

	}


out:

	/* return ndepth video channels */
	for(i = 0; i < 2; i++) {
		frCloseChannel(channel[i]);
	}

}





