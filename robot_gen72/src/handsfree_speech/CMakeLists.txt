cmake_minimum_required(VERSION 2.8.3)
project(handsfree_speech)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  rospy
  std_msgs
  roslib
)
catkin_package(DEPENDS)

catkin_install_python(PROGRAMS
  script/robot/send.py
  DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

include_directories(
# include
  ${catkin_INCLUDE_DIRS}
)
add_executable(handsfree_offlinelistener src/Voice_recognition/asr_offline_record_sample.cpp src/linuxrec.c src/speech_recognizer.c)
if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "aarch64")
    target_link_libraries(handsfree_offlinelistener ${catkin_LIBRARIES} ${PROJECT_SOURCE_DIR}/lib/arm64/libmsc.so -lmsc -lrt -ldl -lpthread -lasound -lstdc++
 )
else()
    target_link_libraries(handsfree_offlinelistener ${catkin_LIBRARIES} ${PROJECT_SOURCE_DIR}/lib/x64/libmsc.so -lmsc -lrt -ldl -lpthread -lasound -lstdc++
 )
endif()
add_executable(handsfree_offlinetalker src/Voice_feedback/tts_offline_sample.cpp)
if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "aarch64")
    target_link_libraries(handsfree_offlinetalker ${catkin_LIBRARIES} ${PROJECT_SOURCE_DIR}/lib/arm64/libmsc.so -lmsc -lrt -ldl -lpthread -lasound -lstdc++
 )
else()
    target_link_libraries(handsfree_offlinetalker ${catkin_LIBRARIES} ${PROJECT_SOURCE_DIR}/lib/x64/libmsc.so -lmsc -lrt -ldl -lpthread -lasound -lstdc++
 )
endif()

