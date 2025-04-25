import socket
import wave
import os

sock=socket.socket(socket.AF_INET, socket.SOCK_DGRAM) 

server_address2 = ('172.16.11.37',6000)


BUFFER_SIZE = 1024

filename='/home/handsfree/handsfree/handsfree_ros_ws/src/handsfree/offline_mic_vad/audio/input_audio.wav'

file_size=os.stat(filename).st_size #huoqu wenjiandaxiao

sock.sendto(str(file_size).encode(encoding="utf-8"),server_address2)

print(file_size)

with open("/home/handsfree/handsfree/handsfree_ros_ws/src/handsfree/offline_mic_vad/audio/input_audio.wav", "rb") as audio_file:
    while True:
        data = audio_file.read(1024)
        if not data:
            break
        sent = sock.sendto(data,server_address2)

message, server_address = sock.recvfrom(BUFFER_SIZE)
print((message.decode('utf-8')))

