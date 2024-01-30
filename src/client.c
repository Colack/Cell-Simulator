#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUFFER_SIZE 1024

int main(int argc, char* argv[], char* username[])
{
    if (argc < 3) {
        printf("Usage: %s <server IP address> <port>\n", argv[0]);
        return 1;
    }

    // Initialize Winsock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    // Create a socket for the client
    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_socket == INVALID_SOCKET) {
        printf("socket creation failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Resolve the server address and port
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(atoi(argv[2]));

    // Connect to the server
    // Send the username to the server
    iResult = connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (iResult == SOCKET_ERROR) {
        printf("connection failed: %d\n", WSAGetLastError());
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    iResult = connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (iResult == SOCKET_ERROR) {
        printf("connection failed: %d\n", WSAGetLastError());
        closesocket(client_socket);
        WSACleanup();
        return 1; 
    }

    // Receive welcome message from the server
    char buffer[BUFFER_SIZE];
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        printf("%s", buffer);
    }

    // Send messages to the server
    do {
        printf("> ");
        fgets(buffer, BUFFER_SIZE, stdin);
        int len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n') {
            buffer[len-1] = '\0';
        }
        if (strcmp(buffer, "/quit") == 0) {
            break;
        }
        iResult = send(client_socket, buffer, strlen(buffer), 0);
        if (iResult == SOCKET_ERROR) {
            printf("send failed: %d\n", WSAGetLastError());
            closesocket(client_socket);
            WSACleanup();
            return 1;
        }
    } while (1);

    // Clean up and exit
    closesocket(client_socket);
    WSACleanup();

    return 0;
}
