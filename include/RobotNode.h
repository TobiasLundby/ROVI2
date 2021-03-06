#ifndef ROBOT_NODE_H
#define ROBOT_NODE_H

#include "ros/ros.h"
#include <rw/models/WorkCell.hpp>
#include <rw/kinematics/State.hpp>
#include <rw/models/Device.hpp>
#include <rw/rw.hpp>
#include <rwhw/universalrobots/URCallBackInterface.hpp>
#include <rwhw/universalrobots/UniversalRobotsRTLogging.hpp>
#include <rwhw/universalrobots/UniversalRobotsData.hpp>
#include <rovi2/Q.h>
#include <rovi2/MovePtp.h>
#include <rovi2/State.h>
#include <rovi2/xyz.h>
#include <rovi2/Movexyz.h>
#include <boost/thread/thread.hpp>
#include <rw/math/Q.hpp>
#include <rw/math/RPY.hpp>
#include <rw/math/Vector3D.hpp>
#include <rw/math/Transform3D.hpp>
#include <rw/invkin/JacobianIKSolver.hpp>
#include <rw/kinematics/Frame.hpp>


#define ROBOT_ip "192.168.100.4"



class RobotNode_ros
{
public:

	RobotNode_ros(ros::NodeHandle h); 

	virtual ~RobotNode_ros();
 
	bool Move_nonlinear_ptp(rovi2::MovePtp::Request & request, rovi2::MovePtp::Response &res);
	bool Move_servo_ptp(rovi2::MovePtp::Request &request, rovi2::MovePtp::Response &res);
	bool Move_xyz(rovi2::Movexyz::Request &request, rovi2::Movexyz::Response &res);

	void StatePublisher();

	


	rw::math::Q toRw(const rovi2::Q& q);
	rovi2::Q toRos(const rw::math::Q& q);




protected:
	  void initWorkCell();
	  void initDevice();
	  rw::math::Q getQ();





protected:

	ros::NodeHandle _nodehandle;
	ros::ServiceServer service_nonlinear;
	ros::ServiceServer service_servo;
	ros::ServiceServer service_inverse;
  	rw::models::WorkCell::Ptr _workcell;
	rw::kinematics::State _state;
  	rw::models::Device::Ptr _device;
	rwhw::URCallBackInterface *_ur;
	rwhw::UniversalRobotsRTLogging *_urrt;
	rw::invkin::JacobianIKSolver *_solver; 
	rw::kinematics::Frame *_BallErrorFrame;
	boost::thread publisher;
	ros::Publisher state_updater;
	rw::math::Q qCurrent;
	float speed = 0.1;
	float blend = 0.1;

};

#endif ROBOT_NODE
