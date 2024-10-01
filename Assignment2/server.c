#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <dirent.h>

#define PORT 8080
#define BACKLOG 10
#define MAX_BUFFER 1024

// Structure to store process information
struct process_info {
    char name[256];
    int pid;
    long user_time;
    long kernel_time;
};

// Function to get process information from the stat file
int read_process_stat(const char *proc_path, struct process_info *proc_info) {
    FILE *file = fopen(proc_path, "r");
    if (!file) {
        return -1;
    }

    // Read process information from the stat file
    fscanf(file, "%d %s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu",
           &proc_info->pid, proc_info->name, &proc_info->user_time, &proc_info->kernel_time);
    fclose(file);
    return 0;
}

// Function to update the top two CPU-consuming processes
void update_top_processes(struct process_info *top_procs, struct process_info *new_proc) {
    long total_time_new = new_proc->user_time + new_proc->kernel_time;
    long total_time_top1 = top_procs[0].user_time + top_procs[0].kernel_time;
    long total_time_top2 = top_procs[1].user_time + top_procs[1].kernel_time;

    if (total_time_new > total_time_top1) {
        top_procs[1] = top_procs[0];  
        top_procs[0] = *new_proc;     
    } else if (total_time_new > total_time_top2) {
        top_procs[1] = *new_proc;     
    }
}

// Function to get the top two CPU-consuming processes from /proc
void get_top_cpu_processes(struct process_info processes[2]) {
    DIR *dir;
    struct dirent *entry;
    char proc_path[256];
    struct process_info temp_proc;

   
    processes[0].user_time = processes[1].user_time = 0;

    if ((dir = opendir("/proc")) != NULL) {
        while ((entry = readdir(dir)) != NULL) {
            if (isdigit(entry->d_name[0])) {
                snprintf(proc_path, sizeof(proc_path), "/proc/%s/stat", entry->d_name);
                if (read_process_stat(proc_path, &temp_proc) == 0) {
                    update_top_processes(processes, &temp_proc);
                }
            }
        }
        closedir(dir);
    }
}

// Function to send process information to the client
void send_process_info(int client_sock, struct process_info processes[2]) {
    char send_buffer[MAX_BUFFER];

    // Format the process information into a buffer
    snprintf(send_buffer, sizeof(send_buffer),
             "Top CPU-consuming process 1:\nPID: %d\nName: %s\nUser time: %ld\nKernel time: %ld\n\n"
             "Top CPU-consuming process 2:\nPID: %d\nName: %s\nUser time: %ld\nKernel time: %ld\n",
             processes[0].pid, processes[0].name, processes[0].user_time, processes[0].kernel_time,
             processes[1].pid, processes[1].name, processes[1].user_time, processes[1].kernel_time);

  
    send(client_sock, send_buffer, strlen(send_buffer), 0);
}

// Function to handle each client connection
void *handle_client(void *client_socket) {
    int new_sock = *((int *)client_socket);
    free(client_socket);  

    struct process_info processes[2];
    get_top_cpu_processes(processes);  

    send_process_info(new_sock, processes);  
    sleep(100);  // just to show concurrent connections

    close(new_sock);
    return NULL;
}

// Main server initialization function
int initialize_server(struct sockaddr_in *server_addr) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(PORT);
    server_addr->sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr *)server_addr, sizeof(*server_addr)) == -1) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, BACKLOG) == -1) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);
    return server_fd;
}

int main() {
    int server_fd, new_sock, *client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);

    // Initialize and start the server
    server_fd = initialize_server(&server_addr);

    // Accept and handle clients in concurrent threads
    while (1) {
        new_sock = accept(server_fd, (struct sockaddr *)&client_addr, &addr_size);
        if (new_sock == -1) {
            perror("Accept failed");
            continue;
        }

        // Allocate memory for client socket and create a thread
        client_sock = malloc(sizeof(int));
        *client_sock = new_sock;
        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_client, (void *)client_sock) != 0) {
            perror("Thread creation failed");
            free(client_sock);
        }

        pthread_detach(tid); 
    }

    close(server_fd);
    return 0;
}
