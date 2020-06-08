
#include "gfserver-student.h"

/* 
 * Modify this file to implement the interface specified in
 * gfserver.h.
 */
#define BUFSIZE 4096

void gfs_abort(gfcontext_t **ctx){

}
struct gfcontext_t
{
    int newsocket；
};


struct gfserver_t{
    unsigned short port;
    int max_npending;
    char *handlerarg;
    gfh_error_t (*handler)(gfcontext_t **, const char *, void*)
};

gfserver_t* gfserver_create(){
    gfserver_t *gfs;
    gfs=(gfserver_t*)malloc(sizeof(gfserver_t));
    return gfs;
}

ssize_t gfs_send(gfcontext_t **ctx, const void *data, size_t len){
    ssize_t n;
    ssize_t total_writen;
    while (len>0)
    {
        n = send((*ctx)->newsocket；,data,len,0);  
        if (n < 0) error("ERROR writing to socket");
        if (n == 0) printf("done sending");
        data+=n;
        len-=n;
    }
    return total_writen;
}

ssize_t gfs_sendheader(gfcontext_t **ctx, gfstatus_t status, size_t file_len){
    return -1;
}

void gfserver_serve(gfserver_t **gfs){
    if (!(*gfs)) {
     errno = EINVAL;
     return EXIT_FAILURE;
   }

   int newsockfd;
   int sockfd;
   int n;
   ssize_t total_writen;
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
   listen(sockfd,(*gfs)->max_npending);
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
   total_writen=gfs_send();
   close(newsockfd);
   close(sockfd);
   return 0; 
   /* send file to client */

   return EXIT_SUCCESS;

}

void gfserver_set_handlerarg(gfserver_t **gfs, void* arg){
    (*gfs)->handlerarg=arg;
}

void gfserver_set_handler(gfserver_t **gfs, gfh_error_t (*handler)(gfcontext_t **, const char *, void*)){
    (*gfs)->handler=handler;
}

void gfserver_set_maxpending(gfserver_t **gfs, int max_npending){
    (*gfs)->max_npending=max_npending;
}

void gfserver_set_port(gfserver_t **gfs, unsigned short port){
    (*gfs)->port=port;
}


