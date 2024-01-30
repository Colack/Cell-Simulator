#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib,"ws2_32.lib")

#define PORT 8888
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

typedef struct {
    SOCKET socket;
    char name[50];
} client_t;

client_t clients[MAX_CLIENTS];
int num_clients = 0;

void broadcast(char* message, int sender_index) {
    int i;
    for (i = 0; i < num_clients; i++) {
        if (i != sender_index) {
            send(clients[i].socket, message, strlen(message), 0);
        }
    }
}

void kick(int index) {
    closesocket(clients[index].socket);
    int i;
    for (i = index; i < num_clients - 1; i++) {
        clients[i] = clients[i+1];
    }
    num_clients--;
}

void list(int index) {
    int i;
    char message[50];
    sprintf(message, "There are %d clients connected:\n", num_clients);
    send(clients[index].socket, message, strlen(message), 0);
    for (i = 0; i < num_clients; i++) {
        sprintf(message, "%d. %s\n", i+1, clients[i].name);
        send(clients[index].socket, message, strlen(message), 0);
    }
}

void ban(int index) {
    //TODO: Implement ban functionality
}

int main() {
    WSADATA wsa;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server, client;
    char buffer[BUFFER_SIZE];

    // Read from a file called "config.txt"
    FILE* f = fopen("config.txt", "r");
    if (f == NULL) {
        // If the file doesn't exist, create it
        f = fopen("config.txt", "w");
        fprintf(f, "Welcome to the chat room!\n");
        fclose(f);
        f = fopen("config.txt", "r");
    }

    // Read the welcome message from the file
    char welcome_message[BUFFER_SIZE];
    fgets(welcome_message, BUFFER_SIZE, f);
    fclose(f);

    // Make the welcome message "Server: <welcome_message>"
    char server_welcome_message[BUFFER_SIZE];
    // Make it a string
    printf("Welcome message: %s", welcome_message);
    sprintf(server_welcome_message, "Server: %s", welcome_message);

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code : %d", WSAGetLastError());
        return 1;
    }

    // Create a socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket : %d", WSAGetLastError());
        return 1;
    }

    // Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    // Bind the socket to the address and port
    if (bind(server_socket, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Bind failed with error code : %d", WSAGetLastError());
        return 1;
    }

    // Listen for incoming connections
    listen(server_socket, 3);

    printf("Waiting for incoming connections...\n");

    // Accept incoming connections
    int c = sizeof(struct sockaddr_in);
    while ((client_socket = accept(server_socket, (struct sockaddr*)&client, &c)) != INVALID_SOCKET) {
        printf("Connection accepted from %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

        // Add the client to the list
        if (num_clients < MAX_CLIENTS) {
            clients[num_clients].socket = client_socket;
            sprintf(clients[num_clients].name, "Client%d", num_clients+1);
            num_clients++;
        } else {
            printf("Max number of clients reached\n");
            closesocket(client_socket);
        }

        char message[50];
        sprintf(message, "Welcome to the chatroom, %s\n", clients[num_clients-1].name);
        send(client_socket, welcome_message, strlen(message), 0);

        int index = num_clients-1;
        int bytes_received;
        do {
            bytes_received = recv(clients[index].socket, buffer, BUFFER_SIZE, 0);
            if (bytes_received > 0) {
                buffer[bytes_received] = '\0';
                if (strncmp(buffer, "/list", 5) == 0) {
                    list(index);
                } else if (strncmp(buffer, "/kick", 5) == 0) {
                    int client_index = atoi(buffer+5)-1;
                    if (client_index >= 0 && client_index < num_clients && client_index != index) {
                        char kick_message[50];
                        sprintf(kick_message, "%s has been kicked from the chatroom.\n", clients[client_index].name);
                        broadcast(kick_message, -1);
                        kick(client_index);
                    } else {
                        char error_message[50];
                        sprintf(error_message, "Invalid client index\n");
                        send(clients[index].socket, error_message, strlen(error_message), 0);
                    }
                } else if (strncmp(buffer, "/ban", 4) == 0) {
                    //TODO: Implement ban functionality
                } else {
                    char message[BUFFER_SIZE+50];
                    sprintf(message, "[%s]: %s", clients[index].name, buffer);
                    broadcast(message, index);
                }
            }
        } while (bytes_received > 0);

        // Client disconnected
        printf("Client %s disconnected\n", clients[index].name);
        char disconnect_message[50];
        sprintf(disconnect_message, "%s has left the chatroom.\n", clients[index].name);
        broadcast(disconnect_message, -1);
        kick(index);
    }

    closesocket(server_socket);
    WSACleanup();

    return 0;
}
