#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

// Socket maximum size
#define BUFFER_SIZE 128
#define MAX_CONNECTIONS 10
#define PORT 8001
#define MAX_TRIALS_PER_CONNECTION 5

#if __APPLE__
#define NOSIGNAL SO_NOSIGPIPE
#else
#define NOSIGNAL MSG_NOSIGNAL
#endif

int generate_super_fancy_password() {
    return rand();  // Returns a pseudo-random integer between 0 and RAND_MAX.
}

struct client_data {
    struct sockaddr_in client_socket;
    socklen_t client_len;
    int client_fd;
};

struct server_data {
    struct sockaddr_in server_socket;
    int server_fd;
    struct timeval timeout;
};

// Handle all client-server-communication in this function
void communicate(struct client_data* clientData) {
    uint8_t counter = 0;

    char secret_pwd_of_the_day[64];
    char secret_msg[] = "fl{EN-PTtlxonw}";

    char msg_to_send[BUFFER_SIZE];
    char received_msg[BUFFER_SIZE];

    if ((*clientData).client_fd < 0) {
        free(clientData);
        return;
    }
    // send clear instructions to the client
    int len = sprintf(msg_to_send,
                      "Welcome to CheapEnigma. Use your secret dictionary to find the password of the day. Please "
                      "input the password:");
    int err = send((*clientData).client_fd, msg_to_send, len, NOSIGNAL);
    if (err < 0) {
        free(clientData);
        return;
    }
    // generate secret password of the day, copy it into secret_pwd_of_the_day, keep its length in pwd_len
    int pwd_len = sprintf(secret_pwd_of_the_day, "%i", generate_super_fancy_password());
    // don't forget to reset counter
    counter = 0;

    while (1) {
        memset(received_msg, 0, BUFFER_SIZE);
        // receive password
        int numbytes = recv((*clientData).client_fd, received_msg, BUFFER_SIZE, 0);
        if (numbytes < 0) {
            break;
        }
        // increase tries counter
        ++counter;
        // how many time did this user tried today?
        if (counter > MAX_TRIALS_PER_CONNECTION) {
            len = sprintf(msg_to_send, "You tried too many times! The password of the day was %s. Come back tomorrow!",
                          secret_pwd_of_the_day);
        } else if (strcmp(received_msg, secret_pwd_of_the_day) == 0) {
            // the password is correct
            len = sprintf(msg_to_send, "Correct! Secret message of the day: %s", secret_msg);
        } else {
            len = sprintf(msg_to_send,
                          "Incorrect! You tried %i out of %i times today. Input the correct password:", counter,
                          MAX_TRIALS_PER_CONNECTION);
        }
        err = send((*clientData).client_fd, msg_to_send, len, NOSIGNAL);
        if (err < 0) {
            break;
        }
    }
    close((*clientData).client_fd);
    free(clientData);
    printf("%s\n", "Socket closed");
}

int main(int argc, char* argv[]) {
    struct server_data serverData;

    // Set timeouts
    serverData.timeout.tv_sec = 40;  // Timeout after 40 secs
    serverData.timeout.tv_usec = 0;

    int err;
    int len = 0;

    // Randomize random()
    srand(time(NULL));

    // Open a socket
    serverData.server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverData.server_fd < 0) {
        printf("Could not create socket.\n");
        return -1;
    }

    // Store important socket data in server_data struct
    serverData.server_socket.sin_family = AF_INET;
    serverData.server_socket.sin_port = htons(PORT);
    serverData.server_socket.sin_addr.s_addr = htonl(INADDR_ANY);

    int opt_val = 1;
    setsockopt(serverData.server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);

    // Bind port
    err = bind(serverData.server_fd, (struct sockaddr*)&(serverData.server_socket), sizeof(serverData.server_socket));
    if (err < 0) {
        printf("Could not bind socket\n");
        return -1;
    }

    // Listen on opened socket
    err = listen(serverData.server_fd, MAX_CONNECTIONS);
    if (err < 0) {
        printf("Could not listen on socket\n");
        return -1;
    }

    printf("Server is listening on %d\n", PORT);

    // Handle all incomming connections for ever...
    while (1) {
        // Allocate a new struct containing all client data for using it in a thread
        struct client_data* clientData = (struct client_data*)malloc(sizeof(struct client_data));
        (*clientData).client_len = sizeof((*clientData).client_socket);
        // Wait for an incomming connection
        (*clientData).client_fd =
            accept(serverData.server_fd, (struct sockaddr*)&(*clientData).client_socket, &(*clientData).client_len);

        // Set socket-options for this connection
        setsockopt((*clientData).client_fd, IPPROTO_TCP, TCP_NODELAY, &opt_val, sizeof opt_val);
        setsockopt((*clientData).client_fd, SOL_SOCKET, NOSIGNAL, &opt_val, sizeof opt_val);
        setsockopt((*clientData).client_fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&serverData.timeout,
                   sizeof(serverData.timeout));
        setsockopt((*clientData).client_fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&serverData.timeout,
                   sizeof(serverData.timeout));
        // Create a new thread to handle client-server communication
        pthread_t thread;
        pthread_create(&thread, NULL, (void*)communicate, (void*)clientData);
    }
    // Close the open socket and exit the program
    close(serverData.server_fd);
    return 0;
}