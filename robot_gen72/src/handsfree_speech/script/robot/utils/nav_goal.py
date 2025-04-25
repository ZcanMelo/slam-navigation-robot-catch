#!/usr/bin/env python
#coding=UTF-8
import rospy
import actionlib
from actionlib_msgs.msg import *
from geometry_msgs.msg import Pose, Point, Quaternion, Twist
from move_base_msgs.msg import MoveBaseAction, MoveBaseGoal
from tf.transformations import quaternion_from_euler
from visualization_msgs.msg import Marker
from std_msgs.msg import String
from math import radians, pi

target_point_map = {"主展厅":"A","区块链与金融科技":"B","三维立体建模":"C","泛在感知与智慧城市":"D","机器学习与智能系统":"E","智能传感与精准医疗":"F","未来智能互联网络":"G"}

class MoveBaseSquare(object):
    def __init__(self):
        self.pose_x = rospy.get_param('~goal_pose_x',0.0)
        self.pose_y = rospy.get_param('~goal_pose_y',0.0)
        self.pose_z = rospy.get_param('~goal_pose_z',0.0)

        self.quat_x = rospy.get_param('~goal_quat_x',0.0)
        self.quat_y = rospy.get_param('~goal_quat_y',0.0)
        self.quat_z = rospy.get_param('~goal_quat_z',0.0)
        self.quat_w = rospy.get_param('~goal_quat_w',1.0)

        self.goal_finish = 0
        self.shutdown_finish = 0
        rospy.on_shutdown(self.shutdown)
        
        rospy.Subscriber('/nav_goal', String, self.speech_callback)
        # Subscribe to the move_base action server
        self.move_base = actionlib.SimpleActionClient("move_base", MoveBaseAction)
        rospy.loginfo("Waiting for move_base action server...")
        # Wait 60 seconds for the action server to become available
        self.move_base.wait_for_server()
        rospy.loginfo("Connected to move base server")
        rospy.loginfo("Starting navigation test")
        rate = rospy.Rate(10) 
        while 1 :
            if self.goal_finish == 1 :
                self.pose_x = rospy.get_param('~goal_pose_x',0.0)
                self.pose_y = rospy.get_param('~goal_pose_y',0.0)
                self.pose_z = rospy.get_param('~goal_pose_z',0.0)

                self.quat_x = rospy.get_param('~goal_quat_x',0.0)
                self.quat_y = rospy.get_param('~goal_quat_y',0.0)
                self.quat_z = rospy.get_param('~goal_quat_z',0.0)
                self.quat_w = rospy.get_param('~goal_quat_w',1.0)
            elif self.shutdown_finish == 1 :
                break
            rate.sleep()

    def speech_callback(self, msg):
        msg.data = target_point_map[msg.data]
        rospy.loginfo(msg.data)
        if msg.data == 'A':
            self.pose_x = rospy.get_param('~goal_pose_x',14.105) # 16.9 19.1
            self.pose_y = rospy.get_param('~goal_pose_y',-1.1892) # -2.24 -1.94
            self.pose_z = rospy.get_param('~goal_pose_z',0.0)

            self.quat_x = rospy.get_param('~goal_quat_x',0.0)
            self.quat_y = rospy.get_param('~goal_quat_y',0.0)
            self.quat_z = rospy.get_param('~goal_quat_z',-0.7285)
            self.quat_w = rospy.get_param('~goal_quat_w',0.685)
        elif msg.data == 'B':
            self.pose_x = rospy.get_param('~goal_pose_x',24.878)
            self.pose_y = rospy.get_param('~goal_pose_y',-4.1544)
            self.pose_z = rospy.get_param('~goal_pose_z',0.0)

            self.quat_x = rospy.get_param('~goal_quat_x',0.0)
            self.quat_y = rospy.get_param('~goal_quat_y',0.0)
            self.quat_z = rospy.get_param('~goal_quat_z',-0.7758)
            self.quat_w = rospy.get_param('~goal_quat_w',0.6310)
        elif msg.data == 'C':
            self.pose_x = rospy.get_param('~goal_pose_x',29.577)
            self.pose_y = rospy.get_param('~goal_pose_y',-4.818)
            self.pose_z = rospy.get_param('~goal_pose_z',0.0)

            self.quat_x = rospy.get_param('~goal_quat_x',0.0)
            self.quat_y = rospy.get_param('~goal_quat_y',0.0)
            self.quat_z = rospy.get_param('~goal_quat_z',0.1843)
            self.quat_w = rospy.get_param('~goal_quat_w',0.9829)
        elif msg.data == 'D':
            self.pose_x = rospy.get_param('~goal_pose_x',10.4)
            self.pose_y = rospy.get_param('~goal_pose_y',25.4)
            self.pose_z = rospy.get_param('~goal_pose_z',0.0)

            self.quat_x = rospy.get_param('~goal_quat_x',0.0)
            self.quat_y = rospy.get_param('~goal_quat_y',0.0)
            self.quat_z = rospy.get_param('~goal_quat_z',0.0)
            self.quat_w = rospy.get_param('~goal_quat_w',1.0)
        elif msg.data == 'shutdown':
            self.shutdown_finish=1
        elif msg.data == 'patrol':
             a=0
        elif msg.data == 'stoppatrol':
             a=0
        else:
            self.pose_x = rospy.get_param('~goal_pose_x',0.0)
            self.pose_y = rospy.get_param('~goal_pose_y',0.0)
            self.pose_z = rospy.get_param('~goal_pose_z',0.0)

            self.quat_x = rospy.get_param('~goal_quat_x',0.0)
            self.quat_y = rospy.get_param('~goal_quat_y',0.0)
            self.quat_z = rospy.get_param('~goal_quat_z',0.0)
            self.quat_w = rospy.get_param('~goal_quat_w',1.0)

        self.goal_finish = 0
	    # rospy.loginfo('Moving the base through velocity commands')
        goal = MoveBaseGoal()
        goal.target_pose.header.frame_id = 'map'
        goal.target_pose.header.stamp = rospy.Time.now()
            
        quaternion = Quaternion()
        quaternion.x = self.quat_x
        quaternion.y = self.quat_y
        quaternion.z = self.quat_z
        quaternion.w = self.quat_w
        goal.target_pose.pose.position.x = self.pose_x 
        goal.target_pose.pose.position.y = self.pose_y
        goal.target_pose.pose.position.z = self.pose_z 
        goal.target_pose.pose.orientation = quaternion
            
        self.move_base.send_goal(goal,self.donecb,self.activecb,self.feedbackcb)

           

    def shutdown(self):
        rospy.loginfo("Stopping the robot...")
        #self.move_base.cancel_goal()
        rospy.sleep(2)

    def donecb(self, status, result):
        rospy.loginfo("navigation done!")
        self.goal_finish = 1


    def activecb(self):
        rospy.loginfo("Goal just went active")
    
    def feedbackcb(self,feedback):
        pass
        rospy.loginfo("navigation feedback\r\n%s"%feedback)

if __name__ == '__main__':
    try:
        rospy.init_node('nav_goal', anonymous=False)
        MoveBaseSquare()
    except (rospy.ROSInterruptException,rospy.ROSException,rospy.ROSInternalException):
        pass
