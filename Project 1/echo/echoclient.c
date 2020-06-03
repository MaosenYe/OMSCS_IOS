/* client.c */
/* A simple client using TCP
   The server port and message are passed as an argument */
/* 0  Copyright (c) 2020 もぃ. All rights reserved. */
/* 1 includes */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

/* 2 defines */ 
#define USAGE                                                                       \
    "usage:\n"                                                                      \
    "  echoclient [options]\n"                                                      \
    "options:\n"                                                                    \
    "  -s                  Server (Default: localhost)\n"                           \
    "  -p                  Port (Default: 20502)\n"                                  \
    "  -m                  Message to send to server (Default: \"Hello world.\")\n" \
    "  -h                  Show this help message\n"
#define OPTSTR "s:p:m:hx"
#define USAGE_FMT  "%s [-s Server] [-p Port] [-m Message] [-h]\n"
#define ERR_SEND_SOCKET "send sockets"
#define DEFAULT_PROGNAME "moli"
#define BUFSIZE 256

/* 3 external declarations */
extern int errno;
extern char *optarg;
extern int opterr, optind;

/* 4 typedefs */
typedef struct {
  char          *hostname;
  int           portno;
  char          *message;
} options_t;

/* 5 global variable declarations */
/* 6 function prototypes */
void usage(char *progname, int opt);
int  send_sockets(options_t *options);

/* 7 main function  */
int main(int argc, char *argv[]) {
    int opt;
    options_t  options={"localhost",20502,"Hello World"};
    opterr = 0;

    while ((opt = getopt(argc, argv, OPTSTR)) != EOF)
       switch(opt) {
           case 's':
                options.hostname=optarg;
                break;

           case 'p':
                options.portno=atoi(optarg);
                break;

           case 'm':
                options.message=optarg;
                break;               

           case 'h':
              fprintf(stdout, "%s", USAGE);
              exit(0);
              break;

           default:
              usage(basename(argv[0]), opt);
              /* NOTREACHED */
              break;
       }

    if (send_sockets(&options) != EXIT_SUCCESS) {
       perror(ERR_SEND_SOCKET);
       exit(EXIT_FAILURE);
       /* NOTREACHED */
    }

    return EXIT_SUCCESS;
}  

/* 8 function declarations */
void usage(char *progname, int opt) {
   fprintf(stderr, USAGE_FMT, progname?progname:DEFAULT_PROGNAME);
   exit(EXIT_FAILURE);
   /* NOTREACHED */
}

int send_sockets(options_t *options) {

   if (!options) {
     errno = EINVAL;
     return EXIT_FAILURE;
   }

   if ((options->hostname==NULL)||(options->portno < 1025) || (options->portno > 65535)) {
     errno = EDESTADDRREQ;
     return EXIT_FAILURE;
   }

   if (options->message==NULL) {
     errno = ENODATA;
     return EXIT_FAILURE;
   }

    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[BUFSIZE];
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
     errno=ESOCKTNOSUPPORT;
     return EXIT_FAILURE;
    }
    server = gethostbyname(options->hostname);
    if (server == NULL) {
     errno=EHOSTUNREACH;
     return EXIT_FAILURE;
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(options->portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
     errno=ENOTCONN;
     return EXIT_FAILURE;        
    }
    bzero(buffer,BUFSIZE);
    strcpy((char*) buffer , options->message);
    int n = write(sockfd,buffer,strlen(buffer));
    if (n < 0){
     errno=EBADMSG;
     return EXIT_FAILURE;
    } 
    bzero(buffer,BUFSIZE);
    n = read(sockfd,buffer,BUFSIZE);
    if (n < 0){
     errno=EBADMSG;
     return EXIT_FAILURE;
    } 
    printf("%s\n",buffer);
    close(sockfd);
    return EXIT_SUCCESS;
}