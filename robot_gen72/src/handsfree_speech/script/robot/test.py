#!/usr/bin/env python3
#coding=UTF-8 

import rospy
from std_msgs.msg import Int8
from std_msgs.msg import String
import json
import socket
import os
import re  # 导入正则表达式模块

msg_node = rospy.Publisher("msg", String, queue_size = 10)

# 新增送文件并接收响应的函数
def send_wav_file_and_receive_response(server_ip, server_port, file_path):
    # 创建 socket 对象
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    # 连接到服务器
    client_socket.connect((server_ip, server_port))
    
    # 读取并发送文件
    with open(file_path, 'rb') as wav_file:
        # 首先发送文件大小让服务端准备
        wav_data = wav_file.read()
        file_size = len(wav_data)
        client_socket.sendall(file_size.to_bytes(4, 'big'))
        
        # 服务端确认可以开始发送数据
        server_ack = client_socket.recv(1024)  # 假设服务端发送的确认信息不会超过1024字节
        print("服务端确认：", server_ack.decode("utf-8"))
        
        # 然后发送文件内容
        client_socket.sendall(wav_data)
    
    print('文件发送完毕，等待服务端处理并返回结果...')
    
    # 接收服务端返回的消息
    response_data = client_socket.recv(1024) # 根据你的实际情况，你可能需要调整这个接收的字节大小
    if response_data:
        print("接收到服务端的回应：", response_data.decode("utf-8"))
        return response_data.decode("utf-8")
    
    # 操作完成，关闭socket连接
    client_socket.close()

def parse_and_publish(data):
    try:
        parsed_data = json.loads(data)
        objects = []
        # 处理列表格式的数据
        if isinstance(parsed_data, list):
            for action in parsed_data:
                if "object" in action:
                    objects.append(action["object"])
            if msg_node is not None:
                for i in range(3):
                    msg_node.publish(objects[i])
        print("Objects extracted and published:", objects)
    except json.JSONDecodeError:
        print("Received data is not a valid JSON format.")

def find_json_data(text):
    # 使用正则表达式查找可能的JSON列表格式数据
    matches = re.findall(r'\[.*?\](?![\s\S]*\{)', text, re.DOTALL)
    if matches:
        # 尝试解析第一个匹配项
        try:
            json_data = json.loads(matches[0])
            return json_data
        except json.JSONDecodeError as e:
            print("Extracted text is not a valid JSON format.", e)
    return None

def callback_tcpclient(endflag):
    print("进入callback_tcpclient函数")
    flag = endflag.data
    if flag == 1:
        filename = '/home/nvidia/SSD/indoorslam/robot_gen72/src/handsfree_speech/res/speaking_ok.wav'  # 更新为音频文件的实际路径
        #server_ip = '192.168.43.112'  # 服务器IP地址
        # server_ip = '192.168.182.211'  # 服务器IP地址
        # server_ip = '192.168.3.5' 
        server_ip = '192.168.68.44' 
        server_port = 8889  # 服务器端口
        response_data = send_wav_file_and_receive_response(server_ip, server_port, filename)
        json_data = find_json_data(response_data)
        if json_data:
            parse_and_publish(json.dumps(json_data))

if __name__ == '__main__':
    rospy.init_node('speech_processing_node')
    print("节点初始化，等待语音录制结束信号")
    rospy.Subscriber("/record_end_flag", Int8, callback_tcpclient) # 订阅录音结束标志
    rospy.spin()  # 保持节点运行，等待回调函数触发
