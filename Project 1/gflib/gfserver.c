#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <sys/fcntl.h>

#include "gfserver-student.h"
#include "content.h"
/* 
 * Modify this file to implement the interface specified in
 * gfserver.h.
 */


struct gfcontext_t
{
    int newsocket;
    char *path;
    int status;
};


struct gfserver_t{
    unsigned short port;
    int max_npending;
    char *handlerarg;
    gfh_error_t (*handler)(gfcontext_t **, const char *, void*)
};

void gfs_abort(gfcontext_t **ctx){
    close((*ctx)->newsocket);
}

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
        n = send((*ctx)->newsocket,data,len,0);  
        if (n < 0) error("ERROR writing to socket");
        if (n == 0) printf("done sending");
        data+=n;
        len-=n;
    }
    return total_writen;
}

ssize_t gfs_sendheader(gfcontext_t **ctx, gfstatus_t status, size_t file_len){
    char *strstatus=gfs_strstatus(status);
    int header_len=strlen(scheme)+strlen(strstatus)+24+strlen(marker);
    char header[header_len];
    ssize_t n;
    if (status==GF_OK)
    {
        sprintf(header,"%s %s %d %s",scheme,strstatus,file_len,marker);
    }
    else
    {
        sprintf(header,"%s %s %s",scheme,strstatus,marker);
    }
    n=send((*ctx)->newsocket,header,sizeof(header),0);
    if (n < 0) error("ERROR send header");
    if (status!=GF_OK)
    {
        gfs_abort(ctx);
    }
    return n;
}

void gfserver_serve(gfserver_t **gfs){
    if (!(*gfs)) {
        errno = EINVAL;
        return EXIT_FAILURE;
    }
    int newsockfd;
    int sockfd;
    char buffer[BUFSIZE],path[PATH_BUFFER_SIZE];
    ssize_t total_writen,n;
    gfcontext_t ctx;
    FILE* intl;
    socklen_t clilen;
    //    FILE *local_file=options->input;
    struct sockaddr_in serv_addr, cli_addr;
    struct stat statbuf;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons((*gfs)->port);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
            sizeof(serv_addr)) < 0) 
            error("ERROR on binding");
    listen(sockfd,(*gfs)->max_npending);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, 
            (struct sockaddr *) &cli_addr, 
            &clilen);
    if (newsockfd < 0){
        errno=ENOTCONN;
        return EXIT_FAILURE;
    } 
    ctx.newsocket=newsockfd;
    bzero(path,PATH_BUFFER_SIZE);
    n = recv(sockfd,path,PATH_BUFFER_SIZE,0);
    if (n < 0){
        errno=EBADMSG;
        return EXIT_FAILURE;
    } 
    parse_header(path,&ctx);
    content_get(ctx.path);
    gfs_sendheader(&&ctx,ctx.status,);
    n=send(newsockfd,buffer,BUFSIZE,0);
    if (n < 0){
        errno=EBADMSG;
        return EXIT_FAILURE;
    } 
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


