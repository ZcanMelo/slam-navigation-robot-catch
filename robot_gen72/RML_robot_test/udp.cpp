#include <iostream>
#include <cstring>
#include <cerrno>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <atomic>

std::atomic_bool running(true);

void UDPReceiver(int sock, struct sockaddr_in& client_address, socklen_t& client_len)
{
    char buffer[1024];
    int bytes_read;

    while (running)
    {
        bytes_read = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_address, &client_len);
        if (bytes_read > 0)
        {
            buffer[bytes_read] = '\0'; // 确保字符串正确终止
            std::cout << "Received message: " << buffer << std::endl;
        }
        else if (bytes_read < 0)
        {
            std::cerr << "Receive error" << std::endl;
            break;
        }
    }
}

void UDPSender(int sock, struct sockaddr_in& client_address, socklen_t& client_len)
{
    char buffer[1024];

    while (running)
    {
        std::cout << "Enter message to send (or 'exit' to quit): ";
        std::cin.getline(buffer, sizeof(buffer));

        if (strcmp(buffer, "exit") == 0)
        {
            running = false;
            break;
        }

        // 发送消息到客户端
        sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr*)&client_address, client_len);
    }

    close(sock);
}

int main()
{
    int sock;
    struct sockaddr_in server_address, client_address;
    socklen_t client_len = sizeof(client_address);

    // 创建UDP socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }

    // 设置服务器地址和端口
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(33334); // 与发送方相同的端口
    server_address.sin_addr.s_addr = INADDR_ANY; // 监听任何IP地址

    // 绑定socket到服务器地址和端口
    if (bind(sock, (struct sockaddr*)&server_address, sizeof(server_address)) < 0)
    {
        std::cerr << "Bind failed" << std::endl;
        close(sock);
        return -1;
    }

    std::cout << "Receiver is listening on port 12345..." << std::endl;

    // 创建接收和发送线程
    std::thread receiver_thread(UDPReceiver, sock, std::ref(client_address), std::ref(client_len));
    std::thread sender_thread(UDPSender, sock, std::ref(client_address), std::ref(client_len));

    // 等待发送线程结束
    sender_thread.join();
    // 发送线程结束后，通知接收线程也应该结束
    running = false;
    // 等待接收线程结束
    receiver_thread.join();

    return 0;
}
