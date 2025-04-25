import socket
import re
import requests
import numpy as np
import wave
import os
import subprocess
import threading
import sys
import time
import struct
from modelscope import AutoModelForCausalLM, AutoTokenizer
import socket
import re
import requests
import numpy as np
import wave
import os
import subprocess
import threading
import sys
import time
import struct
from transformers import AutoProcessor, Qwen2_5_VLForConditionalGeneration



def run_whisper(segment_filename):
    # command = f'./main -m models/ggml-base.bin -l zh -f speaking.wav --output-txt'
    # subprocess.run(command, shell=True)
    segment_filename_out = "{}{}".format(segment_filename[:-4],'_out.wav')
    command = f'ffmpeg -i {segment_filename} -acodec pcm_s16le -ac 1 -ar 16000 {segment_filename_out}'
    print(command)
    subprocess.run(command, shell=True)
    command = f'./main -m models/ggml-large.bin -l zh -f speaking_out.wav --output-txt -f "{segment_filename_out}"'
    subprocess.run(command, shell=True)
    vtt_filename = "{}{}".format(segment_filename_out[:-4],'.txt')
    #with open(vtt_filename) as file:
        #transcription = file.read()
    #return transcription

def receive_and_save_client_data(client_socket):
    buffer_size = 1024
    file_size_bytes = client_socket.recv(4)
    file_size = int.from_bytes(file_size_bytes, 'big')
    print("服务端发送的文件大小为 %s" % file_size)
    client_socket.send("准备就绪，可以开始发送文件。".encode("utf-8"))
    file_total_size = file_size
    received_size = 0
    with open("speaking.wav", "wb") as f:
        while received_size < file_total_size:
            data = client_socket.recv(buffer_size)
            f.write(data)
            received_size += len(data)

    print("客户端文件接收完成，大小为 %s 字节" % received_size)
    segment_filename = "speaking.wav"
    run_whisper(segment_filename)
   
    file_path = "speaking.wav.txt"
    chinese_text = read_chinese_from_file(file_path)
    print("提取的中文文本内容为：", chinese_text)
    response_content = test_function_2(chinese_text)    
    client_socket.sendall(response_content.encode('utf-8'))
    client_socket.close()

def start_server():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    host = '192.168.68.44'
    port = 8889
    server_socket.bind((host, port))
    server_socket.listen(1)
    print("等待客户端连接...")
    while True:
        client_socket, client_address = server_socket.accept()
        print(f"连接来自 {client_address}")
        receive_and_save_client_data(client_socket)

def read_chinese_from_file(file_path):
    with open(file_path, "r", encoding='utf-8') as file:
        content = file.read()
        chinese_characters = re.findall(r'[\u4e00-\u9fff]+', content)
        chinese_text = ''.join(chinese_characters)
    return chinese_text

def test_function_2(chinese_text):
    fixed_prompt = """
 {假设你是一个机器人,对于像：“帮我拿一个水瓶/帮我摘一个苹果”这样的抓取指令,请给我返回一个包含action和object的JSON格式数据例如{
    "actions": [
      {"action": "定位", "object": "水瓶"},
      {"action": "移动", "object": "水瓶"},
      {"action": "抓取", "object": "水瓶"},
      {"action": "返回", "object": "用户"},
      {"action": "交付", "object": "水瓶"}
    ]
}
，而对于像“你是谁开发的呀？”这样问答式的问题，请你回复我它的回答，比如：“我是重庆邮电大学智能汽车与信息融合实验室开发集成的”}
"""
    prompt = f"\"{chinese_text}\",{fixed_prompt} "
    messages = [
        {"role": "system", "content": "You are Qwen, created by Alibaba Cloud. You are a helpful assistant."},
        {"role": "user", "content": prompt}
    ]
     # 使用modelscope的生成方式替代openai调用
    text = tokenizer.apply_chat_template(
        messages,
        tokenize=False,
        add_generation_prompt=True
    )
    model_inputs = tokenizer([text], return_tensors="pt").to(model.device)
    
    generated_ids = model.generate(
        **model_inputs,
        max_new_tokens=512
    )
    # 解码生成的响应
    generated_ids = [
        output_ids[len(input_ids):] 
        for input_ids, output_ids in zip(model_inputs.input_ids, generated_ids)
    ]
    response = tokenizer.batch_decode(generated_ids, skip_special_tokens=True)[0]
    
    print("Response received from model:", response)
    return response

if __name__ == '__main__':
    # 初始化模型和分词器
    model_name = "Qwen/Qwen2.5-3B-Instruct"
    model = AutoModelForCausalLM.from_pretrained(
    #model = Qwen2_5_VLForConditionalGeneration.from_pretrained(
        model_name,
        torch_dtype="auto",
        device_map="auto"
    )
    tokenizer = AutoTokenizer.from_pretrained(model_name)

    start_server()
