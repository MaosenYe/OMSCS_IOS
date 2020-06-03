// /* A simple server in the internet domain using TCP
//    The port number is passed as an argument */
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <sys/types.h> 
// #include <sys/socket.h>
// #include <netinet/in.h>

// void error(const char *msg)
// {
//     perror(msg);
//     exit(1);
// }

// int main(int argc, char *argv[])
// {
//      int sockfd, newsockfd, portno;
//      socklen_t clilen;
//      char buffer[BUFSIZE];
//      struct sockaddr_in serv_addr, cli_addr;
//      int n;
//      if (argc < 2) {
//          fprintf(stderr,"ERROR, no port provided\n");
//          exit(1);
//      }
//      sockfd = socket(AF_INET, SOCK_STREAM, 0);
//      if (sockfd < 0) 
//         error("ERROR opening socket");
//      bzero((char *) &serv_addr, sizeof(serv_addr));
//      portno = atoi(argv[1]);
//      serv_addr.sin_family = AF_INET;
//      serv_addr.sin_addr.s_addr = INADDR_ANY;
//      serv_addr.sin_port = htons(portno);
//      if (bind(sockfd, (struct sockaddr *) &serv_addr,
//               sizeof(serv_addr)) < 0) 
//               error("ERROR on binding");
//      listen(sockfd,5);
//      clilen = sizeof(cli_addr);
//      while (1)
//      {
//      newsockfd = accept(sockfd, 
//                  (struct sockaddr *) &cli_addr, 
//                  &clilen);
//      if (newsockfd < 0) 
//           error("ERROR on accept");
//      bzero(buffer,BUFSIZE);
//      n = read(newsockfd,buffer,255);
//      if (n < 0) error("ERROR reading from socket");
//      printf("Here is the message: %s\n",buffer);
//      n = write(newsockfd,buffer,BUFSIZE);
//      if (n < 0) error("ERROR writing to socket");
//     close(newsockfd);
//      }
//      close(sockfd);
//      return 0; 
// }
/* client.c */
/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
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
#define USAGE                                                                 \
"usage:\n"                                                                    \
"  echoserver [options]\n"                                                    \
"options:\n"                                                                  \
"  -p                  Port (Default: 20502)\n"                                \
"  -m                  Maximum pending connections (default: 1)\n"            \
"  -h                  Show this help message\n" 
#define OPTSTR "p:h:"
#define USAGE_FMT  "%s [-p Port]  [-h]\n"
#define ERR_RECEIVE_SOCKET "receive sockets"
#define DEFAULT_PROGNAME "moli"
#define BUFSIZE 256

/* 3 external declarations */
extern int errno;
extern char *optarg;
extern int opterr, optind;

/* 4 typedefs */
typedef struct {
  int           portno;
  int           maxnpending;
} options_t;

/* 5 global variable declarations */
/* 6 function prototypes */
void usage(char *progname, int opt);
int  receive_sockets(options_t *options);

/* 7 main function  */
int main(int argc, char *argv[]) {
    int opt;
    options_t options={20502,1};

    opterr = 0;

    while ((opt = getopt(argc, argv, OPTSTR)) != EOF)
       switch(opt) {
           case 'p':
                options.portno=atoi(optarg);
                break;

           case 'h':
              fprintf(stdout, "%s", USAGE);
              exit(EXIT_SUCCESS);
              break;

           case 'm': 
              options.maxnpending = atoi(optarg);
              break;

           default:
              usage(basename(argv[0]), opt);
              /* NOTREACHED */
              break;
       }

    if (receive_sockets(&options) != EXIT_SUCCESS) {
       perror(ERR_RECEIVE_SOCKET);
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

int receive_sockets(options_t *options) {

   if (!options) {
     errno = EINVAL;
     return EXIT_FAILURE;
   }

   if ((options->portno < 1025) || (options->portno > 65535)) {
     errno = EDESTADDRREQ;
     return EXIT_FAILURE;
   }

   if (options->maxnpending<1) {
     errno = EDESTADDRREQ;
     return EXIT_FAILURE;
   }
   int sockfd, newsockfd;
   socklen_t clilen;
   char buffer[BUFSIZE];
   struct sockaddr_in serv_addr, cli_addr;
   int n;
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
     errno=ESOCKTNOSUPPORT;
     return EXIT_FAILURE;
    }
   bzero((char *) &serv_addr, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = INADDR_ANY;
   serv_addr.sin_port = htons(options->portno);
   if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
     errno=ENOTCONN;
     return EXIT_FAILURE;
   }
   listen(sockfd,options->maxnpending);
   clilen = sizeof(cli_addr);
   while (1)
   {
   newsockfd = accept(sockfd, 
               (struct sockaddr *) &cli_addr, 
               &clilen);
   if (newsockfd < 0){
     errno=ENODATA;
     return EXIT_FAILURE;
    } 
   bzero(buffer,BUFSIZE);
   n = read(newsockfd,buffer,BUFSIZE);
   if (n < 0){
     errno=EBADMSG;
     return EXIT_FAILURE;
    } 
   printf("Here is the message: %s\n",buffer);
   n = write(newsockfd,buffer,BUFSIZE);
   if (n < 0){
     errno=EBADMSG;
     return EXIT_FAILURE;
    } 
   close(newsockfd);
   }
   close(sockfd);
    return EXIT_SUCCESS;
}