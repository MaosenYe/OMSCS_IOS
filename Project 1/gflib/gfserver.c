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
#include "gfserver.h"
/* 
 * Modify this file to implement the interface specified in
 * gfserver.h.
 */


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
    ssize_t total_writen=0;
    while (len>0)
    {
        n = send((*ctx)->newsocket,data,len,0);  
        if (n < 0) {
        errno=EBADMSG;
        exit(EXIT_FAILURE);
        } 
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
        sprintf(header,"%s %s %ld %s",scheme,strstatus,file_len,marker);
    }
    else
    {
        sprintf(header,"%s %s %s",scheme,strstatus,marker);
    }
    n=send((*ctx)->newsocket,header,sizeof(header),0);
    if (n < 0) {
        errno=EBADMSG;
        exit(EXIT_FAILURE);
    } 
    if (status!=GF_OK)
    {
        exit(EXIT_FAILURE);
    }
    return n;
}

void gfserver_serve(gfserver_t **gfs){
    if (!(*gfs)) {
        errno = EINVAL;
        exit(EXIT_FAILURE);
    }
    int newsockfd;
    int sockfd;
    char buffer[BUFSIZE],path[PATH_BUFFER_SIZE];
    ssize_t total_writen=0,n=0;
    gfcontext_t ctx;
    gfcontext_t* ptr=&ctx;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    struct stat statbuf;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        exit(EXIT_FAILURE);
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons((*gfs)->port);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
            sizeof(serv_addr)) < 0) 
            exit(EXIT_FAILURE);
    listen(sockfd,(*gfs)->max_npending);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, 
            (struct sockaddr *) &cli_addr, 
            &clilen);
    if (newsockfd < 0){
        errno=ENOTCONN;
        exit(EXIT_FAILURE);
    } 
    ctx.newsocket=newsockfd;
    bzero(path,PATH_BUFFER_SIZE);
    n = recv(sockfd,path,PATH_BUFFER_SIZE,0);
    if (n < 0){
        errno=EBADMSG;
        exit(EXIT_FAILURE);
    } 
    parse_header(path,&ctx);
    
    int fd=content_get(ctx.path);
    if (fd == -1)
    {
        printf("failed to open %s\n", ctx.path);
        ctx.status=GF_FILE_NOT_FOUND;
    }
    if (fstat(fd, &statbuf) == -1)
    {
        printf("failed to fstat %s\n", ctx.path);
        ctx.status=GF_ERROR;
    }
    gfs_sendheader(&ptr,ctx.status,statbuf.st_size);
    int read_num=0;
    while (read_num!=0)
    {
        read_num=read(fd, buffer, BUFSIZE); 
        n=send(newsockfd,buffer,BUFSIZE,0);
        if (n < 0){
        errno=EBADMSG;
        exit(EXIT_FAILURE);
        } 
        total_writen+=n;
    }
    close(newsockfd);
    close(sockfd);
    /* send file to client */
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

void parse_header(char *path_buffer, gfcontext_t *ctx) {
    ctx->path=calloc(PATH_BUFFER_SIZE, sizeof(char));
    char path[PATH_BUFFER_SIZE];
    int num_parts = sscanf(path_buffer, "GETFILE GET %s\r\n\r\n", path);
    if (num_parts == EOF) {
        printf("MALFORMED HEADER\n");
        ctx->status = GF_INVALID;
        return;
    }
    strncpy(ctx->path,path,PATH_BUFFER_SIZE);
    ctx->status = GF_OK;
}


 char* gfs_strstatus(gfstatus_t status){
    char *strstatus = NULL;
    switch (status)
    {
        default: {
            strstatus = "UNKNOWN";
        }
        break;

        case GF_INVALID: {
            strstatus = "INVALID";
        }
        break;

        case GF_FILE_NOT_FOUND: {
            strstatus = "FILE_NOT_FOUND";
        }
        break;

        case GF_ERROR: {
            strstatus = "ERROR";
        }
        break;

        case GF_OK: {
            strstatus = "OK";
        }
        break;   
    }

    return strstatus;
}

