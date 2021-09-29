#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8001       /* the port client will be connecting to */
#define BUFFER_SIZE 128 /* max number of bytes we can get at once */

int main(int argc, char *argv[])
{
  int sockfd, numbytes;
  char msg_to_send[BUFFER_SIZE];
  char received_msg[BUFFER_SIZE];
  struct hostent *he;
  struct sockaddr_in server; /* connector's address information */

  if (argc != 2)
  {
    fprintf(stderr, "usage: ./enigma_client distantServerIP\n");
    return -1;
  }

  if ((he = gethostbyname(argv[1])) == NULL)
  {
    printf("Could not get IP address from input\n");
    return -1;
  }

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    printf("Cannot open socket\n");
    return -1;
  }

  server.sin_family = AF_INET;   /* host byte order */
  server.sin_port = htons(PORT); /* short, network byte order */
  server.sin_addr = *((struct in_addr *)he->h_addr);
  bzero(&(server.sin_zero), 8); /* zero the rest of the struct */

  if (connect(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
  {
    printf("Couldn't connect to distant server\n");
    return -1;
  }
  
  int i = 0;
  while (i++ < 254)
  {
    // server sends some info
    if ((numbytes = recv(sockfd, received_msg, BUFFER_SIZE - 1, 0)) == -1)
    {
      printf("Could not receive from server\n");
      return -1;
    }
    received_msg[numbytes] = '\0';
    //printf("%s\n", received_msg);
    memset(&received_msg, 0, BUFFER_SIZE);
    
    // we can send password
    if (send(sockfd, "1234", 4, 0) == -1)
    {
      printf("Could not send message\n");
      return -1;
    }
  }
  
  while (1)
  {
    // server sends some info
    if ((numbytes = recv(sockfd, received_msg, BUFFER_SIZE - 1, 0)) == -1)
    {
      printf("Could not receive from server\n");
      return -1;
    }
    received_msg[numbytes] = '\0';
    printf("%s\n", received_msg);
    memset(&received_msg, 0, BUFFER_SIZE);
    
    // we can send password
    scanf("%s", msg_to_send);
    int msg_len = strlen(msg_to_send);
    if (send(sockfd, msg_to_send, msg_len, 0) == -1)
    {
      printf("Could not send message\n");
      return -1;
    }
  }

  close(sockfd);

  return 0;
}
