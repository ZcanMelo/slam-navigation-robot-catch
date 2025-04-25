#include "Rml_robot.h"

int main(int argc, char *argv[])
{

    ros::init(argc, argv, "robot_contrl_node");
    ROS_INFO("RML--------start----------");
    RobotArm robotarm;
    int flag_sub=0;
    while (ros::ok())
    {
        setlocale(LC_ALL, "");
        robotarm.trans_name();
        static bool first_print = true;
        if(first_print){
            ROS_INFO("标志位状态: vehicle=%d, rml=%d, camera=%d", 
            robotarm.vehicle_flag, robotarm.rml_flag, robotarm.camera_flag);
            first_print = false;
        }
        if (robotarm.vehicle_flag > 0)
        {
            ROS_INFO("底盘已停止，等待机械臂抓取");
            sleep(2);
            if(flag_sub==0)
            {
                robotarm.subscrib();
                flag_sub=1;
            }
            // ROS_INFO("%d",robotarm.id);
            if (robotarm.q.size() >= 20)
            {
                ROS_INFO("开始坐标转换");
                robotarm.Storage();
            }
            ROS_INFO("%d,%d,%d",robotarm.vehicle_flag,robotarm.rml_flag,robotarm.camera_flag);
            if (robotarm.vehicle_flag > 0 && robotarm.rml_flag > 0 && robotarm.camera_flag > 0)
            {
                robotarm.rml_flag=0;
                ROS_INFO("机械臂开始启动");
                robotarm.Coordinate_Transformation();
                robotarm.Robot_move();
                robotarm.Robot_gripper();
                sleep(3);
                // robotarm.Robot_gip();
                robotarm.Robot_back();
                sleep(1);
                robotarm.init();
            }
        }
        ros::spinOnce();
    }
    return 0;
}