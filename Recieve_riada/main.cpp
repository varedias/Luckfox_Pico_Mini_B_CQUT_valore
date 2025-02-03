#include <iostream>
#include <winsock2.h>
#include <vector>
#include <string>
#include <sstream>  // 引入 stringstream 库
#include <algorithm>  // 引入算法库

#pragma comment(lib, "ws2_32.lib")  // Winsock 库

#define SERVER_IP "192.168.37.33"  // 服务端IP地址
#define SERVER_PORT 8082           // 服务端端口
#define BUFFER_SIZE 1024           // 数据缓冲区大小
#define EXPECTED_DATA_SIZE 10000   // 期望接收到的数据大小（例如：100*100字节）

// 用于计算距离，单位为毫米，直接取int因为结果不用很精确
int Distance(char byte) {
    // 直接使用字符作为十六进制数来计算距离
    int decimalNum = static_cast<unsigned char>(byte);  // 直接将字符转换为无符号整型

    return (decimalNum / 5.1) * (decimalNum / 5.1);  // 计算距离
}

int main() {
    WSADATA wsaData;
    SOCKET client_socket;
    struct sockaddr_in server_addr;

    // 初始化 Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed. Error Code: " << WSAGetLastError() << std::endl;
        return 1;
    }

    // 创建套接字
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        std::cerr << "Socket creation failed. Error Code: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // 配置服务端地址
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // 连接到服务端
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed. Error Code: " << WSAGetLastError() << std::endl;
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to server at " << SERVER_IP << ":" << SERVER_PORT << std::endl;

    char buffer[BUFFER_SIZE];
    std::string received_data;

    while (true) {
        // 清空接收缓冲区
        received_data.clear();

        // 接收数据直到接收到足够的数据
        while (received_data.size() < EXPECTED_DATA_SIZE) {
            int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
            if (bytes_received > 0) {
                received_data.append(buffer, bytes_received);
            } else if (bytes_received == 0) {
                std::cout << "Server disconnected" << std::endl;
                break;
            } else {
                std::cerr << "Receive failed. Error Code: " << WSAGetLastError() << std::endl;
                break;
            }
        }

        // 如果接收到的数据量已经足够
        if (received_data.size() >= EXPECTED_DATA_SIZE) {
            // 输出原始数据的总字节数和原始数据
            std::cout << "Received raw data total bytes: " << received_data.size() << std::endl;
//            std::cout << "Received raw data: ";
//            for (size_t i = 0; i < received_data.size(); ++i) {
//                printf("0x%02X ", static_cast<unsigned char>(received_data[i]));
//            }
            std::cout << std::endl;

            // 计算并输出数据对应的距离
            std::vector<int> distances;
            for (char byte : received_data) {
                int distance = Distance(byte);  // 对每个字节计算距离
                if (distance != -1) {  // 如果计算正常
                    distances.push_back(distance);  // 存储距离
                }
            }

            // 输出计算结果的总数
            std::cout << "Calculated distances count: " << distances.size() << std::endl;

            // 输出计算结果
            if (!distances.empty()) {
                std::cout << "Calculated distances: ";
                for (int distance : distances) {
                    std::cout << distance << " ";
                }
                std::cout << std::endl;
            }

            // 清空缓存，准备接收新的数据
            received_data.clear();
        } else {
            std::cerr << "Received data size less than expected, something went wrong!" << std::endl;
            break;
        }
    }

    // 关闭套接字
    closesocket(client_socket);

    // 清理 Winsock
    WSACleanup();

    return 0;
}
