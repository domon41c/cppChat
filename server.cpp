#include <iostream>
#include <string>
#include <WS2tcpip.h>
#include <thread>
#include <vector>
#include <algorithm> // For using std::find

#define PORT 8080
#define BUFFER_SIZE 1024

struct ClientInfo {
    SOCKET socket;
    std::string nickname;
};

std::vector<ClientInfo> clients;

void handleClient(SOCKET clientSocket) {
    char buffer[BUFFER_SIZE];
    int valread;

    // Receive client's nickname
    valread = recv(clientSocket, buffer, BUFFER_SIZE, 0);
    if (valread > 0) {
        buffer[valread] = '\0';
        std::string nickname = buffer;

        // Store client's nickname and socket
        clients.push_back({clientSocket, nickname});
    }

    // Receive and broadcast messages
    while ((valread = recv(clientSocket, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[valread] = '\0';
        std::string message = buffer;

        // Find client's nickname
        std::string clientNickname;
        for (const auto& client : clients) {
            if (client.socket == clientSocket) {
                clientNickname = client.nickname;
                break;
            }
        }

        // Concatenate nickname and message
        std::string fullMessage = clientNickname + ": " + message;

        // Broadcast the message to all other clients
        for (const auto& client : clients) {
            if (client.socket != clientSocket) {
                send(client.socket, fullMessage.c_str(), fullMessage.length(), 0);
            }
        }
    }

    // Remove client from the list of connected clients
    auto it = std::find_if(clients.begin(), clients.end(), [clientSocket](const ClientInfo& client) {
        return client.socket == clientSocket;
    });
    if (it != clients.end()) {
        clients.erase(it);
    }

    closesocket(clientSocket);
}

int main() {
    WSADATA wsData;
    if (WSAStartup(MAKEWORD(2, 2), &wsData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return -1;
    }

    SOCKET serverSocket;
    sockaddr_in serverAddr;

    // Create socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        std::cerr << "Failed to create socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return -1;
    }

    // Prepare the sockaddr_in structure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    // Bind
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed with error code: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return -1;
    }

    // Listen
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed with error code: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return -1;
    }

    std::cout << "Waiting for connections..." << std::endl;

    SOCKET clientSocket;
    sockaddr_in clientAddr;
    int clientAddrSize = sizeof(clientAddr);

    // Accept incoming connections
    while ((clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize)) != INVALID_SOCKET) {
        std::cout << "Connection established with client" << std::endl;

        // Create a thread to handle the client
        std::thread clientThread(handleClient, clientSocket);
        clientThread.detach(); // Detach the thread to allow it to run independently
    }

    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
