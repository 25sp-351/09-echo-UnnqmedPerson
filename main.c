#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define DEFAULT_PORT 8080
#define BUFFER_SIZE 1024

void *handleConnection(void *sock_fd_ptr) {
    int sock_fd = *((int *)sock_fd_ptr);
    free(sock_fd_ptr); // Free the allocated memory for the socket fd

    printf("Handling connection on %d\n", sock_fd);

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    while ((bytes_read = read(sock_fd, buffer, BUFFER_SIZE - 1)) > 0) {
        buffer[bytes_read] = '\0'; // Null-terminate for printing
        printf("Received: %s", buffer);

        // Echo back the same data
        if (write(sock_fd, buffer, bytes_read) < 0) {
            perror("write");
            break;
        }
    }

    if (bytes_read == 0) {
        printf("Client disconnected\n");
    } else if (bytes_read < 0) {
        perror("read");
    }

    close(sock_fd);
    printf("done with connection %d\n", sock_fd);
    return NULL;
}

int main(int argc, char *argv[]) {
    int port = DEFAULT_PORT;
    int socket_fd;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_len = sizeof(client_address);

    // Optional: Allow setting port from command line
    if (argc > 1) {
        port = atoi(argv[1]);
    }

    // Ignore SIGPIPE to avoid crashing if writing to a closed socket
    signal(SIGPIPE, SIG_IGN);

    // Create socket
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Set SO_REUSEADDR to allow quick restarts
    int opt = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Bind socket
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    if (bind(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(socket_fd, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", port);

    while (1) {
        pthread_t thread;
        int *client_fd_buf = malloc(sizeof(int));

        *client_fd_buf = accept(socket_fd, (struct sockaddr *)&client_address, &client_address_len);
        if (*client_fd_buf < 0) {
            perror("accept");
            free(client_fd_buf);
            continue;
        }

        printf("Accepted connection on %d\n", *client_fd_buf);

        if (pthread_create(&thread, NULL, handleConnection, (void *)client_fd_buf) != 0) {
            perror("pthread_create");
            close(*client_fd_buf);
            free(client_fd_buf);
            continue;
        }

        // Detach the thread so we don't need to join it
        pthread_detach(thread);
    }

    // Unreachable in this server loop
    close(socket_fd);
    return 0;
}
