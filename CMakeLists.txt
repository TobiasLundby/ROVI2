cmake_minimum_required(VERSION 2.8.3)
project(rovi2)
set(SHOULD_BUILD 1)
set(node_name "robot_node")

#include("cmake/use-c++11.cmake")

########################################################################
#### Catkin Packages
########################################################################
find_package(catkin REQUIRED COMPONENTS
  message_generation
  rosconsole
  roscpp
  std_msgs
  cv_bridge
  image_transport
)


########################################################################
#### RobWork
########################################################################
set(RW_ROOT "$ENV{RW_ROOT}")
find_package(RobWork REQUIRED PATHS "${RW_ROOT}")

########################################################################
#### RobWorkHardware
########################################################################
set(RWHW_ROOT "$ENV{RWHW_ROOT}")
find_package(RobWorkHardware COMPONENTS universalrobots PATHS "${RWHW_ROOT}")
if (NOT ROBWORKHARDWARE_FOUND)
  message(WARNING "Requires the universalrobots component of RobWorkHardware!")
  set(SHOULD_BUILD 0)
endif()

########################################################################
#### Build process
########################################################################
if (SHOULD_BUILD)
  ################################################
  ## Declare ROS messages, services and actions ##
  ################################################
  add_service_files(
    FILES
    MovePtp.srv
    )

	add_message_files(
	  FILES
	  Q.msg
	  State.msg
	)

  generate_messages(
    DEPENDENCIES
    std_msgs
    )

  ###################################
  ## catkin specific configuration ##
  ###################################
  catkin_package(
    INCLUDE_DIRS include
    CATKIN_DEPENDS message_runtime roscpp std_msgs
    DEPENDS ROBWORK ROBWORKHARDWARE
    #CFG_EXTRAS "use-c++11.cmake"
    )

  ########################################################################
  #### Build
  ########################################################################
  include_directories(${node_name}
    include
    ${ROBWORK_INCLUDE_DIR}
    ${ROBWORKHARDWARE_INCLUDE_DIRS}
    ${catkin_INCLUDE_DIRS}
    )

  add_executable(${node_name}
    src/RobotNode.cpp
    src/node_main.cpp

    )

  target_link_libraries(${node_name}
    ${ROBWORK_LIBRARIES}
    ${ROBWORKHARDWARE_LIBRARIES}
    ${catkin_LIBRARIES}
    )


  #add_dependencies(
  #  )

  #find_package( OpenCV REQUIRED ) # Required below
  add_executable(image_converter src/image_converter.cpp)
  #target_link_libraries(image_converter ${catkin_LIBRARIES} ${OpenCV_LIBS} )
  target_link_libraries(image_converter ${catkin_LIBRARIES} )
  add_dependencies(image_converter sensor_msgs cv_bridge roscpp std_msgs image_transport opencv)

  #add_executable(offboard_node src/offboard_node.cpp)
  #target_link_libraries(offboard_node ${catkin_LIBRARIES})
  #add_dependencies(offboard_node geometry_msgs mavros_msgs mavros roscpp rospy std_msgs)

catkin_package()

  ########################################################################
  #### Testing
  ########################################################################
else()
  message(WARNING "Skipping ${PROJECT_NAME} due to missing libraries - see earlier warnings.")
endif()