#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "home.h"   // Existing home.c for transaction routes
#include "login.h"  // New login.c for login & create account

#define PORT 8080
#define BUFFER_SIZE 4096

// Utility function to send a response with custom headers
void send_response(int socket_fd, const char *status, const char *content_type, const char *body) {
    char response[BUFFER_SIZE];

    // Construct HTTP response headers.
    // Add CORS headers so the browser permits cross-origin requests.
    snprintf(response, sizeof(response),
        "%s\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "\r\n"
        "%s",
        status, content_type, strlen(body), body);

    write(socket_fd, response, strlen(response));
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // 1. Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 2. Bind
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 3. Listen
    if (listen(server_fd, 5) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // 4. Main loop
    while (1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (new_socket < 0) {
            perror("accept");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        // 5. Read request
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytes_read = read(new_socket, buffer, BUFFER_SIZE - 1);
        if (bytes_read <= 0) {
            close(new_socket);
            continue;
        }

        // Print the incoming HTTP request
        printf("Received request:\n%s\n", buffer);

        // Extract HTTP method and path from the request line
        // e.g. "POST /home HTTP/1.1\r\n"
        char method[8] = {0};
        char path[256] = {0};
        sscanf(buffer, "%s %s", method, path);

        // 6. Handle OPTIONS (preflight) requests for CORS
        if (strcmp(method, "OPTIONS") == 0) {
            // Just send back OK with the CORS headers
            send_response(new_socket, "HTTP/1.1 200 OK", "text/plain", "");
            close(new_socket);
            continue;
        }

        // 7. Routing
        if (strcmp(path, "/home") == 0 && strcmp(method, "POST") == 0) {
            // Existing /home route from home.c
            char *dynamic_response = handle_home_request(buffer);
            send_response(new_socket, "HTTP/1.1 200 OK", "text/plain", dynamic_response);
            free(dynamic_response);

        } else if (strcmp(path, "/create_account") == 0 && strcmp(method, "POST") == 0) {
            // New route for account creation
            char *dynamic_response = handle_create_account_request(buffer);
            send_response(new_socket, "HTTP/1.1 200 OK", "text/plain", dynamic_response);
            free(dynamic_response);

        } else if (strcmp(path, "/login") == 0 && strcmp(method, "POST") == 0) {
            // New route for user login
            char *dynamic_response = handle_login_request(buffer);
            send_response(new_socket, "HTTP/1.1 200 OK", "text/plain", dynamic_response);
            free(dynamic_response);

        } else {
            // 404 Not Found
            send_response(new_socket, "HTTP/1.1 404 Not Found", "text/plain", "Not Found");
        }

        // Close client socket
        close(new_socket);
    }

    return 0;
}
