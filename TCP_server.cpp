

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <csignal>
#include <Windows.h>
#include <fstream>
#include <filesystem>
#include <string>

#pragma comment(lib, "ws2_32.lib")

// Function to read the contents of a file
std::string readFile(const std::string& filePath) {
    // Log the full path being accessed
    std::cout << "Attempting to open file: " << std::filesystem::absolute(filePath) << std::endl;

    std::ifstream file(filePath, std::ios::binary); // Open in binary mode for non-text files
    if (!file.is_open()) {
        // Log an error if the file cannot be opened
        std::cerr << "Failed to open file: " << std::filesystem::absolute(filePath) << std::endl;
        return ""; // Return empty string to indicate failure
    }
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

// Function to determine content type based on file extension
std::string getContentType(const std::string& filePath) {
    if (filePath.ends_with(".html") || filePath.ends_with(".htm")) {
        return "text/html";
    }
    else if (filePath.ends_with(".css")) {
        return "text/css";
    }
    else if (filePath.ends_with(".js")) {
        return "application/javascript";
    }
    else if (filePath.ends_with(".jpg") || filePath.ends_with(".jpeg")) {
        return "image/jpeg";
    }
    else if (filePath.ends_with(".png")) {
        return "image/png";
    }
    else if (filePath.ends_with(".gif")) {
        return "image/gif";
    }
    else if (filePath.ends_with(".ico")) {
        return "image/x-icon";
    }
    else {
        return "application/octet-stream"; // Default for unknown types
    }
}

// Function to handle HTTP requests
void handleClientRequest(SOCKET clientSocket) {
    const int BUFFER_SIZE = 10240; // Buffer size for incoming data
    char buffer[BUFFER_SIZE] = { 0 };

    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';

        // Parse the request
        std::istringstream requestStream(buffer);
        std::string method, path, protocol;
        requestStream >> method >> path >> protocol;

        std::string responseHeader, responseBody;
        if (method == "GET") {
            // Remove leading '/' from the path
            if (path[0] == '/') path = path.substr(1);
            if (path.empty()) path = "index.html"; // Default to "index.html"

            responseBody = readFile(path); // Call readFile function
            if (responseBody.empty()) {
                // File not found
                responseBody = "<html><h1>404 - File not found</h1></html>";
                responseHeader = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: " +
                    std::to_string(responseBody.size()) + "\r\n\r\n";
            }
            else {
                // File found
                std::string contentType = getContentType(path);
                responseHeader = "HTTP/1.1 200 OK\r\nContent-Type: " + contentType + "\r\nContent-Length: " +
                    std::to_string(responseBody.size()) + "\r\n\r\n";
            }
        }
        else {
            // Method not allowed
            responseBody = "<html><h1>405 - Method Not Allowed</h1></html>";
            responseHeader = "HTTP/1.1 405 Method Not Allowed\r\nContent-Type: text/html\r\nContent-Length: " +
                std::to_string(responseBody.size()) + "\r\n\r\n";
        }

        // Send the response
        std::string response = responseHeader + responseBody;
        send(clientSocket, response.c_str(), response.size(), 0);
    }

    closesocket(clientSocket); // Close the client socket
}

int main() {
    // Initialize Winsock
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // Create and bind the server socket
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in serverAddr = { 0 };
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("10.0.0.22"); // Update with your IP
    serverAddr.sin_port = htons(8080);                  // Update with your desired port
    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));

    // Listen for connections
    listen(serverSocket, 10);

    std::cout << "Server is listening on port 8080..." << std::endl;

    // Accept and handle client connections
    while (true) {
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket != INVALID_SOCKET) {
            handleClientRequest(clientSocket);
        }
    }

    // Cleanup
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
