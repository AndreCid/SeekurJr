/****************************************************************************
 * Copyright (c) 2009 by Focus Robotics. All rights reserved.
 *
 * This program is an unpublished work fully protected by the United States 
 * copyright laws and is considered a trade secret belonging to the copyright
 * holder. No part of this design may be reproduced stored in a retrieval 
 * system, or transmitted, in any form or by any means, electronic, 
 * mechanical, photocopying, recording, or otherwise, without prior written 
 * permission of Focus Robotics, Inc.
 *
 * Proprietary and Confidential
 *
 * Created By   :  Andrew Worcester
 * Creation_Date:  Mon Feb 23 2009
 * 
 * Brief Description:
 * 
 * Functionality:
 * 
 * Issues:
 * 
 * Limitations:
 * 
 * Testing:
 * 
 ******************************************************************************/
#include "frcam.h"
#define USING_OPENCV
#include "FrEagleStereoSmartCam.h"

struct FrChannel {
  int didx; // device index 0-3 for which eagle device in the system
  int cidx; // channel index 0-3 for which channel within that device
  FrEagleStereoSmartCam *ssc; // pointer to the stereo smart cam obj for that device
};

struct FrCamera {
  FrEagleStereoSmartCam *ssc; // the obj for this camera device
  int grabbing;
  FrChannel* chans[4];
};

static FrCamera *cams = 0;

/**
 * If this is the first channel being opened for this device, create a new ssc obj
 * If this device already has other channels opened, just reference the already
 * opened ssc object. 
 *
 * How do I know what has already been opened? There must be a global array of
 * struct FrCamera which is initialized to zero. It also seems like the channel
 * should have a pointer to it's camera and the camera should have pointers to 
 * it's channels. It that necessary or useful?
 *
 * Maybe I can have just one global--a pointer to an array of Cameras which in
 * turn contain pointer to each open channel. The pointer is initialized to zero
 * and the first open malloc's and initialized the whole structure.
 */
FrChannel* frOpenChannel(int index, int device) {
  // Check to see if the global cams array has been initialized
  if(cams == 0) {
    // cams not yet initialized, must alloc memory for the structure and initialize it
    cams = (FrCamera*)calloc(4, sizeof(FrCamera));
  }

  // Check to see if the requested device has been opened
  if(cams[device].ssc == 0) {
    cams[device].ssc = new FrEagleStereoSmartCam(device);
#ifdef DEVSYS_MODE
    cams[device].ssc->setDevsysMode(DEVSYS_MODE);
#endif
    cams[device].grabbing = 0;
  }

  // Check to see if the requested channel has been initialized
  if(cams[device].chans[index] == 0) {
    FrChannel *chan = (FrChannel*)calloc(1, sizeof(FrChannel));
    cams[device].chans[index] = chan;
    chan->didx = device;
    chan->cidx = index;
    chan->ssc = cams[device].ssc;
  }

  return cams[device].chans[index];
}

/**
 *
 */
int frSetChannelProperty(FrChannel* channel, int property_id, int value) {
  // big switch statement -- returns an error for invalid values
  switch(property_id) {
  case FR_CHAN_PROP_FRAME_WIDTH:
    channel->ssc->setFrameWidth(value); // 1-1023 is valid in theory, but will currently error for other than 752
    break;
  case FR_CHAN_PROP_FRAME_HEIGHT:
    channel->ssc->setFrameHeight(value); // 1-1023 is valid in theory, but will error for other than 480
    break;
  case FR_CHAN_PROP_FOURCC: // will currently error for other than NDEPTH_PIX_FMT_GREY (=0) defined in fr3 driver
    break;
  case FR_CHAN_PROP_CHIP_VERSION: // ignores attempts to set
    break;
  case FR_CHAN_PROP_GPIO: // danger! this can royally screw up the chip; must add to ssc
    break;
  case FR_CHAN_PROP_AEGC_EN: // probably becomes aegc_mode now
    break;
  case FR_CHAN_PROP_AEGC_SKIP:
    break;
  case FR_CHAN_PROP_DESIRED_BIN: // between 0 and 64
    channel->ssc->setCamBrightness(value);
    break;
  case FR_CHAN_PROP_EXPOSURE: // clamped between 1 and 480 (but does it change for other frame heights?
    // do I try to support setting separate exposure and gain for left and right image sensors?
    // I probably do need to get separate actual bin numbers, so there will already be some of that!
    break;
  case FR_CHAN_PROP_GAIN: // clamped between 16 and 64
    break;
  case FR_CHAN_PROP_MATCH_QUAL: // what values should I provide here? Int values like I send from the track bar?
    channel->ssc->setMatchQualThresh(value);
    break;
  case FR_CHAN_PROP_MAX_SEARCH: // not working yet; real soon
    break;
  case FR_CHAN_PROP_MIN_SEARCH: // not working yet; real soon
    break;
  };
  return 0;
}

/**
 *
 */
int frGetChannelProperty(FrChannel* channel, int property_id) {
  // bit switch statement -- returns property value
  switch(property_id) {
  case FR_CHAN_PROP_FRAME_WIDTH:
    return channel->ssc->getFrameWidth();
  case FR_CHAN_PROP_FRAME_HEIGHT:
    return channel->ssc->getFrameHeight();
  case FR_CHAN_PROP_FOURCC:
    return 0;
  case FR_CHAN_PROP_CHIP_VERSION:
    return channel->ssc->getChipVersion();
  case FR_CHAN_PROP_GPIO:
    break;
  case FR_CHAN_PROP_AEGC_EN:
    return channel->ssc->getAegcMode();
  case FR_CHAN_PROP_AEGC_SKIP:
    return channel->ssc->getAegcSkip();
  case FR_CHAN_PROP_DESIRED_BIN:
    return channel->ssc->getDesiredBin(0);
  case FR_CHAN_PROP_EXPOSURE:
    break;
  case FR_CHAN_PROP_GAIN:
    break;
  case FR_CHAN_PROP_MATCH_QUAL:
    return channel->ssc->getMatchQualThresh();
  case FR_CHAN_PROP_MAX_SEARCH:
    return 64;
  case FR_CHAN_PROP_MIN_SEARCH:
    return 0;
  };
  return 0;
}

/**
 * I don't like the notation, this should be setChannelOutput, not input!
 */
int frSetChannelInput(FrChannel* channel, int index) {
  switch(index) {
  case FR_CHAN_INPUT_TYPE_DISP:
    channel->ssc->setChanType(channel->cidx, ITYPE_FINAL_DISP);
    break;
  case FR_CHAN_INPUT_TYPE_CAL_RT:
    channel->ssc->setChanType(channel->cidx, ITYPE_RIGHT_RECT);
    break;
  case FR_CHAN_INPUT_TYPE_CAL_LT:
    channel->ssc->setChanType(channel->cidx, ITYPE_LEFT_RECT);
    break;
  case FR_CHAN_INPUT_TYPE_RT:
    channel->ssc->setChanType(channel->cidx, ITYPE_RIGHT_RAW);
    break;
  case FR_CHAN_INPUT_TYPE_LT:
    channel->ssc->setChanType(channel->cidx, ITYPE_LEFT_RAW);
    break;
  };
  return 0;
}

/**
 *
 */
int frGrabFrame(FrChannel *channel) {
  channel->ssc->grabImg(channel->cidx);
  cams[channel->didx].grabbing = 1;
  return 0;
}

/**
 *
 */
IplImage* frRetrieveFrame(FrChannel *channel) {
  if(cams[channel->didx].grabbing==1) {
    channel->ssc->run();
    cams[channel->didx].grabbing = 0;
  }
  FrImage *img = channel->ssc->retrieveImg(channel->cidx);
  return img->getIplImage(0);
}

/**
 * This would basically just call run, but it brings up a problem: right now the
 * ssc assumes you call run every cycle. I need to support implicit run calls 
 * determined only from calls to grab and retrieve. The first retrieve after any
 * grab implied a call to run, I guess. This won't work just like the v4l driver
 * but will probably be OK for most purposes.
 */
int frStepVcam(FrChannel* channel) {
  return 0;
}

// This function is deprecated and should not be used in new code
// return (cidx | (didx<<4)) to get a unique identifier that can easily be turned
// back into a channel pointer
int frGetChannelDev(FrChannel *channel) {
  return 0;
}

/**
 *
 */
int frProgRemapData(FrChannel *channel, FrCalibMap *map) {
  channel->ssc->setRectData(frGetRemapData(map), frGetRemapDataLen(map));
  return 0;
}

/**
 *
 */
void frCloseChannel(FrChannel *channel) {
  // FIXME: only delete channel->ssc if this is the last channel open for that 
  // device.
  //delete(channel->ssc);
  int device = channel->didx;
  int index = channel->cidx;
  // Verify that this device really appears to be open
  // Check how many channels are open for this device, and that this channel is one of them
  // If no errors, and this channel is the only one open for this device, then
  // delete the device->ssc in addition to the channel

  // In any event (if no errors) delete this channel and set the pointer to it
  // in the device struct to 0;
  cams[device].chans[index] = 0;
  delete(channel);
  return;
}

/**
 *
 */
int frProgRemapCoords(FrChannel *channel, FrCalibMap *map) {
  // need a function in the calib map that returns an int pointer and a length
  return 0;
}

/////////////////////////Stuff in frmap that isn't driver dependant
//FrCalibMap* frOpenCalibMap(const char* filename) {return 0;}
//int frProgramRemapCoords(FrCalibMap *map, int fd) {return 0;}
//int frMakeHWRemapCoords(FrCalibMap *map) {return 0;}
//void frCloseCalibMap(FrCalibMap *map) {return;}
//float frGetCameraBaseline(FrCalibMap *map) {return 0;}
//float frGetCameraFocalLength(FrCalibMap *map) {return 0;}

/**
 *
 */
FrVcam* frOpenVcam(int index) {return 0;}

/**
 *
 */
int frSetVcamProperty(FrVcam* vcam, int property_id, int value) {return 0;}

/**
 *
 */
int frGetVcamProperty(FrVcam* vcam, int property_id) {return 0;}

/**
 * This is kinda weird -- shouldn't setVcamProperty be used for this?
 */
int frSetVcamFmt(FrVcam* vcam, IplImage *frame) {return 0;}

/**
 * I don't like the notation -- this should be setVcamInput, not output
 */
int frSetVcamOutput(FrVcam* vcam, int index) {return 0;}

/**
 *
 */
int frWriteVcamFrame(FrVcam* vcam, IplImage *frame) {return 0;}

/**
 *
 */
void frCloseVcam(FrVcam *vcam) {return;}


int frI2CRead(FrChannel *chan, unsigned int offset, unsigned int devselect) {
  return chan->ssc->getI2cReg(offset, devselect);
}

int frI2CWrite(FrChannel *chan, unsigned int offset, unsigned int value, unsigned int devselect) {
  chan->ssc->setI2cReg(offset, devselect, value);
  return 0;
}

int frI2CWriteBC(FrChannel *chan, unsigned int offset, unsigned int value) {
  chan->ssc->setI2cReg(offset, -1, value);
  return 0;
}






/////////////////////Stuff in frvis that isn't driver dependant
//void  frCreateColorLUT(int maxvalue, uchar *redLut, uchar *greenLut, uchar *blueLut) {return;}
//void frRemapImageColor(IplImage *disp, IplImage *color, char *redLut, char *greenLut, char *blueLut) {return;}

////////////////////Stuff in frstat that isn't driver dependant
//FrStatusTool* frOpenStatusTool(FrChannel* channel, char* name) {return 0;}
//void frCloseStatusTool(FrStatusTool *stat) {return;}
//int frAddI2CMon(FrStatusTool* stat, int offset, char* field_name, char* field_fmt, int field_start, int field_size) {return 0;}
//int frAddExposureMon(FrStatusTool* stat) {return 0;}
//int frAddGainMon(FrStatusTool* stat) {return 0;}
//int frAddBinMon(FrStatusTool* stat) {return 0;}
//int frAddAEGCMon(FrStatusTool* stat) {return 0;}
//int frAddFPSMon(FrStatusTool* stat, int* frame_count) {return 0;}
//int frUpdateMonitors(FrStatusTool* stat) {return 0;}
//void frAddDesiredBinTbar(FrStatusTool* stat) {return;}
//void frAddManualExpTbar(FrStatusTool* stat) {return;}
//void frAddManualGainTbar(FrStatusTool* stat) {return;}
//void frAddMatchQualTbar(FrStatusTool* stat) {return;}
//void frAddI2CKeypress(FrStatusTool* stat) {return;}
//int  frCheckKeypress(FrStatusTool* stat, int keypressed) {return 0;}
