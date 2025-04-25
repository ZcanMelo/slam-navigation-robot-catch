#!/usr/bin/env python

import tf
import math
import smach
import rospy
import actionlib
import geometry_msgs
import move_base_msgs.msg
import tf.transformations
from std_msgs.msg import String

def gen_new_movebase_goal(frame_id,
                          target_x,
                          target_y,
                          next_target_x,
                          next_target_y):
    goal = move_base_msgs.msg.MoveBaseGoal()
    goal.target_pose.header.frame_id = frame_id
    goal.target_pose.header.stamp = rospy.Time.now()
    goal.target_pose.pose.position.x = target_x
    goal.target_pose.pose.position.y = target_y

    # to make our robot face the next target point,
    # when we arrive the target point

    target_yaw = (math.atan2(next_target_y - target_y, next_target_x - target_x) % (math.pi*2))
    target_quaternion = tf.transformations.quaternion_from_euler(0, 0, target_yaw)
    goal.target_pose.pose.orientation.x = target_quaternion[0]
    goal.target_pose.pose.orientation.y = target_quaternion[1]
    goal.target_pose.pose.orientation.z = target_quaternion[2]
    goal.target_pose.pose.orientation.w = target_quaternion[3]
    return goal

def speech_callback(msg):
        #rospy.loginfo(msg)
        global XL_goal
        global XL_finish
        if msg.data == 'patrol':
            XL_goal=1
        elif msg.data == 'stoppatrol':
            XL_goal=0
        elif msg.data == 'shutdown':
            XL_finish=1

class MoveToPoint(smach.State):
    def __init__(self, target_x, target_y, next_target_x, next_target_y):
        smach.State.__init__(self, outcomes=['next_point'])
        self.__target_x = target_x
        self.__target_y = target_y
        self.__next_target_x = next_target_x
        self.__next_target_y = next_target_y

    def execute(self, ud):
        goal = gen_new_movebase_goal('map',
                                     self.__target_x,
                                     self.__target_y,
                                     self.__next_target_x,
                                     self.__next_target_y)   
        if XL_goal==1:
            server_movebase.send_goal(goal)
        # todo: what should robot do when failed to finish the target point?
        is_server_availabel = server_movebase.wait_for_result()
        if is_server_availabel is False:
            return 'error_exit'
        else:
            if XL_finish==1:
               return 'error_exit'
            else:
               return 'next_point'


if __name__ == '__main__':
    global XL_goal
    global XL_finish
    XL_goal=0
    XL_finish=0
    rospy.init_node('smach_serial_demo2',log_level=rospy.DEBUG)
    server_movebase = actionlib.SimpleActionClient('/move_base', move_base_msgs.msg.MoveBaseAction)
    connect_state = server_movebase.wait_for_server()
    points = [[11.3,50.1],  # target point 1  only need x and y
              [14.7,41.9],  # target point 2
              [16.9,18.2],  # target point 3
              [10.4,25.4]]  # target point 4
    if connect_state is not True:
        # please to ensure weather your move_base server is opened?
        rospy.logerr('can not connect to move_base server')
        exit(-1)
    rospy.logdebug('move base connect successfully!')
    rospy.Subscriber('/recognizer/output', String, speech_callback)
    size_points = len(points)
    name_target_point = ""
    name_next_target_point = ""
    smach_serial_demo2 = smach.StateMachine(outcomes=['successful'])
    a=0
    b=0
    with smach_serial_demo2:
        while XL_goal==0:
              b=0
        if XL_goal==1 and a==0:
                    a+1
        # to point in points list into state machine
                    for target_point in range(0, size_points, 1):
                        next_target_point = (target_point+1) % size_points
                        name_target_point = 'POINT%d' % target_point
                        name_next_target_point = 'POINT%d' % next_target_point
                        smach.StateMachine.add(name_target_point,
                                   MoveToPoint(target_x=points[target_point][0],
                                               target_y=points[target_point][1],
                                               next_target_x=points[next_target_point][0],
                                               next_target_y=points[next_target_point][1]),
                                   transitions={'next_point': name_next_target_point}) 
        smach_serial_demo2.execute()

    #rospy.loginfo("HandsFree smach_serial_demo 2 finish!")



