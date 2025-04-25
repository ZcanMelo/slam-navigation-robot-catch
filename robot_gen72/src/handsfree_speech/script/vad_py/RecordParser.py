#!/usr/bin/env python
#-*- coding: utf-8 -*-
import sys 
sys.path.append("../")
from audio import Audio
from vad import Vad 
import threading
import rospy
from std_msgs.msg import String


class StreamParser(Vad):
    def __init__(self):
        self.record = Audio()
        self.active = False
        self.play =Audio()
        Vad.__init__(self)
   
    def open_mic(self):
        print("start recording")      
        t = threading.Thread(target=self.mic_record)
        t.setDaemon(True)
        t.start()
        #t.join()
 
    def mic_record(self):
        self.record.record_stream_start()
        self.active = True
        print("The microphone has opened")
        while self.active: 
            data = self.record.record_read()
            self.add(data)
        self.record.record_stream_end()
        print("exit mic")
        

    def close_mic(self):
        print("stop recording")
        if self.record:
            print(self.active)
            self.active = False
       

    def play_stream(self, data):
        #rospy.loginfo("I hear!")
        print("I hear!")
        #pub.publish("I hear!")
        self.play.play_stream(data)

def test():
    stream_test = StreamParser()
    stream_test.open_mic()
    import time
    time.sleep(5)
    stream_test.close_mic()
    time.sleep(5)
    import util
    data = "".join(stream_test.cache_frames)      
    util.save_file(data)
    stream_test.play_stream(data)

def main():
    stream_test = StreamParser()
    stream_test.open_mic()
    import time
    while 1:
        
        try:
            time.sleep(5)
        except KeyboardInterrupt as e:
            #time.sleep(5)
            stream_test.close_mic()
            break
    import util
    data = "".join(stream_test.cache_frames)      
    util.save_file(data)
    stream_test.play_stream(data)

if __name__ == "__main__":
    main()
    





