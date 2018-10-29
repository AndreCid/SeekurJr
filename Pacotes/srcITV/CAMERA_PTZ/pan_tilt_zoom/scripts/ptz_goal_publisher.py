#!/usr/bin/python
import rospy
import roslib
import time
import math

#imports of messages

from pan_tilt_zoom.msg import pan_tilt_goal
from sensor_msgs.msg import JointState

# Create the Class that will publish on the camera's topic
class ControlePTZ():

	# Class-creating method
	def __init__(self):
		
		# Declaration of variables
		self.X = 0.0
		self.Y = 0.0
		self.Z = 0.0
#		self.P = 0.0
#		self.T = 0.0
		self.Pan = 0.0
		self.Tilt = 0.0
		self.pub = rospy.Publisher('/RosAria/ptz_goal', pan_tilt_goal)
		rospy.Subscriber('/end_effector_pose', JointState, self.coordCallback)
		#rospy.spin()


		# Command iniciate 
		command=pan_tilt_goal()
		while not rospy.is_shutdown():

		# Assignment of values to the ptz_goal topic

			#	self.X = 0.0
			#	self.Y = 0.0
			#	self.Z = 808.0
			#	self.P = (self.Y)/(self.X+660.0)
			#	self.T = (self.Z - 170.0) / (self.X + 660.0)
				self.Pan = math.atan((self.Y) / (self.X + 60.0))
				self.Tilt = math.atan((self.Z - 170.0) / (self.X + 60.0))
				self.Pan = (180 * self.Pan) / (math.pi)
				self.Tilt = (180 * self.Tilt) / (math.pi)
				command.pan=self.Pan
				command.tilt=self.Tilt
				self.pub.publish(command)
				print ("X = %f"%self.X)
				print ("Y = %f"%self.Y)
				print ("Z = %f"%self.Z)
				print " "
				time.sleep(0.5)

	
	# Callback funtion who receive the Phantom coordenates.
	def coordCallback(self,data):  		
		self.X = data.position[2] + 88.114196777343746	# The Phantom Z is the PTZ X
		self.Y = data.position[0]	# The Phantom X is the PTZ Y
		self.Z = data.position[1] + 65.51071166992187	# The Phantom Y is The PTZ Z

# Main Function#
if __name__ == '__main__':

	# Create the node with "ptz_goal_publisher"
	rospy.init_node('ptz_goal_publisher', anonymous=True)

	# Create a object with the "ControlePTZ" class
	try:
		obj_no = ControlePTZ()
	except rospy.ROSInterruptException: pass
