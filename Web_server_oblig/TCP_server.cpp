
#include <iostream>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <csignal>
#include <Windows.h>
#include <fstream>
#include <filesystem>
#include <string>
#include <sstream>

#pragma comment(lib, "ws2_32.lib")


int Error_reurner() {
    int Error_code = WSAGetLastError();
    char* Error_Message;

    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        0, Error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&Error_Message, 0, 0);

    std::cout << Error_Message << '\n';

    return Error_code;
}

std::string readFile(const std::string& filePath) {

    std::cout << "Attempting to open file: " << std::filesystem::absolute(filePath) << std::endl;

    std::ifstream file(filePath, std::ios::binary); 
    if (!file.is_open()) {
        
        std::cout << "Failed to open file: " << std::filesystem::absolute(filePath) << std::endl;
        return ""; //show where the atmept to pene the file was made, and that it failed.
    }
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

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
	else if (filePath.ends_with(".png")) {
		return "image/png";
	}
    else {
        return "application/octet-stream"; // Default for unknown types
    }

}

void recive_client(SOCKET clientSocket) {
    const int BUFFER_SIZE = 10240; // Buffer size for incoming data
    char buffer[BUFFER_SIZE] = { 0 };

    int recive_message = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (recive_message > 0) {
        buffer[recive_message] = '\0';

        std::istringstream request_stream_from_website(buffer);
        std::string method, path, protocol;
        request_stream_from_website >> method >> path >> protocol; 

        
        std::cout << "Received request path: " << path << std::endl;//for troubleshooting

        std::string response_header, response_body;

        if (method == "POST") {
         
            size_t header_end = std::string(buffer).find("\r\n\r\n");
            std::string body;
            if (header_end != std::string::npos) {
                header_end += 4; // 4 is addded to hopp over the header to the body of the file

                std::string header_from_client(buffer, header_end);
                size_t content_length_pos = header_from_client.find("Content-Length: ");
                size_t contentLength = 0;
                if (content_length_pos != std::string::npos) {
					content_length_pos += 16; //16 is added here to hopp over the characters in the word "Content-Length: "
                    size_t contentLengthEnd = header_from_client.find("\r\n", content_length_pos);
                    contentLength = std::stoul(header_from_client.substr(content_length_pos, contentLengthEnd - content_length_pos));
                }

                // Extract the body based on Content-Length
                body = std::string(buffer).substr(header_end, contentLength);
            }

            std::string redirectUrl = "/result.html?" + body;//save the post in the url

            response_header = "HTTP/1.1 302 Found\r\n"
                "Location: " + redirectUrl + "\r\n"
                "Content-Length: 0\r\n\r\n";
        }
        else if (method == "GET") {
            

            
            size_t queryPos = path.find('?');//searches for a ? and then uses that what is before it as the path
            if (queryPos != std::string::npos) {
                path = path.substr(0, queryPos); // Remove everything after "?"
            }

            if (path[0] == '/') path = path.substr(1); 
            if (path.empty()) path = "index.html";// directs to index.html if nothing is spesified after port or a "/" is left
            /*if (!path.empty()) path = "egen_fil.png";*/
            response_body = readFile(path);
            if (response_body.empty()) {
                response_body = "<html><h1>404 - File Not Found</h1></html>";
                response_header = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: " +
                    std::to_string(response_body.size()) + "\r\n\r\n";
            }
            else {
                std::string file_type = content_type(path);
                response_header = "HTTP/1.1 200 OK\r\nContent-Type: " + file_type + "\r\nContent-Length: " +
                    std::to_string(response_body.size()) + "\r\n\r\n";
            }
        }
        else {
			std::cout << "error occured. Error: " << Error_reurner() << '\n';
			WSACleanup();
			exit(1);
        }

        // Send response back to the client
        std::string response = response_header + response_body;
        send(clientSocket, response.c_str(), response.size(), 0);
    }
    closesocket(clientSocket);
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

    const char* ip = "10.0.0.22"; 
    int port = 8080; 

    sockaddr_in serverAddr = { 0 };
    serverAddr.sin_family = AF_INET;
    if(inet_pton(serverAddr.sin_family, "10.0.0.22", &serverAddr.sin_addr) != 1) {
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

   
    while (true) {
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket != INVALID_SOCKET) {
            recive_client(clientSocket);
        }
    }

    
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
