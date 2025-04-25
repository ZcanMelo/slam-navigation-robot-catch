#!/usr/bin/env python3
#coding=UTF-8 
import sys 
sys.path.append("./")
from std_msgs.msg import String
from std_msgs.msg import Int16
import rospy
import json
import time
import socket
import os

wavflag = 0
cur_pos = ""

msg_node = rospy.Publisher("msg", String, queue_size = 1)

# 处理PCM
def callback_tcpclient(endflag):
    flag = endflag.data
    if flag == 1:
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_host = "172.16.11.37"  # 服务器的IP地址或主机名
        server_port = 6000          # 服务器端口号
        BUFFER_SIZE = 1024
        client_socket.connect((server_host, server_port))
        filename='/home/handsfree/handsfree/handsfree_ros_ws/src/handsfree/handsfree_speech/res/speaking_ok.wav'
        file_size=os.stat(filename).st_size #huoqu wenjiandaxiao
        client_socket.send(str(file_size).encode(encoding="utf-8"))
        
        receive_server_ok=client_socket.recv(1024)
        print(("接收到的反馈为",receive_server_ok.decode(encoding="utf-8")))
        
        # 发送音频文件给服务器
        with open(filename, "rb") as audio_file:
            while True:
                data = audio_file.read(1024)
                if not data:
                    break
                client_socket.send(data)
        print("音频文件发送完成")

        # 接收并打印回复的转录文本  
        transcribed_text=client_socket.recv(1024).decode('utf-8')

        print(("Transcript:", transcribed_text))

        msg = String()
        msg.data = transcribed_text
        msg_node.publish(msg)
        client_socket.close()


        
    

if __name__ == '__main__':
    rospy.init_node('main')
    rospy.Subscriber("/record_end_flag", Int16, callback_tcpclient) #订阅 录音结束话题，并且回调相关函数
    rospy.spin()
