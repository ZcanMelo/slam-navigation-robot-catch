#!/usr/bin/env python
#-*- coding: utf-8 -*-
"""
基础工具
"""
from datetime import datetime
import wave
import os,sys

def read_file_data(filename):
    """
    输入:需要读取的文件名
    返回:（声道，量化位数，采样率，数据)
    """
    read_file = wave.open(filename, "r")
    params = read_file.getparams()
    nchannels, sampwidth, framerate, nframes = params[:4]
    data = read_file.readframes(nframes)
    return nchannels, sampwidth, framerate, data

def save_file(data, filename=None, sampwidth=2, channels=1, rate=16000):
    """
    保存数据流文件
    """
    filename=os.path.realpath(__file__)
    if filename[-1]=='y':
        filename=filename[:-8]+"/../../res/speaking_ok.wav"
    if filename[-1]=='c':
        filename=filename[:-9]+"/../../res/speaking_ok.wav"
    #print(filename)
    write_file = wave.open(filename, 'wb')
    write_file.setnchannels(channels)
    write_file.setsampwidth(sampwidth)
    write_file.setframerate(rate)
    write_file.writeframes(data)
    write_file.close()
    return os.path.abspath(filename)

        

    
