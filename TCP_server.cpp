
#include <iostream>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <csignal>
#include <Windows.h>
#include <fstream>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <sstream>

#pragma comment(lib, "ws2_32.lib")

std::string readFile(const std::string& filePath) {

    std::cout << "Attempting to open file: " << std::filesystem::absolute(filePath) << std::endl;

    std::ifstream file(filePath, std::ios::binary); 
    if (!file.is_open()) {
        
        std::cout << "Failed to open file: " << std::filesystem::absolute(filePath) << std::endl;
        return ""; 
    }
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

// Function to determine content type based on file extension
std::string content_type(const std::string& filePath) {
    if (filePath.ends_with(".html")) {
        return "text/html";
    }
    else if (filePath.ends_with(".css")) {
        return "text/css";
    }
    else if (filePath.ends_with(".js")) {
        return "application/javascript";
    }
    else if (filePath.ends_with(".jpg")) {
        return "image/jpeg";
    }
    else if (filePath.ends_with(".webp")) {
        return "image/webp";
    }
    else {
        return "application/octet-stream"; // Default for unknown types
    }

}

 //Function to handle HTTP requests

//void recive_client(SOCKET clientSocket) {
//    const int BUFFER_SIZE = 10240; // Buffer size for incoming data
//    char buffer[BUFFER_SIZE] = { 0 };
//
//    int recive_message = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
//    if (recive_message > 0) {
//        buffer[recive_message] = '\0';
//
//        // Parse the request
//        std::istringstream requestStream(buffer);
//        std::string method, path, protocol;
//        requestStream >> method >> path >> protocol;
//
//        std::string responseHeader, responseBody;
//        if (method == "GET") {
//            // Remove leading '/' from the path
//            if (path[0] == '/') path = path.substr(1);
//            if (path.empty()) path = "index.html"; // Default to "index.html"
//
//            responseBody = readFile(path); // Call readFile function
//            if (responseBody.empty()) {
//                // File not found
//                responseBody = "<html><h1>404 - File not found</h1></html>";
//                responseHeader = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: " +
//                    std::to_string(responseBody.size()) + "\r\n\r\n";
//            }
//            else {
//                // File found
//                std::string file_type = content_type(path);
//                responseHeader = "HTTP/1.1 200 OK\r\nContent-Type: " + file_type + "\r\nContent-Length: " +
//                    std::to_string(responseBody.size()) + "\r\n\r\n";
//            }
//        }
//        else if (method == "POST") {
//            // Parse headers
//            std::unordered_map<std::string, std::string> headers;
//            std::string line;
//            while (std::getline(requestStream, line) && line != "\r") {
//                size_t colon = line.find(':');
//                if (colon != std::string::npos) {
//                    std::string key = line.substr(0, colon);
//                    std::string value = line.substr(colon + 2); // Skip ": "
//                    headers[key] = value.erase(value.find_last_not_of("\r") + 1);
//                }
//            }
//
//            // Get POST data
//            size_t contentLength = 0;
//            if (headers.count("Content-Length")) {
//                contentLength = std::stoul(headers["Content-Length"]);
//            }
//
//            std::string body;
//            size_t headerEnd = std::string(buffer).find("\r\n\r\n");
//            if (headerEnd != std::string::npos) {
//                body = std::string(buffer).substr(headerEnd + 4, contentLength);
//            }
//
//            // Process POST data (example: echo back)
//            responseBody = "Received POST data:\n" + body;
//            responseHeader = "HTTP/1.1 200 OK\r\n"
//                "Content-Type: text/plain\r\n"
//                "Content-Length: " + std::to_string(responseBody.size()) + "\r\n\r\n";
//        }
//        else {
//            // Method not allowed
//            responseBody = "<html><h1>405 - Method Not Allowed</h1></html>";
//            responseHeader = "HTTP/1.1 405 Method Not Allowed\r\n"
//                "Content-Type: text/html\r\n"
//                "Content-Length: " + std::to_string(responseBody.size()) + "\r\n\r\n";
//        }
//
//        // Send response
//        std::string response = responseHeader + responseBody;
//        send(clientSocket, response.c_str(), response.size(), 0);
//    }
//    closesocket(clientSocket);
//}


void recive_client(SOCKET clientSocket) {
    const int BUFFER_SIZE = 10240; // Buffer size for incoming data
    char buffer[BUFFER_SIZE] = { 0 };

    int recive_message = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (recive_message > 0) {
        buffer[recive_message] = '\0';

        // Parse the HTTP request
        std::istringstream requestStream(buffer);
        std::string method, path, protocol;
        requestStream >> method >> path >> protocol;

        // Log the received request path
        std::cout << "Received request path: " << path << std::endl;

        std::string responseHeader, responseBody;

        if (method == "POST") {
            // Extract headers and calculate body offset
            size_t headerEnd = std::string(buffer).find("\r\n\r\n");
            std::string body;
            if (headerEnd != std::string::npos) {
                headerEnd += 4; // Skip past "\r\n\r\n"

                // Extract Content-Length header
                std::string rawHeaders(buffer, headerEnd);
                size_t contentLengthPos = rawHeaders.find("Content-Length: ");
                size_t contentLength = 0;
                if (contentLengthPos != std::string::npos) {
                    contentLengthPos += 16; // Length of "Content-Length: "
                    size_t contentLengthEnd = rawHeaders.find("\r\n", contentLengthPos);
                    contentLength = std::stoul(rawHeaders.substr(contentLengthPos, contentLengthEnd - contentLengthPos));
                }

                // Extract the body based on Content-Length
                if (contentLength > 0) {
                    body = std::string(buffer).substr(headerEnd, contentLength);
                }
            }

            // Log the extracted body
            std::cout << "Extracted POST body: " << body << std::endl;

            // Redirect to result.html with the POST data as query parameters
            std::string redirectUrl = "/result.html?" + body;

            // Log the redirect URL
            std::cout << "Redirecting to: " << redirectUrl << std::endl;

            responseHeader = "HTTP/1.1 302 Found\r\n"
                "Location: " + redirectUrl + "\r\n"
                "Content-Length: 0\r\n\r\n";
        

        

        }
        else if (method == "GET") {
            // Handle GET requests

            // Strip query parameters from the path
            size_t queryPos = path.find('?');
            if (queryPos != std::string::npos) {
                path = path.substr(0, queryPos); // Remove everything after "?"
            }

            if (path[0] == '/') path = path.substr(1); // Remove leading '/'
            if (path.empty()) path = "index.html";     // Default to index.html

            responseBody = readFile(path);
            if (responseBody.empty()) {
                responseBody = "<html><h1>404 - File Not Found</h1></html>";
                responseHeader = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: " +
                    std::to_string(responseBody.size()) + "\r\n\r\n";
            }
            else {
                std::string file_type = content_type(path);
                responseHeader = "HTTP/1.1 200 OK\r\nContent-Type: " + file_type + "\r\nContent-Length: " +
                    std::to_string(responseBody.size()) + "\r\n\r\n";
            }
        }
        else {
            // Unsupported HTTP method
            responseBody = "<html><h1>405 - Method Not Allowed</h1></html>";
            responseHeader = "HTTP/1.1 405 Method Not Allowed\r\nContent-Type: text/html\r\nContent-Length: " +
                std::to_string(responseBody.size()) + "\r\n\r\n";
        }

        // Send response back to the client
        std::string response = responseHeader + responseBody;
        send(clientSocket, response.c_str(), response.size(), 0);
    }
    closesocket(clientSocket);
}





int Error_reurner() {
    int Error_code = WSAGetLastError();
    char* Error_Message;

    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        0, Error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&Error_Message, 0, 0);

    std::cout << Error_Message << '\n';

    return Error_code;
}

void sign_clean(int signal) {
    std::cout << "resived signal: " << signal << ", ending program." << '\n';
    WSACleanup();
    exit(1);
}

int main() {
    std::signal(SIGINT, sign_clean);
    std::signal(SIGTERM, sign_clean);

    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "failed to initialize Winsock. Error: " << Error_reurner() << '\n';
        return 1;
    };

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cout << "Failed to create socket. Error: " << Error_reurner() << '\n';
        WSACleanup();
        return 1;
    }

    const char* ip = "172.29.77.71"; // ip is hard coded here
    int port = 8080; //port is hard coded here

    sockaddr_in serverAddr = { 0 };
    serverAddr.sin_family = AF_INET;
    if(inet_pton(serverAddr.sin_family, "172.29.77.71", &serverAddr.sin_addr) != 1) {
        std::cout << "Invalid Ip address, Error: " << Error_reurner() << '\n';
    };

    serverAddr.sin_port = htons(8080); 

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cout << "Failed to bind socket. Error: " << Error_reurner() << '\n';
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, 10) == SOCKET_ERROR) {
        std::cout << "Failed to listen on socket. Error: " << Error_reurner() << '\n';
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "listening with the current ip: " << ip <<  " and port: " << port << '\n';

    // Accept and handle client connections
    while (true) {
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket != INVALID_SOCKET) {
            recive_client(clientSocket);
        }
    }

    // Cleanup
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
