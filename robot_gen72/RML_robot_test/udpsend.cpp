// #include <iostream>
// #include <cstring>
// #include <cerrno>
// #include <sys/socket.h>
// #include <netinet/in.h>
// // #inlcude<winsock2.h>
// #include <unistd.h>
// #include <arpa/inet.h>
// #include "../../../usr/include/arpa/inet.h"

// void UDP()
// {
//     int udp_flag = 0;
//     int back_flag = 0;
//     int vehicle_flag = 0;
//     int whisper_flag = 0;

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
//     memset(&server_address, 0, sizeof(server_address));
//     server_address.sin_family = AF_INET;
//     server_address.sin_port = htons(12345); // 服务器端口
//     server_address.sin_addr.s_addr = inet_addr("127.0.0.1"); // 服务器IP地址

//     while (true)
//     {
//         char command[10];
//         std::cout << "Enter command (start, finish, exit): ";
//         std::cin >> command;

//         if (strcmp(command, "start") == 0)
//         {
//             udp_flag = 1;
//         }
//         else if (strcmp(command, "finish") == 0)
//         {
//             back_flag = 1;
//         }
//         else if (strcmp(command, "exit") == 0)
//         {
//             break;
//         }

//         if (udp_flag == 1)
//         {
//             std::strcpy(message, "start");
//             sendto(sock, message, strlen(message), 0, (struct sockaddr *)&server_address, sizeof(server_address));
//             udp_flag = 0; // Reset flag
//         }

//         if (back_flag == 1)
//         {
//             std::strcpy(message, "finish");
//             sendto(sock, message, strlen(message), 0, (struct sockaddr *)&server_address, sizeof(server_address));
//             back_flag = 0; // Reset flag
//         }

//         // 接收服务器响应
//         socklen_t server_len = sizeof(server_address);
//         int bytes_received = recvfrom(sock, message, sizeof(message), 0, (struct sockaddr *)&server_address, &server_len);
//         if (bytes_received > 0)
//         {
//             message[bytes_received] = '\0'; // 确保字符串正确终止
//             if (strcmp(message, "stop") == 0)
//             {
//                 vehicle_flag = 1;
//                 std::cout << "Stop message received, vehicle_flag set to 1.车辆停止" << std::endl;
//             }
//             else if (strcmp(message, "go") == 0)
//             {
//                 whisper_flag = 0;
//                 std::cout << "Home message received, whisper_flag set to 0.车辆出发" << std::endl;
//             }
//             else if (strcmp(message, "Home") == 0)
//             {
//                 whisper_flag = 1;
//                 std::cout << "Home message received, whisper_flag set to 1.车辆回到原点" << std::endl;
//             }
//             else if (strcmp(message, "back") == 0)
//             {
//                 std::cout << "Home message received, vehicle is returning.车辆返程" << std::endl;
//             }
//         }
//     }

//     close(sock);
// }

// int main()
// {
//     UDP();
//     return 0;
// }

#include <iostream>
#include <cstring>
#include <cerrno>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <atomic>

std::atomic_bool running(true); // 全局原子变量，用于控制线程运行

void UDPReceiver(int sock, struct sockaddr_in& server_address) {
    char message[1024];
    socklen_t server_len = sizeof(server_address);
    int bytes_received;
    server_address.sin_port = htons(33333);  // 新增接收端口
    bind(sock, (struct sockaddr*)&server_address, sizeof(server_address));  // 新增绑定

    while (running) {
        bytes_received = recvfrom(sock, message, sizeof(message), 0, (struct sockaddr *)&server_address, &server_len);
        if (bytes_received > 0) {
            message[bytes_received] = '\0'; // 确保字符串正确终止
            if (strcmp(message, "stop") == 0) {
                std::cout << "Stop message received.车辆停止" << std::endl;
            }
            else if (strcmp(message, "go") == 0) {
                std::cout << "Go message received.车辆出发" << std::endl;
            }
            else if (strcmp(message, "Home") == 0) {
                std::cout << "Home message received.车辆回到原点" << std::endl;
            }
            else if (strcmp(message, "back") == 0) {
                std::cout << "Back message received.车辆返程" << std::endl;
            }
        }
    }
}

void UDPSender(int sock, struct sockaddr_in& server_address) {
    char command[10];
    char message[1024];
    // server_address.sin_port = htons(33334);
    while (running) {
        std::cout << "Enter command (start, finish, exit): ";
        command[10] = {0};
        std::cin >> command;

        if (strcmp(command, "exit") == 0) {
            running = false; // 设置运行标志为false，退出程序
            break;
        }

        if (strcmp(command, "start") == 0) {
            std::strcpy(message, "start");
            server_address.sin_port = htons(33334);
            sendto(sock, message, strlen(message), 0, (struct sockaddr *)&server_address, sizeof(server_address));
        }
        else if (strcmp(command, "finish") == 0) {
            std::strcpy(message, "finish");
            server_address.sin_port = htons(33335);
            sendto(sock, message, strlen(message), 0, (struct sockaddr *)&server_address, sizeof(server_address));
            }
        else if (strcmp(command, "stop") == 0) {
            std::strcpy(message, "stop");
            server_address.sin_port = htons(33335);
            sendto(sock, message, strlen(message), 0, (struct sockaddr *)&server_address, sizeof(server_address));
        }
    }

    close(sock);
}

int main() {
    int sock;
    struct sockaddr_in server_address;

    // 创建UDP socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }

    // 设置服务器地址
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    // server_address.sin_port = htons(33336); // 服务器端口
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1"); // 服务器IP地址



    //----------------------------------------------------------------------------------------
    ;

    // 创建接收和发送线程
    std::thread receiver_thread(UDPReceiver, sock, std::ref(server_address));
    std::thread sender_thread(UDPSender, sock, std::ref(server_address));

    // 等待发送线程结束
    sender_thread.join();
    // 发送线程结束后，通知接收线程也应该结束
    running = false;
    // 等待接收线程结束
    receiver_thread.join();

    return 0;
}
