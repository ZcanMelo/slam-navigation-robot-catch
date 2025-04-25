#!/usr/bin/env python3 
# -*- coding: utf-8 -*-
# import socket


import socket
import os

# 创建客户端套接字
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

server_host = "172.16.11.37"  # 服务器的IP地址或主机名
server_port = 6000          # 服务器端口号
BUFFER_SIZE = 1024


# 连接到服务器
client_socket.connect((server_host, server_port))

filename='/home/handsfree/handsfree/handsfree_ros_ws/src/handsfree/offline_mic_vad/audio/input_audio.wav'

file_size=os.stat(filename).st_size #huoqu wenjiandaxiao

client_socket.send(str(file_size).encode(encoding="utf-8"))
    
# 发送音频文件给服务器
with open("/home/handsfree/handsfree/handsfree_ros_ws/src/handsfree/offline_mic_vad/audio/input_audio.wav", "rb") as audio_file:
    while True:
        data = audio_file.read(1024)
        if not data:
            break
        client_socket.send(data)
print("音频文件发送完成")

# 接收并打印回复的转录文本  
text=client_socket.recv(1024).decode('utf-8')

print(("Transcript:", text))

# 关闭连接

