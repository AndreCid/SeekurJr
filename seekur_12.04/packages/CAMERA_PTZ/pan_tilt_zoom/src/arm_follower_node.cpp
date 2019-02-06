#include <ros/ros.h>

#ifdef ADEPT_PKG
  #include <Aria.h>
#else
  #include <Aria/Aria.h>
#endif

#include <ArCommands.h>

#include "pan_tilt_zoom/ArCameraMode.h"

int main(int argc, char **argv){
	ros::init(argc, argv, "pan_tilt_zoom_node"); 

	ros::NodeHandle n; // The main access point to communication with the ROS system.
	ArSerialConnection myConn;
  	ArPTZ *myCam;
  	
	while(ros::ok())
	{
		Aria::init();
		ArArgumentParser parser(&argc, argv);
		parser.loadDefaultArguments();
		ArRobot robot;
		ArRobotConnector robotConnector(&parser, &robot);

		// This is used to create and configure the PTZ interface object based on the
	  	// robot's parameter file and this program's command line options (run with
	  	// -help for list). Must be created before calling Aria::parseArgs().
	  	ArPTZConnector ptzConnector(&parser, &robot);

	  	// Creating PTZ object
	  	//myCam = new ArRVisionPTZ(&robot);
	  	
	  	// Setting up PTZ connection
	  	//myConn.setPort(ArUtil::COM4);
	  	
	
		if(!robotConnector.connectRobot())
		{
		    ArLog::log(ArLog::Terse, "Move Pan-Tilt PTZ Node: Warning, Could not connect to the robot. Won't use robot parameter file for defaults");
		}

		if (!Aria::parseArgs() || !parser.checkHelpAndWarnUnparsed())
		{
		    Aria::logOptions();
		    Aria::exit(1);
			// TODO: ros exit command
		}
	
		ArLog::log(ArLog::Normal, "Move Pan-Tilt PTZ Node: Connected to robot.");
		
		//myCam->init();
  		//robot.setPTZ(myCam);

		// ArRobot contains an exit action for the Escape key. It also 
		// stores a pointer to the keyhandler so that other parts of the program can
		// use the same keyhandler.
		// Used to perform actions when keyboard keys are pressed
  		ArKeyHandler keyHandler;
		robot.attachKeyHandler(&keyHandler);
		printf("You may press escape to exit\n");

		//myCam->panTilt(-180, 43.44);
  		// Start the robot task loop running in a new background thread. The 'true' argument means if it loses
 		// connection the task loop stops and the thread exits.
	  	robot.runAsync(true);
		
		robot.com2Bytes(116,10,1); // Sets PTZ power.

		// now add the modes for this demo
		// only mode camera matters
  		// these classes are defined in ArModes.cpp in ARIA's source code.
		ArCameraMode camera(&robot, "armFollowerMode", 'c', 'C', &n);
		// activate the default mode: mode camera
				
		camera.activate();
		
		// Block execution of the main thread here and wait for the robot's task loop
  		// thread to exit (e.g. by robot disconnecting, escape key pressed, or OS
		// signal)
		//robot.waitForRunExit();
		ros::spin();
  		//Aria::exit(0);
	}
	return 0;

}
