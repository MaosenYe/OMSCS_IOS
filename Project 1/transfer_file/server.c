/* server.c - tranfer the file to client */

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
#include <sys/stat.h>

#define OPTSTR "i:f:p:h"
#define USAGE_FMT  "%s [-f hexflag] [-p portnumber] [-i inputfile] [-h] \n"
#define buffer_size 4096

#define ERR_FOPEN_INPUT  "fopen(input, r)"
#define ERR_FILE_READ "can't read the file in memory"
#define ERR_SOCKET_SEND "can't send the file by sockets"
#define DEFAULT_PROGNAME "unknown program"


extern int errno;
extern char *optarg;
extern int opterr, optind;

char buffer[buffer_size];

typedef struct {
  int          portnum;
  uint32_t      flags;
  FILE         *input;
} options_t;

typedef struct {
int sockfd;
struct sockaddr_in *cli_addr;
int clilen;
} sockets_t;

int dumb_global_variable = -11;

void usage(char *progname, int opt);
int  file_send(options_t *options,char *file_name);
void error(const char *msg);

int main(int argc, char *argv[]) {
    int opt;
    char* file_name="test.txt";
    options_t options = {5708, 0x0, fopen(file_name, "r")};

    opterr = 0;
    while ((opt = getopt(argc, argv, OPTSTR)) != EOF)
       switch(opt) {
           case 'i':
              *file_name=*optarg;
              if (!(options.input = fopen(optarg, "r")) ){
                 perror(ERR_FOPEN_INPUT);
                 exit(EXIT_FAILURE);
                 /* NOTREACHED */
              }
              break;
             
           case 'f':
              options.flags = (uint32_t )strtoul(optarg, NULL, 16);
              break;

           case 'p':
              options.portnum = atoi(optarg);
              break;

           case 'h':
           default:
              usage(basename(argv[0]), opt);
              /* NOTREACHED */
              break;
       }

    if (file_send(&options,file_name) != EXIT_SUCCESS) {
       perror(ERR_SOCKET_SEND);
       exit(EXIT_FAILURE);
       /* NOTREACHED */
    }

    return EXIT_SUCCESS;
}

void usage(char *progname, int opt) {
   fprintf(stderr, USAGE_FMT, progname?progname:DEFAULT_PROGNAME);
   exit(EXIT_FAILURE);
   /* NOTREACHED */
}


int file_send(options_t *options,char *file_name) {

   if (!options) {
     errno = EINVAL;
     return EXIT_FAILURE;
   }

   if (!options->input) {
     errno = ENOENT;
     return EXIT_FAILURE;
   }
   int newsockfd;
   int sockfd;
   int n;
   socklen_t clilen;
   FILE *local_file=options->input;
   struct sockaddr_in serv_addr, cli_addr;
   struct stat statbuf;

   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   if (sockfd < 0) 
      error("ERROR opening socket");
   bzero((char *) &serv_addr, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = INADDR_ANY;
   serv_addr.sin_port = htons(options->portnum);
   if (bind(sockfd, (struct sockaddr *) &serv_addr,
            sizeof(serv_addr)) < 0) 
            error("ERROR on binding");
   listen(sockfd,5);
   clilen = sizeof(cli_addr);
   newsockfd = accept(sockfd, 
            (struct sockaddr *) &cli_addr, 
            &clilen);
   if (newsockfd < 0) 
         error("ERROR on accept");
   
   stat(file_name,&statbuf);
   int size=statbuf.st_size/buffer_size+1;
   int tmp = htonl(size);
   // fprintf(stderr,"%d",size);
   n=send(newsockfd, &tmp, sizeof(tmp),0);
   if (n < 0) error("ERROR writing to socket");
   fprintf(stderr,"%d\n",size);
   for (size_t i = 0; i < size; i++)
   {
      bzero(buffer,buffer_size);
      fgets(buffer,4096,(FILE*)local_file);
      n = send(newsockfd,buffer,buffer_size,0);
      if (n < 0) error("ERROR writing to socket");
   }
   close(newsockfd);
   close(sockfd);
   return 0; 
   /* send file to client */

   return EXIT_SUCCESS;
}

void error(const char *msg)
{
    perror(msg);
    exit(0);
}
