//server
#include <iostream>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <csignal>
#include <Windows.h>
#include <fstream>
#include <sstream>


#pragma comment(lib, "ws2_32.lib")

//server /////////////////////////////////////////////////////////////



int initialize_wsa() {
	WSADATA wsaData{};

	//initialazation part start->
	const int Start_up_result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (Start_up_result == 0) {
		std::cout << "initialization succsesfull!" << '\n';
		return 0;
	}
	else {
		std::cout << "ERROR" << '\n';
		return 1;
	}
	//->initialazation part end
};

int clean_up() {
	//cleanup part start ->

	const int Cleanup_result = WSACleanup();
	if (Cleanup_result == 0) {
		std::cout << "cleanup was succsesfull!" << '\n';
		return 0;
	}
	else {
		std::cout << "ERROR with cleanup" << '\n';
		return 1;
	}

	//->cleanup part end 
}

SOCKET create_socket(int domain, int type, int protocol) {

	SOCKET made_socket = socket(domain, type, protocol);
	if (made_socket == INVALID_SOCKET) {
		std::cout << "error with socket creation" << '\n';
		return INVALID_SOCKET;
	}
	else {
		std::cout << "socket " << made_socket << "  created" << '\n';
		return made_socket;
	}
};

int close_socket(SOCKET n) {
	//closing sockets start

	int check = closesocket(n);
	if (check == 0) {
		std::cout << n << "  closed" << '\n';
	}
	else {
		std::cout << "ERROR with closing" << '\n';
		return 1;
	}

	//closing sockets end
}

int bind_socket(SOCKET n, sockaddr_in server) {
	int error_handling = bind(n, (SOCKADDR*)&server, sizeof(server));
	if (error_handling == 0) {
		std::cout << "socket " << n << " bound" << '\n';
	}
	else {
		std::cout << "error with binding" << '\n';
		return 1;
	};
}

int listen_func(SOCKET n, int backlog) {
	int check = listen(n, backlog);
	if (check == 0) {
		std::cout << "listen succsesful" << '\n';
		std::cout << "waiting for connection... " << '\n';
	}
	else {
		std::cout << "ERROR with listening" << '\n';
		return 1;
	};
}

SOCKET accept_func(SOCKET n, sockaddr* addr, int* addrlen) {
	SOCKET socket_obj;
	socket_obj = accept(n, addr, addrlen);
	if (socket_obj == INVALID_SOCKET) {
		std::cout << "accept failiure" << '\n';
		return INVALID_SOCKET;
	}
	else {
		std::cout << "accept succesfull" << '\n';

		return socket_obj;
	}
	
}

bool is_client_connected(SOCKET client_socket) {
	char buffer;
	int result = recv(client_socket, &buffer, 1, MSG_PEEK);

	if (result > 0) {
		return true;  // Client is still connected
	}
	else if (result == 0) {
		std::cout << "Client disconnected gracefully.\n";
		return false;  // Client disconnected gracefully
	}
	else {
		std::cout << "Network error: " << WSAGetLastError() << '\n';
		return false;  // Network error or wrong error
	}
}

void receive_the_message(SOCKET client_socket) {
	char buffer[1024] = { 0 };

	while (is_client_connected(client_socket)) {
		int message = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
		if (message > 0) {
			buffer[message] = '\0';
			std::cout << "Received message:\n" << buffer << '\n';

			// Parse the HTTP request
			std::istringstream requestStream(buffer);
			std::string method, path, protocol;
			requestStream >> method >> path >> protocol;

			if (method == "GET") {
				// Remove leading '/' from the path
				if (path[0] == '/') path = path.substr(1);

				// Open and read the file
				std::ifstream file(path);
				std::ostringstream responseStream;

				if (file) {
					// HTTP response header
					responseStream << "HTTP/1.1 200 OK\r\n";
					responseStream << "Content-Type: text/html\r\n"; // Adjust content type as needed
					responseStream << "Connection: close\r\n\r\n";

					// File content
					responseStream << file.rdbuf();
				}
				else {
					// File not found
					responseStream << "HTTP/1.1 404 Not Found\r\n";
					responseStream << "Connection: close\r\n\r\n";
					responseStream << "<html><body><h1>404 Not Found</h1></body></html>";
				}

				// Send the response
				std::string response = responseStream.str();
				send(client_socket, response.c_str(), response.size(), 0);
			}
			else {
				// Method not supported
				std::string response = "HTTP/1.1 405 Method Not Allowed\r\nConnection: close\r\n\r\n";
				send(client_socket, response.c_str(), response.size(), 0);
			}
		}
		else if (message == 0) {
			std::cout << "Client disconnected.\n";
			break;
		}
		else {
			std::cout << "Network error.\n";
			break;
		}
		memset(buffer, 0, 1023);
	}
}

//void receive_the_message(SOCKET client_socket) {
//	char buffer[1024] = { 0 };
//	while (is_client_connected(client_socket)) {
//		
//		int message = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
//		if (message > 0) {
//			std::cout << "received message: " << buffer << '\n';
//			buffer[message] = '\0';
//
//		}
//		else if (message == 0) {
//			std::cout << "client disconnected" << '\n';
//			break;
//		}
//		else {
//			std::cout << "error with network" << '\n';
//			break;
//		};	
//		memset(buffer, 0, 1023);
//	}
//}

void sign_clean(int signal){
	std::cout << "resived signal: " << signal << ", ending program." << '\n';
	clean_up();
	exit(1);
}

sockaddr_in set_server_parameters() {
	sockaddr_in server{};
	std::string ip;
	int port;

	std::cout << "Enter ip for server: ";
	std::cin >> ip;

	std::cout << "Enter Port number: ";
	std::cin >> port;

	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	if (inet_pton(server.sin_family, ip.c_str(), &server.sin_addr) <= 0) {
		std::cout << "invalid IP address: " << ip << '\n';
		exit(1);
	}

	return server;
}

//server////////////////////////////////////////////////////////////


int main() {
	std::signal(SIGINT, sign_clean);
	std::signal(SIGTERM, sign_clean);

	SetConsoleOutputCP(CP_UTF8);
	

	sockaddr_in server = set_server_parameters();

	initialize_wsa();

	SOCKET server_1 = create_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	bind_socket(server_1, server);

	listen_func(server_1, 10);

	SOCKET client_socket = accept_func(server_1, 0, 0);

	receive_the_message(client_socket);
	
	
	closesocket(server_1);
	clean_up();



	return 0;
}





