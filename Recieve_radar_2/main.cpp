#include <iostream>
#include <winsock2.h>
#include <vector>
#include <string>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")  // Winsock ��

#define SERVER_IP "192.168.37.33"  // �����IP��ַ
#define SERVER_PORT 8082           // ����˶˿�
#define BUFFER_SIZE 1024           // ���ݻ�������С
#define EXPECTED_DATA_SIZE 10022   // �������յ������ݴ�С��������ͷ�������ȵȣ�

// ���ڼ�����룬��λΪ���ף�ֱ��ȡint��Ϊ������úܾ�ȷ
int Distance(char byte) {
    int decimalNum = static_cast<unsigned char>(byte);  // ֱ�ӽ��ַ�ת��Ϊ�޷�������
    return (decimalNum / 5.1) * (decimalNum / 5.1);  // �������
}

// ���Ϻ������ж��Ƿ����ϰ���
bool AvoidObstacle(const std::vector<std::vector<int>>& distances, int threshold) {
    const int rows = 100;
    const int cols = 100;

    const int horizontal_parts = 4;
    const int vertical_parts = 3;

    const int horizontal_part_size = cols / horizontal_parts;
    const int vertical_part_size = rows / vertical_parts;

    // ����ÿ������
    for (int v = 0; v < vertical_parts; ++v) {
        for (int h = 0; h < horizontal_parts; ++h) {
            std::vector<int> part_distances;

            // �ռ������ڵ���������
            for (int i = v * vertical_part_size; i < (v + 1) * vertical_part_size; ++i) {
                for (int j = h * horizontal_part_size; j < (h + 1) * horizontal_part_size; ++j) {
                    part_distances.push_back(distances[i][j]);
                }
            }

            // ����ȥ����С������һ�������ݣ�����ȥ��10%�����ݣ�
            std::sort(part_distances.begin(), part_distances.end());

            // �Ƴ��쳣ֵȥ����С����� 10%
            int trim_start = part_distances.size() * 0.1;
            int trim_end = part_distances.size() * 0.9;

            int total_distance = 0;
            int count = 0;
            for (int i = trim_start; i < trim_end; ++i) {
                total_distance += part_distances[i];
                count++;
            }

            // ����ȥ���쳣ֵ���ƽ��ֵ
            int average_distance = total_distance / count;

            // ���ƽ������С����ֵ������Ϊ�÷������ϰ���
            if (average_distance < threshold) {
                std::cout << "Obstacle detected in part (" << v << ", " << h << ") with avg distance: " << average_distance << " mm" << std::endl;
                return true;  // ���ϰ���
            }
        }
    }

    return false;  // ���ϰ���
}

int main() {
    WSADATA wsaData;
    SOCKET client_socket;
    struct sockaddr_in server_addr;

    // ��ʼ�� Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed. Error Code: " << WSAGetLastError() << std::endl;
        return 1;
    }

    // �����׽���
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        std::cerr << "Socket creation failed. Error Code: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // ���÷���˵�ַ
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // ���ӵ������
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed. Error Code: " << WSAGetLastError() << std::endl;
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to server at " << SERVER_IP << ":" << SERVER_PORT << std::endl;

    char buffer[BUFFER_SIZE];
    std::string received_data;

    // ����һ�� 100x100 �Ķ�ά����distances�������洢����
    const int rows = 100;
    const int cols = 100;
    std::vector<std::vector<int>> distances(rows, std::vector<int>(cols, 0));

    // ������ֵ����λ�����ף�
    const int obstacle_threshold = 500;  // ʾ����ֵ�����Ը���ʵ���������

    while (true) {
        // ��ս��ջ�����
        received_data.clear();

        // ��������ֱ�����յ��㹻������
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

        // ������յ����������Ѿ��㹻
        if (received_data.size() >= EXPECTED_DATA_SIZE) {
            // ���ԭʼ���ݵ����ֽ���
            std::cout << "Received raw data total bytes: " << received_data.size() << std::endl;

            // ���ǰ 20 ���ֽڵ�ԭʼ���ݣ���ʮ��������ʾ��
            std::cout << "First 30 bytes of received data (hex): ";
            for (size_t i = 0; i < 10; ++i) {
                printf("0x%02X ", static_cast<unsigned char>(received_data[i]));
            }
            std::cout << std::endl;

            // ��ȡ�����ȣ�2�ֽڣ�
            unsigned short package_length = (static_cast<unsigned char>(received_data[2]) << 8) |
                                            static_cast<unsigned char>(received_data[3]);

            // ͼ��֡���ݿ�ʼλ�ã�16�ֽں�
            size_t image_frame_start = 4 + 16;  // ��ͷ��2�ֽڣ�+ �����ȣ�2�ֽڣ�+ �������ݣ�16�ֽڣ�

            // ��ȡͼ��֡����
            std::string image_frame_data = received_data.substr(image_frame_start, package_length - 16 - 3); // ��ȥ��ͷ��2�ֽڣ��Ͱ�β��1�ֽڣ�

            // ���ͼ��֡��ǰ20���ֽڣ�ʮ�����ƣ�
            std::cout << "First 20 bytes of image frame (hex): ";
            for (size_t i = 0;  i<20&&i < image_frame_data.size(); ++i) {
                printf("0x%02X ", static_cast<unsigned char>(image_frame_data[i]));
            }
            std::cout << std::endl;

            // ���㲢�洢ͼ��֡�����ݵ���ά����,�������Ѿ��������еľ�������
            size_t index = 0;
            for (int i = 0; i < rows; ++i) {
                for (int j = 0; j < cols; ++j) {
                    if (index < image_frame_data.size()) {
                        // ����ÿ���ֽڵľ��벢�����ά����
                        distances[i][j] = Distance(image_frame_data[index]);
                        ++index;
                    }
                }
            }

            // ���б����ж�
            if (AvoidObstacle(distances, obstacle_threshold)) {
                std::cout << "Obstacle detected! Taking evasive action..." << std::endl;
                // ��������ӱ��ϲ��ԣ�����ֹͣ��ת���
            } else {
                std::cout << "No obstacle detected. Continuing..." << std::endl;
            }

            // ��ջ��棬׼�������µ�����
            received_data.clear();

        } else {
            std::cerr << "Received data size less than expected, something went wrong!" << std::endl;
            break;
        }
    }

    // �ر��׽���
    closesocket(client_socket);

    // ���� Winsock
    WSACleanup();

    return 0;
}
