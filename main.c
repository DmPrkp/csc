#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <sys/stat.h>
#include <fcntl.h>

#pragma comment(lib, "ws2_32.lib") // Link with ws2_32.lib

#define PORT 8090
#define WEB_ROOT "./webroot"

void handle_request(SOCKET client_socket) {
    char buffer[1024] = {0};
    int valread = recv(client_socket, buffer, sizeof(buffer), 0);
    
    if (valread <= 0) {
        printf("Error reading request\n");
        return;
    }
    
    // Parse HTTP request
    char method[10], path[256];
    sscanf(buffer, "%s %s", method, path);
    
    // Default to index.html if no specific file is requested
    if (strcmp(path, "/") == 0) {
        strcpy(path, "/index.html");
    }
    
    // Construct full path to file in web root directory
    char full_path[256];
    snprintf(full_path, sizeof(full_path), "%s%s", WEB_ROOT, path);
    
    // Open requested file
    FILE *file = fopen(full_path, "rb");
    if (!file) {
        printf("File not found: %s\n", full_path);
        const char *not_found_response =
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/html\r\n\r\n"
            "<html><body><h1>test</h1></body></html>";
        send(client_socket, not_found_response, strlen(not_found_response), 0);
        return;
    }
    
    // Send HTTP response headers
    const char *response_headers =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n\r\n";
    send(client_socket, response_headers, strlen(response_headers), 0);
    
    // Send file content
    while ((valread = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        send(client_socket, buffer, valread, 0);
    }
    
    // Clean up
    fclose(file);
}

int main() {
    WSADATA wsaData;
    SOCKET server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }
    
    // Create server socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Socket creation failed\n");
        WSACleanup();
        return 1;
    }
    
    // Set socket options
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) == SOCKET_ERROR) {
        printf("Set socket options failed\n");
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }
    
    // Bind socket to port 8090
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR) {
        printf("Bind failed\n");
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }
    
    // Listen for incoming connections
    if (listen(server_fd, 3) == SOCKET_ERROR) {
        printf("Listen failed\n");
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }
    
    printf("Server listening on port %d...\n", PORT);
    
    while (1) {
        // Accept incoming connection
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) == INVALID_SOCKET) {
            printf("Accept failed\n");
            closesocket(server_fd);
            WSACleanup();
            return 1;
        }
        
        // Handle request
        handle_request(client_socket);
        
        // Close client socket
        closesocket(client_socket);
    }
    
    // Clean up
    closesocket(server_fd);
    WSACleanup();
    return 0;
}
