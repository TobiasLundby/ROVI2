/* INCLUDES */
// ROS
#include "ros/ros.h"
#include "rovi2/position2D.h"
#include "rovi2/position3D.h"
#include "rovi2/Q.h"
#include "rovi2/State.h"
#include "std_msgs/Bool.h"

// Image
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>

// OPENCV
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <string>

// Subsriber syncrhonizer
#include <message_filters/subscriber.h>
#include <message_filters/synchronizer.h>
#include <message_filters/sync_policies/approximate_time.h>

// Stream
#include <sstream>

// Custom classes
#include "ColorDetector.hpp"

/* NAMESPACES */

/* DEFINES */
#define SHOW_INPUT_STREAM true
#define OUTPUT_TRIANGULATED_POINT false

 int image_number = 0;
 std::vector<double> robotState_vec;
 bool next_image = false;
//
void robotState_callback(const rovi2::State& msg)
{
  ROS_ERROR("robotState_vec size when cleared: %i", robotState_vec.size());
  robotState_vec.clear();
  //ROS_ERROR("robotState");
  {
    for (std::size_t i = 0; i < msg.q.data.size(); ++i)
    {
      robotState_vec.push_back((double)msg.q.data[i]);
    }
  }
}
//
void save_callback (const std_msgs::Bool &msg)
{
  //image_number++;
  std::ofstream ofs("/home/mathias/Desktop/calib/robotState.txt", std::ofstream::app);
  //ofs << "test" << std::endl;
  ROS_ERROR("robotState_vec size when saved: %i", robotState_vec.size());
  ofs << (image_number-1) << '\t';
  for(int i = 0; i < robotState_vec.size(); i++)
       ofs << robotState_vec[i] << '\t';
  ofs << std::endl;
  ofs.close();
}

std::vector<float> calculate_3D_point(double left_x, double left_y, double right_x, double right_y)
{
    Mat left_point(1, 1, CV_64FC2);
    Mat right_point(1, 1, CV_64FC2);
    Mat point_3D(1, 1, CV_64FC4);
    left_point.at<Vec2d>(0)[0] = left_x;
    left_point.at<Vec2d>(0)[1] = left_y;
    right_point.at<Vec2d>(0)[0] = right_x;
    right_point.at<Vec2d>(0)[1] = right_y;

    // Mat proj_l_ros = Mat::zeros(3,4,CV_64F);
    // proj_l_ros.at<double>(0,0) = 1337.227804;
    // proj_l_ros.at<double>(0,2) = 533.141155;
    // proj_l_ros.at<double>(1,1) =  1337.227804;
    // proj_l_ros.at<double>(1,2) = 467.337757;
    // proj_l_ros.at<double>(2,2) = 1.0;
    //
    // Mat proj_r_ros = Mat::zeros(3,4,CV_64F);
    // proj_r_ros.at<double>(0,0) = 1337.227804;
    // proj_r_ros.at<double>(0,2) = 533.141155;
    // proj_r_ros.at<double>(0,3) = -158.387082;
    // proj_r_ros.at<double>(1,1) =  1337.227804;
    // proj_r_ros.at<double>(1,2) = 467.337757;
    // proj_r_ros.at<double>(2,2) = 1.0;

    Mat proj_l_ros = Mat::zeros(3,4,CV_64F);
    proj_l_ros.at<double>(0,0) = 1333.329856;
    proj_l_ros.at<double>(0,2) = 537.019508;
    proj_l_ros.at<double>(1,1) =  1333.329856;
    proj_l_ros.at<double>(1,2) = 375.7627379;
    proj_l_ros.at<double>(2,2) = 1.0;

    Mat proj_r_ros = Mat::zeros(3,4,CV_64F);
    proj_r_ros.at<double>(0,0) = 1333.329856;
    proj_r_ros.at<double>(0,2) = 537.019508;
    proj_r_ros.at<double>(0,3) = -159.033917;
    proj_r_ros.at<double>(1,1) =  1333.329856;
    proj_r_ros.at<double>(1,2) = 375.762737;
    proj_r_ros.at<double>(2,2) = 1.0;

    triangulatePoints(proj_l_ros,proj_r_ros,left_point,right_point,point_3D);
    std::vector<float> result_vec = {(float)(point_3D.at<Vec2d>(0)[0]/point_3D.at<Vec2d>(0)[3]), // THE RESULT IS LENGTH 4!!!!!!
                                   (float)(point_3D.at<Vec2d>(0)[1]/point_3D.at<Vec2d>(0)[3]),
                                   (float)(point_3D.at<Vec2d>(0)[2]/point_3D.at<Vec2d>(0)[3])};

    if (OUTPUT_TRIANGULATED_POINT) {
        std::stringstream buffer;
        //buffer << std::endl << "3D point:" << std::endl << (point_3D / point_3D.at<double>(3, 0)) << std::endl;
        //ROS_ERROR("%s", buffer.str().c_str());
        buffer << std::endl << "3D point:" << std::endl << result_vec.at(0) << std::endl << result_vec.at(1) << std::endl << result_vec.at(2) << std::endl;
        ROS_ERROR("%s", buffer.str().c_str());
    }/* code */
    return result_vec;
}

void callback(
    const sensor_msgs::ImageConstPtr& image_left,
    const sensor_msgs::ImageConstPtr& image_right,
    image_transport::Publisher& image_pub_left_ptr,
    image_transport::Publisher& image_pub_right_ptr,
    ros::Publisher& pos_pub_left_ptr,
    ros::Publisher& pos_pub_right_ptr,
    ros::Publisher& pos_pub_triangulated_ptr,
    ColorDetector& deterctor_left_obj_ptr,
    ColorDetector& deterctor_right_obj_ptr
)
{
    cv_bridge::CvImagePtr cv_left_ptr, cv_right_ptr;
    try
    {
        cv_left_ptr = cv_bridge::toCvCopy(image_left, sensor_msgs::image_encodings::BGR8);
        cv_right_ptr = cv_bridge::toCvCopy(image_right, sensor_msgs::image_encodings::BGR8);
    }
    catch (cv_bridge::Exception& e)
    {
        ROS_ERROR("cv_bridge exception: %s", e.what());
        return;
    }

    // Update GUI Window
    if (SHOW_INPUT_STREAM) {
        cv::imshow("Left input", cv_left_ptr->image);
        cv::imshow("Right input", cv_right_ptr->image);
    }
    cv::waitKey(3);

    // Output modified video stream
    image_pub_left_ptr.publish(cv_left_ptr->toImageMsg());
    image_pub_right_ptr.publish(cv_right_ptr->toImageMsg());

    // Find the left ball
    std::vector<Point2f> position_left;
    position_left = deterctor_left_obj_ptr.FindMarker(cv_left_ptr->image);
    ROS_INFO("Found [%ld] ball", (long int)position_left.size());
    if(position_left.size() > 0)
    {
        rovi2::position2D msg;
        msg.x = (float)position_left.at(0).x;
        msg.y = (float)position_left.at(0).y;
        pos_pub_left_ptr.publish(msg); // Publish it
    }

    // Find the right ball
    std::vector<Point2f> position_right;
    position_right = deterctor_right_obj_ptr.FindMarker(cv_right_ptr->image);
    ROS_INFO("Found [%ld] ball", (long int)position_right.size());
    if(position_right.size() > 0)
    {
        rovi2::position2D msg;
        msg.x = (float)position_right.at(0).x;
        msg.y = (float)position_right.at(0).y;
        pos_pub_right_ptr.publish(msg); // Publish it
    }

    // Triangulation here
    if(position_right.size() > 0 and position_left.size() > 0 ) {
        std::vector<float> triangluated_point;
        triangluated_point = calculate_3D_point((double)position_left.at(0).x, (double)position_left.at(0).y, (double)position_right.at(0).x, (double)position_right.at(0).y);
        if(triangluated_point.size() > 0)
        {
            rovi2::position3D msg;
            msg.x = (float)triangluated_point.at(0);
            msg.y = (float)triangluated_point.at(1);
            msg.z = (float)triangluated_point.at(2);
            pos_pub_triangulated_ptr.publish(msg); // Publish it
        }
    }

    // Save for eye to hand calibration
    if(true)// && (image_number % 15) == 0)
    {

        std::string calib_path = "/home/mathias/Desktop/image_log";
        std::ostringstream name_left;
        name_left << calib_path << "/image_left/" << std::to_string(image_number) << ".jpg";
        std::ostringstream name_right;
        name_right << calib_path << "/image_right/" << std::to_string(image_number) << ".jpg";
        cv::imwrite(name_left.str(),cv_left_ptr->image);
        cv::imwrite(name_right.str(),cv_right_ptr->image);
    }
    image_number++;

}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "ball_locator_3d");

    ros::NodeHandle nh_; // Create node handler

    // Subscribe to input video feed
    std::string node_name;
    node_name = ros::this_node::getName();
    std::string param_name_left, param_name_right;
    param_name_left = node_name + "/camera_sub_left";
    param_name_right = node_name + "/camera_sub_right";

    std::string parameter_left;
    if (nh_.getParam(param_name_left, parameter_left))
    {
        ROS_INFO("Got param: %s", parameter_left.c_str());
    }
    else
    {
        ROS_ERROR("Failed to get parameters");
    }
    std::string parameter_right;
    if (nh_.getParam(param_name_right, parameter_right))
    {
        ROS_INFO("Got param: %s", parameter_right.c_str());
    }
    else
    {
        ROS_ERROR("Failed to get parameters");
    }

    std::string input_topic_cam_left = parameter_left;
    std::string input_topic_cam_right = parameter_right;

    ROS_INFO("Left  image topic: %s", input_topic_cam_left.c_str());
    ROS_INFO("Right image topic: %s", input_topic_cam_right.c_str());

    message_filters::Subscriber<sensor_msgs::Image> image_left(nh_, input_topic_cam_left, 1);
    message_filters::Subscriber<sensor_msgs::Image> image_right(nh_, input_topic_cam_right, 1);

    typedef message_filters::sync_policies::ApproximateTime<sensor_msgs::Image, sensor_msgs::Image> MySyncPolicy;
    // ApproximateTiros::Publisher position_pub_left;me takes a queue size as its constructor argument, hence MySyncPolicy(10)
    message_filters::Synchronizer<MySyncPolicy> sync(MySyncPolicy(10), image_left, image_right);

    // Image conversion
    std::string output_topic_cam_left = node_name + "/bb_cam_opencv_left";
    std::string output_topic_cam_right = node_name + "/bb_cam_opencv_right";
    ROS_INFO("Left  OpenCV image topic (output): %s", output_topic_cam_left.c_str());
    ROS_INFO("Right OpenCV image topic (output): %s", output_topic_cam_right.c_str());
    image_transport::ImageTransport it_(nh_);
    image_transport::Publisher image_pub_left = it_.advertise(output_topic_cam_left, 1);
    image_transport::Publisher image_pub_right = it_.advertise(output_topic_cam_right, 1);

    // Position publisher
    std::string output_topic_position_left = node_name + "/pos_left";
    std::string output_topic_position_right = node_name + "/pos_right";
    std::string output_topic_position_triangulated = node_name + "/pos_triangulated";
    ros::Publisher position_pub_left = nh_.advertise<rovi2::position2D>(output_topic_position_left,1000);
    ros::Publisher position_pub_right = nh_.advertise<rovi2::position2D>(output_topic_position_right,1000);
    ros::Publisher position_pub_triangulated = nh_.advertise<rovi2::position3D>(output_topic_position_triangulated,1000);

    // RobotState subscriber
    ros::Subscriber robotState_sub = nh_.subscribe("/rovi2/robot_node/Robot_state",1,robotState_callback);
    ros::Subscriber save_sub = nh_.subscribe("/save",1,save_callback);

    // Ball detector objects; two have been made if different settings are required later
    ColorDetector detector_left;
    detector_left.set_result_window_name("Left result 2D");
    ColorDetector detector_right;
    detector_right.set_result_window_name("Right result 2D");

    sync.registerCallback(boost::bind(&callback, _1, _2, image_pub_left, image_pub_right, position_pub_left, position_pub_right, position_pub_triangulated, detector_left, detector_right));

    while( ros::ok() ){
        ros::spin();
    }

    return EXIT_SUCCESS;
}
