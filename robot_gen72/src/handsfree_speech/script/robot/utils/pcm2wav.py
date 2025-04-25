#!/usr/bin/env python
#coding:utf‐8
import wave
import struct
import rospy
from std_msgs.msg import String

def pcm2wav_callback():
    # 输入PCM文件和输出WAV文件的路径
    pcm_file = '/home/handsfree/handsfree/handsfree_ros_ws/src/handsfree/offline_mic_vad/audio/vvui_deno.pcm'
    wav_file = '/home/handsfree/handsfree/handsfree_ros_ws/src/handsfree/offline_mic_vad/audio/input_audio.wav'  # 在当前目录下创建的WAV文件

    # 设置WAV文件的参数
    num_channels = 1  # 单声道
    sample_width = 2  # 16位样本宽度
    frame_rate = 16000  # 采样率

    # 打开PCM文件进行读取
    with open(pcm_file, 'rb') as pcmfile:
        pcmdata = pcmfile.read()

    # 创建WAV文件并写入PCM数据
    wavfile = wave.open(wav_file, 'wb')  # 不使用with语句
    wavfile.setnchannels(num_channels)
    wavfile.setsampwidth(sample_width)
    wavfile.setframerate(frame_rate)
    wavfile.writeframes(pcmdata)
    wavfile.close()  # 手动关闭WAV文件

    print(('{} 已成功转换为 {}'.format(pcm_file, wav_file)))

if __name__ == '__main__':
    pcm2wav_callback()
    # rospy.init_node('pcm2wav')
    # rospy.Subscriber("/pcm2wav", String, pcm2wav_callback) #接受topic
    # rospy.spin()
