#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 1024

// Function prototypes
void handle_client_connection(const char *server_ip, int port, int client_id);
int create_socket();
void setup_server_address(struct sockaddr_in *server_addr, const char *server_ip, int port);
void send_request(int sockfd);
void receive_response(int sockfd, int client_id);

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <IP address> <Port> <Number of clients>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *server_ip = argv[1];
    int port = atoi(argv[2]);
    int num_clients = atoi(argv[3]);

    for (int i = 0; i < num_clients; i++) {
        handle_client_connection(server_ip, port, i + 1);
    }

    return 0;
}


void handle_client_connection(const char *server_ip, int port, int client_id) {
    int sockfd = create_socket();
    if (sockfd < 0) {
        return;
    }

    struct sockaddr_in server_addr;
    setup_server_address(&server_addr, server_ip, port);

    // Connect to server
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sockfd);
        return;
    }

    send_request(sockfd);
    receive_response(sockfd, client_id);

    close(sockfd);
}

int create_socket() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
    }
    return sockfd;
}


void setup_server_address(struct sockaddr_in *server_addr, const char *server_ip, int port) {
    memset(server_addr, 0, sizeof(*server_addr));
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(port);

 
    if (inet_pton(AF_INET, server_ip, &server_addr->sin_addr) <= 0) {
        perror("Invalid IP address or IP address not supported");
        exit(EXIT_FAILURE);
    }
}


void send_request(int sockfd) {
    const char *request = "GET_CPU_INFO";
    send(sockfd, request, strlen(request), 0);
}


void receive_response(int sockfd, int client_id) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    int bytes_received = recv(sockfd, buffer, BUFFER_SIZE, 0);
    if (bytes_received > 0) {
        printf("Client %d received:\n%s\n", client_id, buffer);
    } else if (bytes_received == 0) {
        printf("Server closed connection\n");
    } else {
        perror("recv");
    }
}
