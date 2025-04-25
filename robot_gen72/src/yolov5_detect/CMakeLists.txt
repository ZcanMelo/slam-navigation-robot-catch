cmake_minimum_required(VERSION 3.0.2)
project(yolov5_detect)

## Compile as C++11, supported in ROS Kinetic and newer
# add_compile_options(-std=c++11)

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package(catkin REQUIRED COMPONENTS
  cv_bridge
  roscpp
  rospy
  sensor_msgs
  std_msgs
  message_generation
)

## Generate messages in the 'msg' folder
add_message_files(
  FILES
  DetectedObject.msg
  CommandWithPosition.msg
)

## Generate added messages and services with any dependencies listed here
generate_messages(
  DEPENDENCIES
  std_msgs
)

## catkin specific configuration
catkin_package(
  CATKIN_DEPENDS cv_bridge roscpp rospy sensor_msgs std_msgs message_runtime
)

## Specify additional locations of header files
include_directories(
  ${catkin_INCLUDE_DIRS}
)

## Install Python script
catkin_install_python(PROGRAMS
  scripts/rstest.py
  DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

