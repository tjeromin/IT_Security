#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// Ports to listen to
#define TCP_PORT 8559
#define UDP_PORT 8515

// Username
#define username "fajdYt2AZZ1s1b8V"

#define MAX_CONNECTIONS 10

#if __APPLE__
#define NOSIGNAL SO_NOSIGPIPE
#else
#define NOSIGNAL MSG_NOSIGNAL
#endif

// Store socket-client data
struct client_data {
    int client_fd;
    struct sockaddr_in client_socket;
};

// Handle all UDP-Requests
void udpServer() {
    struct server_data {
        int port, server_fd;
        struct sockaddr_in server_socket;
        struct timeval timeout;
    };
    // Init server-struct properly
    struct server_data udpServerData = {UDP_PORT, 0, {0, 0}};
    udpServerData.timeout.tv_sec = 40;
    udpServerData.timeout.tv_usec = 0;
    // create socket
    if ((udpServerData.server_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("Could not create socket!\n");
        return;
    }
    // Store even more data in socket-struct
    udpServerData.server_socket.sin_family = AF_INET;
    udpServerData.server_socket.sin_port = htons(udpServerData.port);
    udpServerData.server_socket.sin_addr.s_addr = htonl(INADDR_ANY);

    int optionValue = 1;
    setsockopt(udpServerData.server_fd, SOL_SOCKET, SO_REUSEADDR, &optionValue, sizeof optionValue);
    // Bind port
    if (bind(udpServerData.server_fd, (struct sockaddr*)&udpServerData.server_socket,
             sizeof(udpServerData.server_socket)) < 0) {
        printf("Could not bind socket!\n");
        return;
    }
    printf("UDP-Server is listening on %d!\n", udpServerData.port);

    char* answer = "The answer to everything is fl{PO-PEMXJgFm}";
    char* passwd = "fl{PO-PEMXJgFm}";
    char* wrongPasswd = "Wrong password, buddy!";
    printf("%s\n", answer);

    // Expect format: "fajdYt2AZZ1s1b8VNAME PASSWORD" to get the flag
    while (1) {
        char message[50] = "";
        struct client_data clientData = {0, {0, 0}};
        socklen_t client_len = sizeof(clientData.client_socket);
        recvfrom(udpServerData.server_fd, message, 49, MSG_WAITALL, (struct sockaddr*)&clientData.client_socket,
                 &client_len);

        // Get username
        char user[strlen(username) + 1];
        memcpy(user, &message[0], strlen(username));
        user[strlen(username)] = '\0';

        // Get password
        char password[strlen(passwd) + 1];
        memcpy(password, &message[strlen(username) + 1], strlen(passwd));
        password[strlen(passwd)] = '\0';

        // Check, if both are correct
        if (strncmp(user, username, strlen(username)) == 0) {
            if (strncmp(password, passwd, strlen(passwd)) == 0) {
                sendto(udpServerData.server_fd, (const char*)wrongPasswd, strlen(wrongPasswd), 0,
                       (struct sockaddr*)&clientData.client_socket, client_len);
            } else {
                sendto(udpServerData.server_fd, (const char*)answer, strlen(answer), 0,
                       (struct sockaddr*)&clientData.client_socket, client_len);
            }
        } else {
            sendto(udpServerData.server_fd, (const char*)wrongPasswd, strlen(wrongPasswd), 0,
                   (struct sockaddr*)&clientData.client_socket, client_len);
        }
    }
    close(udpServerData.server_fd);
}

// Handle the communication with the tcp-server
void tcpCommunication(struct client_data* clientData) {
    // Socket establishment failed
    if ((*clientData).client_fd < 0) {
        printf("Could not establish new connection\n");
        close((*clientData).client_fd);
        free(clientData);
        return;
    }

    char answer[50];
    snprintf(answer, 50, "%d %s", UDP_PORT, username);
    if (send((*clientData).client_fd, answer, strlen(answer), 0) < 0) {
        printf("Client write failed\n");
        close((*clientData).client_fd);
        free(clientData);
        return;
    }

    printf("Data echoed\n");
    close((*clientData).client_fd);
    free(clientData);
}

// Handle the TPC-server
void tcpServer() {
    struct server_data {
        int port, server_fd;
        struct sockaddr_in server_socket;
        struct timeval timeout;
    };
    // Init server-struct properly
    struct server_data tcpServerData = {TCP_PORT, 0, {0, 0}};
    tcpServerData.timeout.tv_sec = 40;
    tcpServerData.timeout.tv_usec = 0;
    // create socket
    if ((tcpServerData.server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Could not create socket!\n");
        return;
    }
    // Store even more data in socket-struct
    tcpServerData.server_socket.sin_family = AF_INET;
    tcpServerData.server_socket.sin_port = htons(tcpServerData.port);
    tcpServerData.server_socket.sin_addr.s_addr = htonl(INADDR_ANY);

    int optionValue = 1;
    setsockopt(tcpServerData.server_fd, SOL_SOCKET, SO_REUSEADDR, &optionValue, sizeof optionValue);
    // Bind port
    if (bind(tcpServerData.server_fd, (struct sockaddr*)&tcpServerData.server_socket,
             sizeof(tcpServerData.server_socket)) < 0) {
        printf("Could not bind socket!\n");
        return;
    }
    // Listen on socket
    if (listen(tcpServerData.server_fd, MAX_CONNECTIONS) < 0) {
        printf("Could not listen on socket!\n");
        return;
    }
    printf("TCP-Server is listening on %d!\n", tcpServerData.port);

    // Wait for incomming connections
    while (1) {
        struct client_data* clientData = (struct client_data*)malloc(sizeof(struct client_data));
        socklen_t client_len = sizeof((*clientData).client_socket);
        (*clientData).client_fd =
            accept(tcpServerData.server_fd, (struct sockaddr*)&((*clientData).client_socket), &client_len);
        setsockopt((*clientData).client_fd, IPPROTO_TCP, TCP_NODELAY, &optionValue, sizeof optionValue);
        setsockopt((*clientData).client_fd, SOL_SOCKET, NOSIGNAL, &optionValue, sizeof optionValue);
        // Set timeout options
        setsockopt((*clientData).client_fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&tcpServerData.timeout,
                   sizeof(tcpServerData.timeout));
        setsockopt((*clientData).client_fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&tcpServerData.timeout,
                   sizeof(tcpServerData.timeout));
        pthread_t thread;
        pthread_create(&thread, NULL, (void*)tcpCommunication, (void*)clientData);
    }

    close(tcpServerData.server_fd);
}

// Set everything up
int main() {
    pthread_t tcpThread;
    pthread_t udpThread;
    pthread_create(&tcpThread, NULL, (void*)tcpServer, NULL);
    pthread_create(&udpThread, NULL, (void*)udpServer, NULL);
    pthread_join(tcpThread, NULL);
    pthread_join(udpThread, NULL);
    return 0;
}