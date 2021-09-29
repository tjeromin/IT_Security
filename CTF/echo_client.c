#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <string.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <unistd.h>

    #define PORT 8000    /* the port client will be connecting to */
    #define BUFFER_SIZE 512 /* max number of bytes we can get at once */

int main(int argc, char *argv[])
{
    int sockfd, numbytes;  
    char buffer[BUFFER_SIZE];
    struct hostent *he;
        struct sockaddr_in server; /* connector's address information */

    if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        return -1;
    }

    if ((he=gethostbyname(argv[1])) == NULL) {
        printf("Could not get IP address from input\n");
        return -1;
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("Cannot open socket\n");
        return -1;
    }

    server.sin_family = AF_INET;      /* host byte order */
    server.sin_port = htons(PORT);    /* short, network byte order */
    server.sin_addr = *((struct in_addr *)he->h_addr);
    bzero(&(server.sin_zero), 8);     /* zero the rest of the struct */

    if (connect(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1) {
        printf("Couldn't connect to distant server\n");
        return -1;
    }

    printf("Please input the number of characters your message will be (as an integer, max 15):\n");
    int nb_of_chars = 0;
    scanf("%i", &nb_of_chars);
    int len = sprintf(buffer, "%i", nb_of_chars);
    // sending to server
    if (send(sockfd, buffer, len+1, 0) == -1){
      printf("Could not send message\n");
      return -1;
    }
    nb_of_chars +=1;

    printf("Please input your message (max size: %i):\n", nb_of_chars-1);
    char message[BUFFER_SIZE];
    scanf("%s", message);
    int msg_len = strlen(message);

    printf("Sending '%s' to the distant server.\n", message);

    if (send(sockfd, message, msg_len, 0) == -1){
      printf("Could not send message\n");
      return -1;
    }
    printf("Received from server: \n");

  if ((numbytes=recv(sockfd, buffer, BUFFER_SIZE-1, 0)) == -1) {
    printf("Could not receive from server\n");
    return -1;
}   

buffer[numbytes] = '\0';

printf("%s -> %s\n", message, buffer);

close(sockfd);

return 0;
}
