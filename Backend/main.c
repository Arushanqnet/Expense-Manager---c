#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "home.h"         // home.c for transaction insert route
#include "login.h"        // login.c for login & create account
#include "transactions.h" // transactions.c for fetching user transactions

#define PORT 8080
#define BUFFER_SIZE 4096

// Log in anavar-in user_id-ai store seyyum
static int g_logged_in_user_id = 0;

// Custom headers-udan response anuppum oru utility function
void send_response(int socket_fd, const char *status, const char *content_type, const char *body) {
    char response[BUFFER_SIZE];

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

    // 1. Socket-ai uruvaakkum
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 2. Bind seyyum
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 3. Kaettukondiruppom (listen)
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

        // 5. Request-ai padikkum
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytes_read = read(new_socket, buffer, BUFFER_SIZE - 1);
        if (bytes_read <= 0) {
            close(new_socket);
            continue;
        }

        // Vandha HTTP request-ai print seyyum
        printf("Received request:\n%s\n", buffer);

        // HTTP method-um path-um eduthukkum
        char method[8] = {0};
        char path[256] = {0};
        sscanf(buffer, "%s %s", method, path);

        // 6. CORS kaga OPTIONS (preflight) request-ai handle seyyum
        if (strcmp(method, "OPTIONS") == 0) {
            send_response(new_socket, "HTTP/1.1 200 OK", "text/plain", "");
            close(new_socket);
            continue;
        }

        // 7. Routing
        // Transaction insert seyyum
        if (strcmp(path, "/home") == 0 && strcmp(method, "POST") == 0) {
            if (g_logged_in_user_id == 0) {
                const char *msg = "HTTP/1.1 401 Unauthorized\r\n"
                                  "Access-Control-Allow-Origin: *\r\n"
                                  "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
                                  "Access-Control-Allow-Headers: Content-Type\r\n"
                                  "Content-Type: text/plain\r\n"
                                  "Content-Length: 22\r\n"
                                  "\r\n"
                                  "Please log in first.\n";
                write(new_socket, msg, strlen(msg));
                close(new_socket);
                continue;
            }
            char *dynamic_response = handle_home_request(buffer, g_logged_in_user_id);
            write(new_socket, dynamic_response, strlen(dynamic_response));
            free(dynamic_response);

        // Account create seyyum
        } else if (strcmp(path, "/create_account") == 0 && strcmp(method, "POST") == 0) {
            char *dynamic_response = handle_create_account_request(buffer);
            send_response(new_socket, "HTTP/1.1 200 OK", "text/plain", dynamic_response);
            free(dynamic_response);

        // Login seyyum
        } else if (strcmp(path, "/login") == 0 && strcmp(method, "POST") == 0) {
            int temp_user_id = 0;
            char *dynamic_response = handle_login_request(buffer, &temp_user_id);
            if (temp_user_id > 0) {
                g_logged_in_user_id = temp_user_id;
                printf("Set global user_id = %d\n", g_logged_in_user_id);
            }
            send_response(new_socket, "HTTP/1.1 200 OK", "text/plain", dynamic_response);
            free(dynamic_response);

        // Transactions-ai edukkum
        } else if (strcmp(path, "/transactions") == 0 && strcmp(method, "GET") == 0) {
            if (g_logged_in_user_id == 0) {
                // Log in seyyavillainaal, error response anuppum
                const char *msg = "HTTP/1.1 401 Unauthorized\r\n"
                                  "Access-Control-Allow-Origin: *\r\n"
                                  "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
                                  "Access-Control-Allow-Headers: Content-Type\r\n"
                                  "Content-Type: text/plain\r\n"
                                  "Content-Length: 22\r\n"
                                  "\r\n"
                                  "Please log in first.\n";
                write(new_socket, msg, strlen(msg));
                close(new_socket);
                continue;
            }

            char *transactions_json = handle_get_transactions_request(g_logged_in_user_id);
            send_response(new_socket, "HTTP/1.1 200 OK", "application/json", transactions_json);
            free(transactions_json);

        } else {
            // 404 Not Found
            send_response(new_socket, "HTTP/1.1 404 Not Found", "text/plain", "Not Found");
        }

        close(new_socket);
    }

    return 0;
}
