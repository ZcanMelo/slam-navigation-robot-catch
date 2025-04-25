import socket
import os
import time

def Get_FilePath_FileName_FileExt(filename):
    filepath, tempfilename = os.path.split(filename)
    shotname, extension = os.path.splitext(tempfilename)
    return filepath, shotname, extension

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

filename = "/home/handsfree/handsfree/handsfree_ros_ws/src/handsfree/offline_mic_vad/audio/input_audio.wav"
filepath, shotname, extension = Get_FilePath_FileName_FileExt(filename)

client_addr = ('172.16.4.188',6000)
f = open(filename,'rb')
count = 0
flag = 1
while True:
    if count == 0:
        data = bytes(shotname+extension, encoding = "utf8")
        start = time.time()
        s.sendto(data,client_addr)
    data = f.read(1024)
    if str(data) != "b''":
        s.sendto(data,client_addr)
        print((str(count)+'byte'))

    else:
        s.sendto('end'.encode('utf-8'),client_addr)
        break
    data, server_addr = s.recvfrom(1024)
    count+=1
print(('recircled'+str(count)))
s.close
end = time.time()
print(('cost'+str(round(end-start,2))+'s'))
