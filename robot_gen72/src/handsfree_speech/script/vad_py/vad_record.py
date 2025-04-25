#!/usr/bin/env python
#-*- coding: utf-8 -*-
import sys 
import rospy
sys.path.append("./")
from RecordParser import StreamParser
from std_msgs.msg import String

def hear(data):
   print("I hear!")


if __name__ == "__main__":
   stream_test = StreamParser()
   stream_test.open_mic()
   stream_test.callback = hear#stream_test.play_stream
   stream_test.run()

   
