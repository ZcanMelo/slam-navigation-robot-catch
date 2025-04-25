#include<iostream>
#include<Eigen/Dense>
#include<ros/ros.h>
#include<string>
#include<sstream>
#include<std_msgs/String.h>
#include"rm_base.h"
#include"yolov5_detect/DetectedObject.h"
#include"yolov5_detect/CommandWithPosition.h"
#include<queue>
#include<vector>
#include <thread>
#include<sys/socket.h>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace Eigen;
using namespace std;
// #include"tf2_ros/transform_listener.h"//监听坐标变换的头文件
// #include"tf2_ros/buffer.h"//缓存数据
// #include"geometry_msgs/PointStamped.h"//坐标点头文件
// #include"tf2_geometry_msgs/tf2_geometry_msgs.h"//tf2坐标转换算法的必要头文件

#define RMERR_SUCC  0
#define ARM_DOF     7


//位置信息
class object_pose
{
    public:
    int class_id;
    float x;
    float y;
    float z;
    object_pose()
    {
        class_id=0;
        x=0.0;
        y=0.0;
        z=0.0;
    }
};
//小车发布的物体信息s
class CommandWithPosition
{
    public:
    string action;
    string object_temp;
};



class RobotArm
{

    private:
    //标识位
    //机械臂节点
        SOCKHANDLE m_sockhand;
    //ros节点
        ros::NodeHandle nh;
        ros::Subscriber sub_process;
        ros::Subscriber sub_vehcil;
        ros::Publisher pub_car;
    //坐标结构体
        object_pose pose;///相机得到的坐标,转化为末端夹抓的坐标
        Pose Rml_pose;//最后的抓取坐标
        CommandWithPosition object;//临时和小车通信存放物体名字
        int flag;
    //存储
        queue<object_pose> queue_do;
    public:
        queue<object_pose> q;
        //标志位
        int id=0;
        int rml_flag=1;//机械臂是否完成动作的标志位
        int vehicle_flag=0;//车辆参数服务器的标志位
        int camera_flag=0;//获取相机物体坐标的标志位
        int udp_flag=0;//是否发布信息的标志位
        int whisper_flag=1;//是否启动语音输入的标志位
        int back_flag=0;//是否返启动回原点的标志位
        //和3588小车通信的线程
        void UDP();
        // 构造和析构函数  
        RobotArm();
        ~RobotArm();
        //参数服务器的读取和写入
        void Setparam();
        void Getparam();
        //透传
        static void MCallback(CallbackData data);
        //处理相机坐标处理
        void Storage();
        //将物体名称转化为编号
        void trans_name();
        //机械臂执行函数
        void base();
        void Coordinate_Transformation();
        void Robot_move();
        void Robot_gripper();
        void Robot_back();
        void Robot_gip();
        void test();
        //标志位以及flag初始化
        void init();
        //消息发布以及订阅函数
        void subscrib();
        void subscribe_vehcil();
        //void subscribe_vehcil();
        void publish();
        //回调函数
        void do_Pose(const yolov5_detect::DetectedObject::ConstPtr& msg);
        void do_vehcil(const std_msgs::String::ConstPtr &msg);
        //多线程开启
        void startUDPThread();
        //
        void UDPSender(int sock, struct sockaddr_in& server_address, std::atomic_bool& running);
        void UDPReceiver(int sock, struct sockaddr_in& server_address, std::atomic_bool& running);
};