#!/usr/bin/env python3
#coding=UTF-8 
#import whisper
import subprocess
import sys 
sys.path.append("./")
from std_msgs.msg import String
from std_msgs.msg import Int8
import rospy
import requests
import json
import time
import socket
import os
import re  # 导入正则表达式模块

base_url = "http://172.16.11.78:30017"

wavflag = 0

msg_node = rospy.Publisher("msg", String, queue_size = 10)
#response = rospy.Publisher("response_msg", String, queue_size=20) 
#LLM_state = rospy.Publisher("whisper_end_flag", Int8,queue_size=1)
#Whisper_state = rospy.Publisher("record_end_flag", Int8, queue_size=1)

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
                        msg_node.publish(action["object"])
        print("Objects extracted and published:", objects)
    except json.JSONDecodeError:
        print("Received data is not a valid JSON format.")

def find_json_data(text):
    json_data = None
    try:
        # 使用正则表达式查找可能的JSON列表格式数据
        matches = re.findall(r'\[.*?\](?![\s\S]*\{)', text, re.DOTALL)
        if matches:
            # 尝试解析第一个匹配项
            json_data = json.loads(matches[0])
    except json.JSONDecodeError as e:
        print("Extracted text is not a valid JSON format.", e)
    return json_data

# def callback_tcp_whisper(endflag):
#     print("进入callback_whisper函数")
#     flag = endflag.data
#     if flag == 1:
#         try:
#             # cmd = "bash -c 'source /home/nvidia/AGV_Robot/anaconda3/etc/profile.d/conda.sh && conda activate whisper && whisper /home/nvidia/AGV_Robot/agv_test/src/handsfree_speech/res/speaking_ok.wav --model medium --language Chinese'"
#             # #bash -c 命令以允许在同一命令行中顺序执行多个命令
#             # process = subprocess.Popen(cmd,shell=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE)#用于执行复杂的命令行调用
#             # output,error = process.communicate()#等待命令执行完成并获取（输出，错误）
#             model = whisper.load_model("medium")
#             audio_path = "/home/nvidia/AGV_Robot/agv_test/src/handsfree_speech/res/speaking_ok.wav"
#             result = model.transcribe(audio_path)
#             transcription = result['text']
#             filename = "/home/nvidia/AGV_Robot/agv_test/src/handsfree_speech/res/speaking_ok.txt"
#             with open(filename,'w')as text_file:
#                 text_file.write(transcription)
#             #发送call_tcp的标志位
#             int_msg=Int8()
#             int_msg.data = 1
#             LLM_state.publish(int_msg)
#             print("Whisper output:",transcription)

#         except Exception as e:
#             print("Faied to execute whisper command:",str(e))



# 处理PCM
def callback_tcpclient(endflag):
    print("进入callback_tcpclient函数")
    flag = endflag.data
    if flag == 1:
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_host = "192.168.2.86"  # 服务器的IP地址或主机名
        server_port = 8080          # 服务器端口号
        BUFFER_SIZE = 1024
        client_socket.connect((server_host, server_port))
        filename='/home/nvidia/AGV_Robot/agv_test/src/handsfree_speech/res/speaking_ok.wav'
        # file_size=os.stat(filename).st_size #huoqu wenjiandaxiao
        # client_socket.send(str(file_size).encode(encoding="utf-8"))
        
        # receive_server_ok=client_socket.recv(1024)
        # print("接收到的反馈为",receive_server_ok.decode(encoding="utf-8"))
        
        # 发送.wav文件给服务器
        with open(filename, "rb") as wav_file:
            wav_data = wav_file.read()
            file_size = len(wav_data)
            client_socket.sendall(file_size.to_bytes(4, 'big'))
        
            # 服务端确认可以开始发送数据
            server_ack = client_socket.recv(1024)  # 假设服务端发送的确认信息不会超过1024字节
            print("服务端确认：", server_ack.decode("utf-8"))
            # while True:
            #     data = wav_file.read(1024)
            #     if not data:
            #         break
            client_socket.send(wav_data)
        print("音频文件发送完毕，等待服务端处理并返回结果")

        # 接收并打印回复的转录文本  
        response_data=client_socket.recv(1024).decode('utf-8')

        print(("response_data:", response_data))
        json_data = find_json_data(response_data)
        if json_data:
            # 如果找到并提取了JSON数据，则进行解析并发布
            parse_and_publish(json.dumps(json_data))  # 重新转换为字符串，因为parse_and_publish期望字符串输入
        else:
            print("未在响应中找到JSON数据。")



        client_socket.close()
        print("已断开与服务端的连接")



# def create_chat_completion(model, messages, functions, use_stream=False):
#     data = {
#         "function": functions,  # 函数定义
#         "model": model,  # 模型名称
#         "messages": messages,  # 会话历史
#         "stream": use_stream,  # 是否流式响应
#         "max_tokens": 100,  # 最多生成字数
#         "temperature": 0.8,  # 温度
#         "top_p": 0.8,  # 采样概率
#     }

#     response = requests.post(f"{base_url}/v1/chat/completions", json=data, stream=use_stream)
#     if response.status_code == 200:
#         if use_stream:
#             # 处理流式响应
#             for line in response.iter_lines():
#                 if line:
#                     decoded_line = line.decode('utf-8')[6:]
#                     try:
#                         response_json = json.loads(decoded_line)
#                         content = response_json.get("choices", [{}])[0].get("delta", {}).get("content", "")
#                         print(content)
#                     except:
#                         print(("Special Token:", decoded_line))
#         else:
#             # 处理非流式响应
#             decoded_line = response.json()
#             content = decoded_line.get("choices", [{}])[0].get("message", "").get("content", "")
#             print(content)
#             return content
#     else:
#         print(("Error:", response.status_code))
#         return None


# def function_chat(use_stream=True):
#     functions = [
#         {
#             "name": "get_current_weather",
#             "description": "Get the current weather in a given location.",
#             "parameters": {
#                 "type": "object",
#                 "properties": {
#                     "location": {
#                         "type": "string",
#                         "description": "The city and state, e.g. Beijing",
#                     },
#                     "unit": {"type": "string", "enum": ["celsius", "fahrenheit"]},
#                 },
#                 "required": ["location"],
#             },
#         }
#     ]
#     chat_messages = [
#         {
#             "role": "user",
#             "content": "波士顿天气如何？",
#         },
#         {
#             "role": "assistant",
#             "content": "get_current_weather\n ```python\ntool_call(location='Beijing', unit='celsius')\n```",
#             "function_call": {
#                 "name": "get_current_weather",
#                 "arguments": '{"location": "Beijing", "unit": "celsius"}',
#             },
#         },
#         {
#             "role": "function",
#             "name": "get_current_weather",
#             "content": '{"temperature": "12", "unit": "celsius", "description": "Sunny"}',
#         },
#         # ... 接下来这段是 assistant 的回复和用户的回复。
#         # {
#         #     "role": "assistant",
#         #     "content": "根据最新的天气预报，目前北京的天气情况是晴朗的，温度为12摄氏度。",
#         # },
#         # {
#         #     "role": "user",
#         #     "content": "谢谢",
#         # }
#     ]
#     answer = create_chat_completion("chatglm3-6b", messages=chat_messages, functions=functions, use_stream=use_stream)
#     return answer

# def simple_chat(use_stream,prompt):
#     functions = None
#     chat_messages = [
#         {
#             "role": "system",
#             "content": "你是一个光明实验室研发的具身智能机器人",
#         },
#         {
#             "role": "user",
#             "content": prompt
#         }
#     ]
#     answer = create_chat_completion("chatglm3-6b", messages=chat_messages, functions=functions, use_stream=use_stream)
#     return answer


# def callback_msg(msg):
#     global cur_pos
#     wav = String()
#     #添加一个判断，输出文字长度小于8，不进入大预言模型。
#     if len(msg.data) >= 4 :
#     #rospy.loginfo(msg.data)
#         prompt = msg.data
#         chatresponse=simple_chat(False, prompt)
#         rospy.loginfo(prompt)
#         rospy.loginfo(chatresponse)
#         #发布一个response
#         response.publish(chatresponse)
#     # 调用播放模块播放res1
#     #LLMstate.publish(0) #关闭互锁状态

if __name__ == '__main__':
    rospy.init_node('main')
    print("运行中, 等待语音消息")
    rospy.Subscriber("/record_end_flag", Int8, callback_tcpclient) #订阅 录音结束话题，并且回调相关函数
    #rospy.Subscriber("/whisper_end_flag",Int8,callback_tcpclient)
    #rospy.Subscriber("/msg", String, callback_msg) #订阅识别文字话题，
    rospy.spin()
