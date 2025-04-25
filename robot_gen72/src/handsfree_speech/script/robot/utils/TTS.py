#!/usr/bin/env python
#coding:utf‐8
import subprocess
import os
import rospy
from std_msgs.msg import String

# 切换到可执行文件的目录
executable_dir = "/home/handsfree/xf_mic/src/xf_mic_asr_offline/Linux_tts/bin/"
save_dir = "/home/handsfree/handsfree/handsfree_ros_ws/src/planning_core/tmp/"

os.chdir(executable_dir)


# 设置DISPLAY环境变量
os.environ['DISPLAY'] = ':0'

def TTS_generation(text_parameter):
    # 字符串参数
    # text_parameter = "哈哈，林杰不想玩啦，哈哈"

    # 文件名参数
    output_wav = save_dir + "tts.wav"

    # 将参数合并为一个命令字符串
    command = "./tts_offline_sample \"{}\" {}".format(text_parameter.data, output_wav)

    try:
        # 使用subprocess运行命令
        subprocess.call(command, shell=True)

        TTS_play()
        print("TTS_generation执行成功!!!")
    except subprocess.CalledProcessError as e:
        print(("TTS_generation执行失败: {}".format(e)))

def TTS_play():
    # 文件名参数
    output_wav = save_dir + "tts.wav"

    # 将参数合并为一个命令字符串
    command = "play {}".format(output_wav)

    try:
        # 使用subprocess运行命令
        subprocess.call(command, shell=True)
        print("TTS_play执行成功!!!")
    except subprocess.CalledProcessError as e:
        print(("TTS_play执行失败: {}".format(e)))

if __name__ == "__main__":
    # TTS_generation()
    rospy.init_node('tts')
    rospy.Subscriber("/tts", String, TTS_generation) #接受topic
    rospy.spin()