#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// Socket maximum size
#define BUFFER_SIZE 16
#define MAX_CONNECTIONS 10
#define PORT 8000

#if __APPLE__
#define NOSIGNAL SO_NOSIGPIPE
#else
#define NOSIGNAL MSG_NOSIGNAL
#endif

// Store socket-server data
struct server_data {
    int port;
    // server file descriptor
    int server_fd;
    struct sockaddr_in server_socket;
    // Timeouts for connection
    struct timeval timeout;
};

// Store socket-client data
struct client_data {
    int client_fd;
    struct sockaddr_in client_socket;
    // where we store received data
    char buffer[BUFFER_SIZE];
    // Our local flag variable
    char flag[60];
};

// Do communication with the client-socket
void communicate(struct client_data* clientData) {
    // Socket establishment failed
    if ((*clientData).client_fd < 0) {
        printf("Could not establish new connection\n");
        close((*clientData).client_fd);
        free(clientData);
        return;
    }

    // The client first sends how many bytes its message will be
    memset((*clientData).buffer, 0, BUFFER_SIZE);
    int numbytes = recv((*clientData).client_fd, (*clientData).buffer, BUFFER_SIZE, 0);
    if (!numbytes || numbytes < 0) {
        printf("Client read failed\n");
        close((*clientData).client_fd);
        free(clientData);
        return;
    }
    int nb_of_char = atoi((*clientData).buffer);
	if (nb_of_char > BUFFER_SIZE) {
		nb_of_char = BUFFER_SIZE;
	}
    printf("Client will send %i bytes.\n", nb_of_char);

    memset((*clientData).buffer, 0, BUFFER_SIZE);
    // receive actual data
    numbytes = recv((*clientData).client_fd, (*clientData).buffer, BUFFER_SIZE, 0);
    if (!numbytes || numbytes < 0) {
        printf("Client read failed\n");
        close((*clientData).client_fd);
        free(clientData);
        return;
    }
    printf("Received data from client! Sending back\n");

    // echo back data
    int err = send((*clientData).client_fd, &(*clientData).buffer, nb_of_char, 0);
    if (err < 0) {
        printf("Client write failed\n");
        close((*clientData).client_fd);
        free(clientData);
        return;
    }
    printf("Data echoed\n");
    close((*clientData).client_fd);
    free(clientData);
}

int main(int argc, char* argv[]) {
    struct server_data serverData = {PORT, 0, 0, 0};
    serverData.timeout.tv_sec = 40;  // Timeout after 40 secs
    serverData.timeout.tv_usec = 0;

    int err;

    // create socket
    serverData.server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverData.server_fd < 0) {
        printf("Could not create socket.\n");
        return -1;
    }
    // Store even more data in socket-struct
    serverData.server_socket.sin_family = AF_INET;
    serverData.server_socket.sin_port = htons(serverData.port);
    serverData.server_socket.sin_addr.s_addr = htonl(INADDR_ANY);

    int opt_val = 1;
    setsockopt(serverData.server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);

    // Bind port
    err = bind(serverData.server_fd, (struct sockaddr*)&serverData.server_socket, sizeof(serverData.server_socket));
    if (err < 0) {
        printf("Could not bind socket\n");
        return -1;
    }

    // Listen to this port
    err = listen(serverData.server_fd, MAX_CONNECTIONS);
    if (err < 0) {
        printf("Could not listen on socket\n");
        return -1;
    }
    printf("Server is listening on %d\n", serverData.port);

    while (1) {
        // Allocate a new struct for the client-data on the heap
        struct client_data* clientData = (struct client_data*)malloc(sizeof(struct client_data));
        socklen_t client_len = sizeof((*clientData).client_socket);
        strcpy((*clientData).flag, "fl{EC-LhUOtjyN}");
        (*clientData).client_fd =
            accept(serverData.server_fd, (struct sockaddr*)&((*clientData).client_socket), &client_len);
        setsockopt((*clientData).client_fd, IPPROTO_TCP, TCP_NODELAY, &opt_val, sizeof opt_val);
        setsockopt((*clientData).client_fd, SOL_SOCKET, NOSIGNAL, &opt_val, sizeof opt_val);
        // Set timeout options
        setsockopt((*clientData).client_fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&serverData.timeout,
                   sizeof(serverData.timeout));
        setsockopt((*clientData).client_fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&serverData.timeout,
                   sizeof(serverData.timeout));
        pthread_t thread;
        pthread_create(&thread, NULL, (void*)communicate, (void*)clientData);
    }
    close(serverData.server_fd);
    return 0;
}
