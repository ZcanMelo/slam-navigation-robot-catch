#include "Rml_robot.h"
#include <Eigen/Dense>

float joint_angle_lift[7] = {0}; // 抓取后提高
// float joint_test[7] = {26.641, 29.844, 0.043, -56.309, 2.977, -3.479,-24.776}; // 放置点
// float joint_base[7] = {0.017,-31.689, 0.033, -36.203, -0.824, -13.632,-24.771};   // 原点
float joint_test[7] = {1.872,-0.269,5.856,-80.075,10.967,35.786,-21.107}; // 放置点
// float joint_base[7] = {89.288,-3.566,-0.137,-29.158,0.554,-12.918,-23.701}; // 原点
float joint_base[7] = {85.892,5.003,3.304,-45.561,0.274,10.335,118.911};
struct sockaddr_in server_address,cliaddr;


RobotArm::RobotArm()
{
    ROS_INFO("1111111111111111111111111111111111");
    // setlocale(LC_CTYPE, "en_US.UTF-8"); // 新增编码设置
    m_sockhand = -1;
    RM_API_Init(ARM_GEN72,MCallback);
    ROS_INFO("2222222222222222222222222222222222");
    m_sockhand = Arm_Socket_Start((char*)"192.168.1.18", 8080, 200);
    ROS_INFO("3333333333333333333333333333333333");
    if(m_sockhand < 0) { // 新增连接检查
        ROS_ERROR("机械臂连接失败！错误码: %d", m_sockhand);
        exit(EXIT_FAILURE); // 严重错误直接退出
    }
    else 
    {
    ROS_INFO("机械臂连接成功 SocketHandle: %d", m_sockhand);
    Set_Collision_Stage(m_sockhand, 4, RM_NONBLOCK); // 设置碰撞保护等级
    char *version;
    version = API_Version();
    ROS_INFO("API_Version :%s", version);
       // FRAME* name;
    // Algo_Get_Curr_WorkFrame(name);
    // ROS_INFO("name:%s", name->frame_name.name);
    // 修改后（正确写法）
    // FRAME name; // 声明栈变量
    // Algo_Get_Curr_WorkFrame(&name); // 传递指针地址
    // ROS_INFO("name:%s", name.frame_name.name);
    Set_Voltage(m_sockhand, 3, true);
    int Set=Set_Modbus_Mode(m_sockhand, 1, 115200, 2, 1);
    // cout<<"modbus"<<Set<<endl;
    this->base();
    this->startUDPThread();
    this->subscribe_vehcil();
}
    
 
}



RobotArm::~RobotArm()
{
    
    Arm_Socket_Close(m_sockhand);
    ros::param::set("arm_code", 0);
    m_sockhand = -1;
    ROS_INFO("已调用析构函数");
}

void RobotArm::MCallback(CallbackData data)
{
    // 判断接口类型
    switch (data.errCode)
    {
    case MOVEJ_CANFD_CB: // 角度透传
        printf("MOVEJ_CANFD 透传结果: %d\r\n", data.errCode);
        break;
    case MOVEP_CANFD_CB: // 位姿透传
        printf("MOVEP_CANFD  透传结果: %d\r\n", data.errCode);
        break;
    case FORCE_POSITION_MOVE_CB: // 力位混合透传
        printf("FORCE_POSITION_MOVE  透传结果: %d\r\n", data.errCode);
        break;
    }
}

void RobotArm::Coordinate_Transformation()
{
    float joint_angle[6] = {0};
    float *joint_angle_ptr = joint_angle;
    int ret_getjoint = 0;
    ret_getjoint = Get_Joint_Degree(m_sockhand, joint_angle_ptr);
    if (ret_getjoint != 0)
    {
        ROS_INFO("获取关节信息失败,ret=%d", ret_getjoint);
        return;
    }
    Rml_pose = Algo_Cartesian_Tool(joint_angle_ptr, this->pose.x, this->pose.y, this->pose.z);
    cout<<"在基坐标系下为"<<"x: "<<pose.x<<"y: "<<pose.y<<"z: "<<pose.z<<endl;
}

void RobotArm::base()
{
    float *joint_base_ptr = joint_base;
    int ret_base;
    ret_base = Movej_Cmd(m_sockhand, joint_base_ptr, 14, 0, 0, 0);
    ROS_INFO("ret_base=%d",ret_base);
    if (ret_base != 0)
    {
        ROS_INFO("机械臂移动base错误,ret=%d", ret_base);
        return;
    }
    else{
        ROS_INFO("目标关节角度: [%.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f]", 
               joint_base[0], joint_base[1], joint_base[2],
               joint_base[3], joint_base[4], joint_base[5], joint_base[6]);
        ROS_INFO("机械臂已到达初始位置");
    }
    int ret_pickgirpper = Write_Single_Register(m_sockhand,1, 40000, 100, 1, RM_BLOCK);
}

void RobotArm::Robot_move()
{
    // int ret_pickgirpper = Write_Single_Register(m_sockhand,1, 40000, 100, 1, RM_BLOCK);
    int ret_movejoint;
    ret_movejoint = Movel_Cmd(m_sockhand, Rml_pose, 30, 0, 0, RM_BLOCK);
    if (ret_movejoint != 0)
    {
        ROS_INFO("机械臂移动move错误,ret=%d", ret_movejoint);
        return;
    }
}

void RobotArm::Robot_gripper()
{
    int ret_pickgirpper;
    // float joint_test[6] = {-88.541, -9.913, -126.885, 7.189, 34.318, 89.706};
    ret_pickgirpper = Write_Single_Register(m_sockhand, 1, 40000, 10, 1, 0);
    if (ret_pickgirpper != 0)
    {
        ROS_INFO("机械臂giper错误,ret=%d", ret_pickgirpper);
        return;
    }
}

void RobotArm::Robot_back()
{
    // int ret_backjoint;
    // ret_backjoint = Movej_Cmd(m_sockhand, joint_test, 10, 0, 0, RM_BLOCK);
    // if (ret_backjoint != 0)
    // {
    //     ROS_INFO("机械臂移动put错误,ret=%d", ret_backjoint);
    //     return;
    // }
    // int ret_pickgirpper = Write_Single_Register(m_sockhand, 1, 40000, 100, 1, 1);
    // float joint_base[6]={-1.020,67.013,-111.033,-0.232,-49.552,89.725};
    float *joint_base_ptr = joint_base;
    int ret_base;
    ret_base = Movej_Cmd(m_sockhand, joint_base, 20, 0, 0, RM_BLOCK);
    if (ret_base != 0)
    {
        ROS_INFO("机械臂移动back错误,ret=%d", ret_base);
        return;
    }
}

void RobotArm::Robot_gip()
{
    float *joint_lift_ptr = joint_angle_lift;
    int ret_getjoint = 1;
    ret_getjoint = Get_Joint_Degree(m_sockhand, joint_lift_ptr);
    if (ret_getjoint != 0)
    {
        ROS_INFO("机械臂移动错误,ret=%d", ret_getjoint);
        return; // sleep(1);
    }
    int ret_lift;
    joint_angle_lift[3] += 10;
    // joint_angle_lift[4] += 10;
    ret_lift = Movej_Cmd(m_sockhand, joint_angle_lift, 20, 0, 0, RM_BLOCK);
    sleep(0.5);
}

void RobotArm::Storage()
{
    // 旋转矩阵
    Eigen::Matrix3d R;
    // R << 0.00351668, -0.99974851, 0.02214832,
    //     0.99998773, 0.0035931, 0.00341149,
    //     -0.00349022, 0.02213605, 0.99974888;

    // // 平移矩阵
    // Eigen::Vector3d t(0.10384907, -0.03292468, 0.03377062);

    // R << -0.347, -0.936, -0.05,
    //     -0.937, -0.349, 0.023,
    //     -0.039, -0.039, 0.998;

    // // 平移矩阵
    // Eigen::Vector3d t(-0.042, -0.055, 0.204);
    
    //old
    //     R << -0.378, -0.926, 0.002,
    //     0.925, -0.378, 0.021,
    //     -0.018, 0.01, 1.0;
    // // 平移矩阵
    // Eigen::Vector3d t(0.047, -0.014, 0.12);

       R << -0.04, -0.995, 0.087,
        0.998, -0.044, -0.05,
        0.053, 0.084, 0.995;
     // 平移矩阵
    Eigen::Vector3d t(0.043, -0.033, 0.149);

    if (id < 0 || id > 80)
    {
        return;
    }
    object_pose stro_temp;
    while (!q.empty())
    {
        stro_temp = q.front();
        if (this->id == stro_temp.class_id)
        {
            queue_do.push(stro_temp);
            // ROS_INFO("存入的id和x坐标%d,%f",stro_temp.class_id,stro_temp.x);
        }
        // ROS_INFO("此时的id和x坐标%d,%f",stro_temp.class_id,stro_temp.x);
        q.pop();
    }
    // ROS_INFO("%d,%f",stro_temp.class_id,stro_temp.x);
    // object_pose test;
    // test=queue_do.front();
    // ROS_INFO("动态数组里的坐标%d,%f",test.class_id,test.x);
    int i = 0;
    object_pose do_temp;
    while (!queue_do.empty())
    {
        i++;
        stro_temp = queue_do.front();
        queue_do.pop();
        do_temp.class_id = stro_temp.class_id;
        do_temp.x += stro_temp.x;
        do_temp.y += stro_temp.y;
        do_temp.z += stro_temp.z;
    }
    object_pose put_out;

    if (do_temp.class_id == 0 && do_temp.x == 0)
    {
    }
    else
    {

        put_out.class_id = do_temp.class_id;
        put_out.x = do_temp.x / i;
        put_out.y = do_temp.y / i;
        put_out.z = do_temp.z / i;
        // class_id的分类别
        std::vector<std::string> classes = {"person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", "traffic light",
                                            "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", "cow",
                                            "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
                                            "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard",
                                            "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple",
                                            "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch",
                                            "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone",
                                            "microwave", "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors", "teddy bear",
                                            "hair drier", "toothbrush"};

        string name = classes[put_out.class_id];
        ROS_INFO("需要抓取的物体id是%d,类别是%s,在相机下的坐标为：%f,%f,%f", put_out.class_id, name.c_str(), put_out.x, put_out.y, put_out.z);
        Eigen::Vector3d P_camera(put_out.x, put_out.y, put_out.z);
        Eigen::Vector3d P_robot = R * P_camera + t;
        this->pose.class_id = put_out.class_id;
        this->pose.x = P_robot.x();
        this->pose.y = P_robot.y();
        this->pose.z = P_robot.z();
        // this->pose.z -= 0.12;
        // this->pose.z -= 0.155;
        this->pose.z -= 0.2;
        ROS_INFO("需要抓取的物体id是%d,类别是%s,在末端下的坐标为：%f,%f,%f", pose.class_id, name.c_str(), pose.x, pose.y, pose.z);
        this->camera_flag = 1;
    }
    i = 0;
}

void RobotArm::do_Pose(const yolov5_detect::DetectedObject::ConstPtr &msg)
{
    if(this->vehicle_flag==1)
    {
        object_pose temp;
        temp.class_id = msg->class_id;
        temp.x = msg->x;
        temp.y = msg->y;
        // robotarm.trans_name();
        temp.z = msg->z;
        // ROS_INFO("push vector");
        q.push(temp);
    }
    
}

void RobotArm::subscrib()
{
    sub_process = nh.subscribe<yolov5_detect::DetectedObject>("/detected_objects", 30, &RobotArm::do_Pose, this);
}

void RobotArm::do_vehcil(const std_msgs::String::ConstPtr &msg)
{
    if (this->whisper_flag == 1)
    {
        this->object.object_temp = msg->data;
        ROS_INFO("通讯成功%s,需要抓取的物体id为", object.object_temp.c_str());
        this->trans_name();
    }
    
}

void RobotArm::subscribe_vehcil()
{
    sub_vehcil = nh.subscribe<std_msgs::String>("msg", 20, &RobotArm::do_vehcil, this);
}

// void RobotArm::Getparam()
// {
//     int temp = 0;
//     ros::param::get("state_code", temp);
//     this->vehicle_flag = temp;
//     if (temp == 1)
//     {
//         ROS_INFO("参数服务器获取的参数为%d", vehicle_flag);
//     }
// }

void RobotArm::Setparam()
{
    ros::param::set("arm_code", 1);
    int temp;
    ros::param::get("arm_code", temp);
    if (temp == 1)
    {
        ROS_INFO("参数服务器获取的参数为%d", temp);
    }
}

void RobotArm::trans_name()
{
    if (this->object.object_temp == "水瓶")
    {
        this->id = 39;
        // ROS_INFO("底盘通信：需要抓取的物体是水瓶");
        this->udp_flag = 1;
        
    }
    else if (this->object.object_temp == "鼠标")
    {
        this->id = 64;
        // ROS_INFO("底盘通信：需要抓取的物体是鼠标");
        this->udp_flag = 1;
        
    }

    else
    {
        // this->id = 0;
        return;
    }
}

void RobotArm::init()
{
    this->rml_flag = 1;
    this->camera_flag = 0;
    this->vehicle_flag=0;
    this->back_flag=1;
}

// void RobotArm::UDP()
// {
//     int sock;
//     struct sockaddr_in server_address;
//     char message[1024]; 

//     // 创建UDP socket
//     sock = socket(AF_INET, SOCK_DGRAM, 0);
//     if (sock < 0)
//     {
//         std::cerr << "Socket creation error" << std::endl;
//         return;
//     }

//     // 设置服务器地址
//     server_address.sin_family = AF_INET;
//     server_address.sin_port = htons(12345);                  // 服务器端口
//     server_address.sin_addr.s_addr = inet_addr("127.0.0.1"); // 服务器IP地址
//     while (true)
//     {
//         // 根据FLAG的值决定是否发送"start"消息
//         if (this->udp_flag == 1)
//         {
//             std::strcpy(message, "start");
//             sendto(sock, message, strlen(message), 0, (struct sockaddr *)&server_address, sizeof(server_address));
//         }

//         if (this->back_flag == 1)
//         {
//             std::strcpy(message, "finish");
//             sendto(sock, message, strlen(message), 0, (struct sockaddr *)&server_address, sizeof(server_address));
//         }

//         // 接收服务器响应（如果需要的话）
//         int server_len = sizeof(server_address);
//         int bytes_received = recvfrom(sock, message, sizeof(message), 0, (struct sockaddr *)&server_address, &server_len);
//         if (bytes_received > 0)
//         {
//             message[bytes_received] = '\0'; // 确保字符串正确终止
//             if (strcmp(message, "stop") == 0)
//             {
//                 this->vehicle_flag = 1; // 收到stop消息后设置vehicle_flag为1
//                 std::cout << "Stop message received, vehicle_flag set to 1.车辆停止" << std::endl;
//             }
//             else if(strcmp(message, "go") == 0) 
//             {
//                 this->whisper_flag = 0; // 收到stop消息后设置vehicle_flag为1
//                 this->udp_flag = 0;
//                 std::cout << "Home message received, whisper_fla set to 0.车辆出发" << std::endl;
//             }
//             else if(strcmp(message, "Home") == 0) 
//             {
//                 this->whisper_flag = 1; // 收到stop消息后设置vehicle_flag为1
//                 std::cout << "Home message received, whisper_flag set to 1.车辆回到原点" << std::endl;
//             }
//             else if(strcmp(message, "back") == 0) 
//             {
//                 this->back_flag=0; // 收到stop消息后设置vehicle_flag为1
//                 std::cout << "Home message received, back_flag set to 1.车辆返程" << std::endl;
//             }
//         }
//     }
//     close(sock);
// }
// void RobotArm::UDP() {
//         int sock;
//         struct sockaddr_in server_address;
//         char message[1024]; 

//         // 创建UDP socket
//         sock = socket(AF_INET, SOCK_DGRAM, 0);
//         if (sock < 0) {
//             std::cerr << "Socket creation error" << std::endl;
//             return;
//         }

//         // 设置服务器地址
//         server_address.sin_family = AF_INET;
//         server_address.sin_port = htons(12345);                  // 服务器端口
//         server_address.sin_addr.s_addr = inet_addr("127.0.0.1"); // 服务器IP地址

//         socklen_t server_len = sizeof(server_address);
//         while (true) {
//             // 根据FLAG的值决定是否发送"start"消息
//             if (this->udp_flag == 1) {
//                 std::strcpy(message, "start");
//                 if (sendto(sock, message, strlen(message), 0, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
//                     std::cerr << "Sendto error" << std::endl;
//                 }
//             }

//             if (this->back_flag == 1) {
//                 std::strcpy(message, "finish");
//                 if (sendto(sock, message, strlen(message), 0, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
//                     std::cerr << "Sendto error" << std::endl;
//                 }
//             }

//             // 接收服务器响应（如果需要的话）
//             int bytes_received = recvfrom(sock, message, sizeof(message), 0, (struct sockaddr *)&server_address, &server_len);
//             if (bytes_received > 0) {
//                 message[bytes_received] = '\0'; // 确保字符串正确终止
//                 if (strcmp(message, "stop") == 0) {
//                     this->vehicle_flag = 1; // 收到stop消息后设置vehicle_flag为1
//                     std::cout << "Stop message received, vehicle_flag set to 1.车辆停止" << std::endl;
//                 } else if (strcmp(message, "go") == 0) {
//                     this->whisper_flag = 0; // 收到go消息后设置whisper_flag为0
//                     this->udp_flag = 0;
//                     std::cout << "Go message received, whisper_flag set to 0.车辆出发" << std::endl;
//                 } else if (strcmp(message, "Home") == 0) {
//                     this->whisper_flag = 1; // 收到Home消息后设置whisper_flag为1
//                     std::cout << "Home message received, whisper_flag set to 1.车辆回到原点" << std::endl;
//                 } else if (strcmp(message, "back") == 0) {
//                     this->back_flag = 0; // 收到back消息后设置back_flag为0
//                     std::cout << "Back message received, back_flag set to 0.车辆返程" << std::endl;
//                 }
//             } else if (bytes_received < 0) {
//                 std::cerr << "Recvfrom error" << std::endl;
//             }
//         }
//         close(sock);
//     }

void RobotArm::startUDPThread()
{
    std::thread udpThread(&RobotArm::UDP, this);
    udpThread.detach(); // 分离线程，使其在后台运行
}

// 接收线程函数
void RobotArm::UDPReceiver(int sock, struct sockaddr_in& server_address, std::atomic_bool& running) {
    char message[1024];
    socklen_t server_len = sizeof(server_address);
    while (running) {
        int bytes_received = recvfrom(sock, message, sizeof(message), 0, (struct sockaddr *)&server_address, &server_len);
        if (bytes_received > 0) {
            message[bytes_received] = '\0'; // 确保字符串正确终止
             if (strcmp(message, "stop") == 0) {
                    this->vehicle_flag = 1; // 收到stop消息后设置vehicle_flag为1
                    std::cout << "Stop message received, vehicle_flag set to 1.车辆停止" << std::endl;
                } else if (strcmp(message, "go") == 0) {
                    this->whisper_flag = 0; // 收到go消息后设置whisper_flag为0
                    this->udp_flag = 0;
                    std::cout << "Go message received, whisper_flag set to 0.车辆出发" << std::endl;
                } else if (strcmp(message, "Home") == 0) {
                    this->whisper_flag = 1; // 收到Home消息后设置whisper_flag为1
                    std::cout << "Home message received, whisper_flag set to 1.车辆回到原点" << std::endl;
                } else if (strcmp(message, "back") == 0) {
                    this->back_flag = 0; // 收到back消息后设置back_flag为0
                    std::cout << "Back message received, back_flag set to 0.车辆返程" << std::endl;
                }
            } else if (bytes_received < 0) {
                std::cerr << "Recvfrom error" << std::endl;
            }
        }
    }
// 发送线程函数
void RobotArm::UDPSender(int sock, struct sockaddr_in& server_address, std::atomic_bool& running) {
    char message[1024];
    while (running) {
        // 根据FLAG的值决定是否发送消息
        if (this->udp_flag == 1) {
            std::strcpy(message, "start");
            server_address.sin_port = htons(33334); // 服务器端口
            sendto(sock, message, strlen(message), 0, (struct sockaddr *)&server_address, sizeof(server_address));
            cout<<"start发送成功"<<std::endl;
            this->udp_flag = 0; // 重置标志
        }

        if (this->back_flag == 1) {
            std::strcpy(message, "finish");
            server_address.sin_port = htons(33335); // 服务器端口
            sendto(sock, message, strlen(message), 0, (struct sockaddr *)&server_address, sizeof(server_address));
            this->back_flag = 0; // 重置标志
        }

        // 休眠一段时间，避免忙等待
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

void RobotArm::UDP() 
{
    int sock;
    std::atomic_bool running(true); // 控制线程运行的原子标志

    // 创建UDP socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return;
    }

    // 设置服务器地址
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(33333); // 服务器端口
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    // server_address.sin_addr.s_addr = INADDR_ANY;

    // 绑定socket到服务器地址和端口
    if (bind(sock, (struct sockaddr*)&server_address, sizeof(server_address)) < 0)
    {
        std::cerr << "Bind failed" << std::endl;
        close(sock);
        return ;
    }

    cliaddr.sin_family = AF_INET;
    cliaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // 客户端 IP 地址
    cliaddr.sin_port = htons(33334); // 端口号

    // 创建接收和发送线程
    std::thread receiver_thread(&RobotArm::UDPReceiver, this, sock, std::ref(server_address), std::ref(running));
    std::thread sender_thread(&RobotArm::UDPSender, this, sock, std::ref(cliaddr), std::ref(running));

    // 等待用户输入退出命令
    char command[10];
    std::cout << "Enter 'exit' to quit." << std::endl;
    std::cin >> command;
    if (strcmp(command, "exit") == 0) {
        running = false; // 设置运行标志为false，退出线程
    }

    // 等待线程结束
    receiver_thread.join();
    sender_thread.join();

    close(sock);
}
