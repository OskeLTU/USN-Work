#include <iostream>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <csignal>
#include <string>
#include <Windows.h>

#pragma comment(lib, "ws2_32.lib") 

int initialize_wsa() {
    WSADATA wsaData{};
    const int Start_up_result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (Start_up_result == 0) {
        std::cout << "Initialization successful!" << '\n';
    }
    else {
        std::cout << "ERROR initializing Winsock" << '\n';
        return 1;
    }
    return 0;
}

SOCKET create_socket(int domain, int type, int protocol) {
    SOCKET made_socket = socket(domain, type, protocol);
    if (made_socket == INVALID_SOCKET) {
        std::cout << "Error with socket creation" << '\n';
        return INVALID_SOCKET;
    }
    else {
        std::cout << "Socket " << made_socket << " created" << '\n';
        return made_socket;
    }
}

int close_socket(SOCKET n) {
    if (n != INVALID_SOCKET) {
        int check = closesocket(n);
        if (check == 0) {
            std::cout << n << " closed" << '\n';
        }
        else {
            std::cout << "ERROR with closing socket" << '\n';
            return 1;
        }
    }
    return 0;
}

int clean_up() {
    const int Cleanup_result = WSACleanup();
    if (Cleanup_result == 0) {
        std::cout << "Cleanup was successful!" << '\n';
    }
    else {
        std::cout << "ERROR with cleanup" << '\n';
        return 1;
    }
    return 0;
}

int connect_to_server(SOCKET n, sockaddr* addr, int addrlen) {
    int check_for_connect = connect(n, addr, addrlen);
    if (check_for_connect != 0) {
        std::cout << "Connection failed" << '\n';
        return 1;
    }
    else {
        std::cout << "Connection successful" << '\n';
    }
    return 0;
}

int send_message(SOCKET n, int flags) {
    std::string message;
    if (n != INVALID_SOCKET) {
        while (1) {
            std::cout << "Enter message (type 'exit' to quit): ";
            getline(std::cin, message);


            if (message == "exit") {
                std::cout << "Exiting message loop..." << '\n';
                break;
            }
            int msg_check = send(n, message.c_str(), message.size(), flags);

            if (msg_check == SOCKET_ERROR) {
                std::cout << "Error sending message: " << '\n';
                break;
            }
            else {
                std::cout << "Message sent: " << message << '\n';

            }
        }
    }
    return 0;
}

void sign_clean(int signal) {
    std::cout << "Received signal: " << signal << ", ending program." << '\n';
    clean_up();
    throw std::runtime_error("SIGNAL ERROR");
}

void set_client_parameters(sockaddr_in& clientService) {


    std::string ip;
    int port;

    std::cout << "Enter IP for server: ";
    std::cin >> ip;

    std::cout << "Enter port number: ";
    std::cin >> port;

   
    clientService.sin_family = AF_INET;
    clientService.sin_port = htons(port);
    
    if (inet_pton(clientService.sin_family, ip.c_str(), &clientService.sin_addr) <= 0) {
        std::cout << "invalid IP address: " << ip << '\n';
        exit(1);
    }
  
}

int main() { 
    
    std::signal(SIGINT, sign_clean);
    std::signal(SIGTERM, sign_clean);

    SetConsoleOutputCP(CP_UTF8);

    sockaddr_in clientService;
    set_client_parameters(clientService);

    initialize_wsa();

    SOCKET client_1 = create_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   

    connect_to_server(client_1, (SOCKADDR*)&clientService, sizeof(clientService));

    send_message(client_1, 0); 

    close_socket(client_1);
    clean_up();

    return 0;


}
