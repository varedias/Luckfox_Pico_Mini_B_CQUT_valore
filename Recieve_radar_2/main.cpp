#include <iostream>
#include <winsock2.h>
#include <vector>
#include <string>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")  // Winsock 库

#define SERVER_IP "192.168.37.33"  // 服务端IP地址
#define SERVER_PORT 8082           // 服务端端口
#define BUFFER_SIZE 1024           // 数据缓冲区大小
#define EXPECTED_DATA_SIZE 10022   // 期望接收到的数据大小（包括包头、包长度等）

// 用于计算距离，单位为毫米，直接取int因为结果不用很精确
int Distance(char byte) {
    int decimalNum = static_cast<unsigned char>(byte);  // 直接将字符转换为无符号整型
    return (decimalNum / 5.1) * (decimalNum / 5.1);  // 计算距离
}

// 避障函数，判断是否有障碍物
bool AvoidObstacle(const std::vector<std::vector<int>>& distances, int threshold) {
    const int rows = 100;
    const int cols = 100;

    const int horizontal_parts = 4;
    const int vertical_parts = 3;

    const int horizontal_part_size = cols / horizontal_parts;
    const int vertical_part_size = rows / vertical_parts;

    // 遍历每个分区
    for (int v = 0; v < vertical_parts; ++v) {
        for (int h = 0; h < horizontal_parts; ++h) {
            std::vector<int> part_distances;

            // 收集分区内的所有数据
            for (int i = v * vertical_part_size; i < (v + 1) * vertical_part_size; ++i) {
                for (int j = h * horizontal_part_size; j < (h + 1) * horizontal_part_size; ++j) {
                    part_distances.push_back(distances[i][j]);
                }
            }

            // 排序并去掉最小和最大的一部分数据（比如去掉10%的数据）
            std::sort(part_distances.begin(), part_distances.end());

            // 移除异常值去掉最小和最大 10%
            int trim_start = part_distances.size() * 0.1;
            int trim_end = part_distances.size() * 0.9;

            int total_distance = 0;
            int count = 0;
            for (int i = trim_start; i < trim_end; ++i) {
                total_distance += part_distances[i];
                count++;
            }

            // 计算去除异常值后的平均值
            int average_distance = total_distance / count;

            // 如果平均距离小于阈值，则认为该分区有障碍物
            if (average_distance < threshold) {
                std::cout << "Obstacle detected in part (" << v << ", " << h << ") with avg distance: " << average_distance << " mm" << std::endl;
                return true;  // 有障碍物
            }
        }
    }

    return false;  // 无障碍物
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

    // 定义一个 100x100 的二维向量distances，用来存储数据
    const int rows = 100;
    const int cols = 100;
    std::vector<std::vector<int>> distances(rows, std::vector<int>(cols, 0));

    // 避障阈值（单位：毫米）
    const int obstacle_threshold = 500;  // 示例阈值，可以根据实际情况调整

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
            // 输出原始数据的总字节数
            std::cout << "Received raw data total bytes: " << received_data.size() << std::endl;

            // 输出前 20 个字节的原始数据（以十六进制显示）
            std::cout << "First 30 bytes of received data (hex): ";
            for (size_t i = 0; i < 10; ++i) {
                printf("0x%02X ", static_cast<unsigned char>(received_data[i]));
            }
            std::cout << std::endl;

            // 获取包长度（2字节）
            unsigned short package_length = (static_cast<unsigned char>(received_data[2]) << 8) |
                                            static_cast<unsigned char>(received_data[3]);

            // 图像帧数据开始位置：16字节后
            size_t image_frame_start = 4 + 16;  // 包头（2字节）+ 包长度（2字节）+ 其他内容（16字节）

            // 提取图像帧数据
            std::string image_frame_data = received_data.substr(image_frame_start, package_length - 16 - 3); // 减去包头（2字节）和包尾（1字节）

            // 输出图像帧的前20个字节（十六进制）
            std::cout << "First 20 bytes of image frame (hex): ";
            for (size_t i = 0;  i<20&&i < image_frame_data.size(); ++i) {
                printf("0x%02X ", static_cast<unsigned char>(image_frame_data[i]));
            }
            std::cout << std::endl;

            // 计算并存储图像帧的数据到二维向量,这里面已经有了所有的距离数据
            size_t index = 0;
            for (int i = 0; i < rows; ++i) {
                for (int j = 0; j < cols; ++j) {
                    if (index < image_frame_data.size()) {
                        // 计算每个字节的距离并存入二维向量
                        distances[i][j] = Distance(image_frame_data[index]);
                        ++index;
                    }
                }
            }

            // 进行避障判断
            if (AvoidObstacle(distances, obstacle_threshold)) {
                std::cout << "Obstacle detected! Taking evasive action..." << std::endl;
                // 在这里添加避障策略，例如停止、转向等
            } else {
                std::cout << "No obstacle detected. Continuing..." << std::endl;
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
