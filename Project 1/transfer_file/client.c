#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define buffer_size 4096

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    int socket_num=0;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    FILE *fptr;
    char buffer[buffer_size];

    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    fptr=fopen("receive.txt","w");
    if(fptr==NULL)
    {
        error("Unable to create file");
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    bzero(buffer,buffer_size);

    int tmp;
    recv(sockfd, &tmp, sizeof(tmp),0);
    socket_num = ntohl(tmp);
    fprintf(stderr,"%d\n",socket_num);
    // close(sockfd);
    // sockfd = -1;
    // if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
    //     error("ERROR connecting");

    for (size_t i = 0; i < socket_num; i++)
    {
        bzero(buffer,buffer_size);
        n = recv(sockfd,buffer,4096,0);
        if (n < 0) 
            error("ERROR reading from socket");
        fputs(buffer,fptr);
        fprintf(stderr,"%s\n",buffer);
    }
    close(sockfd);
    return 0;
}