#ifndef ARMODES_H
#define ARMODES_H
#endif

#include <ariaTypedefs.h>
#include <ArMode.h>
#include <ArActionGroups.h>
#include <ArTcpConnection.h>
#include <ArSerialConnection.h>
#include <ArPTZ.h>
#include <ArTCMCompassRobot.h>
#include <ArRobotConfigPacketReader.h>
#include <ros/ros.h>
#include "pan_tilt_zoom/pan_tilt_goal.h"

/// Mode for controlling the camera
class ArCameraMode : public ArMode
{
public:
  /// Constructor
  AREXPORT ArCameraMode(ArRobot *robot, const char *name, char key,
			 char key2, ros::NodeHandle *node);
  /// Destructor
  AREXPORT virtual ~ArCameraMode();
  AREXPORT virtual void activate(void);
  AREXPORT virtual void deactivate(void);
  AREXPORT virtual void userTask(void);
  AREXPORT virtual void help(void);
  AREXPORT void up(void);
  AREXPORT void down(void);
  AREXPORT void left(void);
  AREXPORT void right(void);
  AREXPORT void center(void);
  AREXPORT void zoomIn(void);
  AREXPORT void zoomOut(void);
  AREXPORT void exercise(void);
  AREXPORT void toggleAutoFocus();
  AREXPORT void rvisionSerial(void);
  
  AREXPORT void com4(void);
  
  AREXPORT void reachGoal(void);
  AREXPORT void ptzGoalCallback(const pan_tilt_zoom::pan_tilt_goal::ConstPtr& pan_tilt_goal);
  
  void setGoalPan(int pan) { goalPan = pan;}
  int getGoalPan(void){ return goalPan;}
  void setGoalTilt(int tilt){ goalTilt = tilt;}
  int getGoalTilt(void){ return goalTilt;}
  
protected:
  void giveUpCameraKeys(void);
  void takePortKeys(void);
  void giveUpPortKeys(void);
  void helpPortKeys(void);
  void takeMovementKeys(void);
  void giveUpMovementKeys(void);
  void helpMovementKeys(void);
  
  enum State {
    STATE_CAMERA,
    STATE_PORT,
    STATE_MOVEMENT
  };
  void cameraToPort(void);
  void portToMovement(void);

  enum ExerState {
    CENTER,
    UP_LEFT,
    UP_RIGHT,
    DOWN_RIGHT,
    DOWN_LEFT
  };

  bool myExercising;
  State myState;
  ExerState myExerState;
  ArTime myLastExer;
  bool myExerZoomedIn;
  ArTime myLastExerZoomed;
  ArSerialConnection myConn;
  ArPTZ *myCam;
  ArFunctorC<ArCameraMode> myUpCB;
  ArFunctorC<ArCameraMode> myDownCB;
  ArFunctorC<ArCameraMode> myLeftCB;
  ArFunctorC<ArCameraMode> myRightCB;
  ArFunctorC<ArCameraMode> myCenterCB;
  ArFunctorC<ArCameraMode> myZoomInCB;
  ArFunctorC<ArCameraMode> myZoomOutCB;
  ArFunctorC<ArCameraMode> myExerciseCB;
  ArFunctorC<ArCameraMode> myRVisionSerialCB;
  ArFunctorC<ArCameraMode> myCom4CB;
  const int myPanAmount;
  const int myTiltAmount;
  bool myAutoFocusOn;
  ArFunctorC<ArCameraMode> myToggleAutoFocusCB;
  
  double goalPan, goalTilt;
};
