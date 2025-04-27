# slam-navigation-robot-catch
本项目是为 2024 Go Sim China 展示开发的示范项目。
项目目标是实现以下流程：通过 Whisper 进行语音识别，利用千问大模型提取指令关键词，将关键词传递给机械臂。同时，车辆接收信号后，基于激光 SLAM 完成定位，并结合预设路线进行自主导航。当车辆到达指定位置后，发送信号至机械臂模块，机械臂通过 YOLOv5 进行目标识别并完成抓取任务。

This project is a demonstration developed for the 2024 Go Sim China event.
The objective is to achieve the following workflow: perform speech recognition using Whisper, extract command keywords using the Qianwen large language model, and transmit the keywords to the robotic arm. Meanwhile, the vehicle receives the signal, localizes itself using LiDAR SLAM, and navigates autonomously based on a predefined route. Upon reaching the target location, the vehicle sends a signal to the robotic arm, which then uses YOLOv5 for object detection and completes the grasping task.

## 📺 项目视频

视频介绍：[点击观看项目演示视频（youtube）](https://www.youtube.com/watch?v=pquhG_lcHEE)

## 📄 项目详细文档

详细项目介绍请参考：[点击下载项目文档](autonomous_manual.pdf)

