#!/usr/bin/env python

import rospy
from std_msgs.msg import String

def talker():
    # 初始化 ROS 节点
    rospy.init_node('msg_node', anonymous=True)
    
    # 创建一个发布者，发布到 "msg" 主题
    pub = rospy.Publisher('msg', String, queue_size=10)
    
    # 设置循环频率为 1 Hz
    rate = rospy.Rate(1)  # 1 Hz

    while not rospy.is_shutdown():
        # 创建要发送的消息
        message = String()
        message.data = "水瓶"
        
        # 发布消息
        pub.publish(message)
        rospy.loginfo("Published: %s", message.data)

        # 等待下一个循环
        rate.sleep()

if __name__ == '__main__':
    try:
        talker()
    except rospy.ROSInterruptException:
        pass