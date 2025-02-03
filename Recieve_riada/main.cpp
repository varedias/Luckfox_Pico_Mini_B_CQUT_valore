#include <iostream>
#include <winsock2.h>
#include <vector>
#include <string>
#include <sstream>  // ���� stringstream ��
#include <algorithm>  // �����㷨��

#pragma comment(lib, "ws2_32.lib")  // Winsock ��

#define SERVER_IP "192.168.37.33"  // �����IP��ַ
#define SERVER_PORT 8082           // ����˶˿�
#define BUFFER_SIZE 1024           // ���ݻ�������С
#define EXPECTED_DATA_SIZE 10000   // �������յ������ݴ�С�����磺100*100�ֽڣ�

// ���ڼ�����룬��λΪ���ף�ֱ��ȡint��Ϊ������úܾ�ȷ
int Distance(char byte) {
    // ֱ��ʹ���ַ���Ϊʮ�����������������
    int decimalNum = static_cast<unsigned char>(byte);  // ֱ�ӽ��ַ�ת��Ϊ�޷�������

    return (decimalNum / 5.1) * (decimalNum / 5.1);  // �������
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
            // ���ԭʼ���ݵ����ֽ�����ԭʼ����
            std::cout << "Received raw data total bytes: " << received_data.size() << std::endl;
//            std::cout << "Received raw data: ";
//            for (size_t i = 0; i < received_data.size(); ++i) {
//                printf("0x%02X ", static_cast<unsigned char>(received_data[i]));
//            }
            std::cout << std::endl;

            // ���㲢������ݶ�Ӧ�ľ���
            std::vector<int> distances;
            for (char byte : received_data) {
                int distance = Distance(byte);  // ��ÿ���ֽڼ������
                if (distance != -1) {  // �����������
                    distances.push_back(distance);  // �洢����
                }
            }

            // ���������������
            std::cout << "Calculated distances count: " << distances.size() << std::endl;

            // ���������
            if (!distances.empty()) {
                std::cout << "Calculated distances: ";
                for (int distance : distances) {
                    std::cout << distance << " ";
                }
                std::cout << std::endl;
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
