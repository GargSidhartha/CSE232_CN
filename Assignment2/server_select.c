#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/select.h>
#include <errno.h>

#define PORT 8080
#define BACKLOG 10
#define MAX_BUFFER 1024
#define MAX_CLIENTS 100

// Structure to store process info
struct process_info {
    char name[256];
    int pid;
    long user_time;
    long kernel_time;
};

// Function to get top 2 CPU-consuming processes
void get_top_cpu_processes(struct process_info processes[2]) {
    DIR *dir;
    struct dirent *entry;
    char path[256], buffer[1024];
    FILE *file;
    struct process_info temp_proc;
    long total_time;
    processes[0].user_time = processes[1].user_time = 0;

    if ((dir = opendir("/proc")) != NULL) {
        while ((entry = readdir(dir)) != NULL) {
            if (isdigit(entry->d_name[0])) {
                snprintf(path, sizeof(path), "/proc/%s/stat", entry->d_name);  
                file = fopen(path, "r");
                if (file) {
                    fscanf(file, "%d %s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu",
                           &temp_proc.pid, temp_proc.name, &temp_proc.user_time, &temp_proc.kernel_time);
                    fclose(file);
                    total_time = temp_proc.user_time + temp_proc.kernel_time;

                    if (total_time > processes[0].user_time + processes[0].kernel_time) {
                        processes[1] = processes[0];
                        processes[0] = temp_proc;
                    } else if (total_time > processes[1].user_time + processes[1].kernel_time) {
                        processes[1] = temp_proc;
                    }
                }
            }
        }
        closedir(dir);
    }
}

// Function to handle each client request
void handle_client(int client_sock) {
    struct process_info processes[2];
    get_top_cpu_processes(processes);

    char send_buffer[MAX_BUFFER];
    snprintf(send_buffer, sizeof(send_buffer),
             "Top CPU-consuming process 1:\nPID: %d\nName: %s\nUser time: %ld\nKernel time: %ld\n\n"
             "Top CPU-consuming process 2:\nPID: %d\nName: %s\nUser time: %ld\nKernel time: %ld\n",
             processes[0].pid, processes[0].name, processes[0].user_time, processes[0].kernel_time,
             processes[1].pid, processes[1].name, processes[1].user_time, processes[1].kernel_time);

    send(client_sock, send_buffer, strlen(send_buffer), 0);
}

// Main server function
int main() {
    int server_fd, new_sock, client_sock, max_sd, sd, activity;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    int client_sockets[MAX_CLIENTS] = {0};  

    fd_set read_fds;

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket to the port
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, BACKLOG) == -1) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

   
    while (1) {
        
        FD_ZERO(&read_fds);

        FD_SET(server_fd, &read_fds);
        max_sd = server_fd;

        
        for (int i = 0; i < MAX_CLIENTS; i++) {
            sd = client_sockets[i];

            
            if (sd > 0) {
                FD_SET(sd, &read_fds);
            }

            if (sd > max_sd) {
                max_sd = sd;
            }
        }

        
        activity = select(max_sd + 1, &read_fds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR)) {
            perror("select error");
        }

        
        if (FD_ISSET(server_fd, &read_fds)) {
            new_sock = accept(server_fd, (struct sockaddr *)&client_addr, &addr_size);
            if (new_sock == -1) {
                perror("Accept failed");
                exit(EXIT_FAILURE);
            }

            printf("New connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_sockets[i] == 0) {
                    client_sockets[i] = new_sock;
                    printf("Added to client list at index %d\n", i);
                    break;
                }
            }
        }

        
        for (int i = 0; i < MAX_CLIENTS; i++) {
            sd = client_sockets[i];

            if (FD_ISSET(sd, &read_fds)) {
                
                char buffer[MAX_BUFFER];
                int bytes_read = read(sd, buffer, sizeof(buffer));
                if (bytes_read == 0) {
                    getpeername(sd, (struct sockaddr *)&client_addr, &addr_size);
                    printf("Client disconnected: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                    close(sd);
                    client_sockets[i] = 0; 
                } else {
                    
                    buffer[bytes_read] = '\0';
                    printf("Received request from client %d: %s\n", i, buffer);

                    
                    handle_client(sd);
                }
            }
        }
    }


    close(server_fd);
    return 0;
}
