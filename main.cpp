#include <iostream>
#include <string>
#include <WS2tcpip.h>
#include <thread>

#define PORT 8080
#define BUFFER_SIZE 1024

void readMessages(SOCKET clientSocket) {
    char buffer[BUFFER_SIZE];
    int valread;

    // Receive messages from server
    while ((valread = recv(clientSocket, buffer, BUFFER_SIZE, 0)) > 0) {
        std::cout << buffer << std::endl;
    }
}

int main() {
    WSADATA wsData;
    if (WSAStartup(MAKEWORD(2, 2), &wsData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return -1;
    }

    SOCKET clientSocket;
    sockaddr_in serverAddr;
    char buffer[BUFFER_SIZE] = {0};

    // Create socket
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        std::cerr << "Socket creation error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return -1;
    }

    // Prepare the sockaddr_in structure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "192.0.0.1", &serverAddr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return -1;
    }

    // Connect to server
    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed with error code: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return -1;
    }

    std::cout << "Connected to server" << std::endl;

    // Get user's nickname
    std::string nickname;
    std::cout << "Enter your nickname: ";
    std::getline(std::cin, nickname);

    // Send nickname to server
    send(clientSocket, nickname.c_str(), nickname.length(), 0);

    // Start a thread to read messages from the server
    std::thread readThread(readMessages, clientSocket);
    readThread.detach(); // Detach the thread to allow it to run independently

    // Main loop for sending messages
    std::string message;
    while (true) {
        std::getline(std::cin, message);
        send(clientSocket, message.c_str(), message.length(), 0);
    }

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
