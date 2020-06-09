
#include "gfclient-student.h"
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

#define BUFSIZE 4096


struct gfcrequest_t{
    char    *server;
    char    *req_path;
    unsigned short  port;
    void  (*writefunc)(void *data_buffer, size_t data_buffer_length, void *handlerarg);
    char*  (*headererfunc)(void *headerer_buffer, size_t headerer_buffer_length, void *handlerarg);
    char *writearg;
    char *headererarg;
    size_t  bytesreceived;
    size_t  filelen;
    gfstatus_t status;
    FILE    *file;
};
// optional function for cleaup processing.
void gfc_cleanup(gfcrequest_t **gfc){

    free((*gfc)->req_path);
    free((*gfc)->server);
    free(*gfc);
}

gfcrequest_t *gfc_create(){
    // dummy for now - need to fill this part in
    gfcrequest_t *gfc=(gfcrequest_t*)malloc(sizeof(gfcrequest_t));
    return gfc;
}

size_t gfc_get_bytesreceived(gfcrequest_t **gfc){
    // not yet implemented
    return (*gfc)->bytesreceived;
}

size_t gfc_get_filelen(gfcrequest_t **gfc){
    // not yet implemented
    return (*gfc)->filelen;
}

gfstatus_t gfc_get_status(gfcrequest_t **gfc){
    // not yet implemented
    return (*gfc)->status;
}

void gfc_global_init(){
}

void gfc_global_cleanup(){
}

int gfc_perform(gfcrequest_t **gfc){
    //Socket connect
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[BUFSIZE];
    int total_byte=0;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
     errno=ESOCKTNOSUPPORT;
     return EXIT_FAILURE;
    }
    server = gethostbyname((*gfc)->server);
    if (server == NULL) {
     errno=EHOSTUNREACH;
     return EXIT_FAILURE;
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons((*gfc)->port);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
     errno=ENOTCONN;
     return EXIT_FAILURE;        
    }
    bzero(buffer,BUFSIZE);
    char *header=request_head((*gfc)->req_path);
    strcpy((char*) buffer,header);
    int n = write(sockfd,(*gfc)->headererarg,strlen((*gfc)->headererarg));
    if (n < 0){
     errno=EBADMSG;
     return EXIT_FAILURE;
    } 
    bzero(buffer,BUFSIZE);
    free(header);
    n = read(sockfd,buffer,BUFSIZE);
    if (n < 0){
     errno=EBADMSG;
     (*gfc)->status=GF_INVALID;
     return EXIT_FAILURE;
    } 
    total_byte+=n;
    char* ptr=strstr(marker,buffer);
    if (ptr==NULL)
    {
        (*gfc)->status=GF_INVALID;
        (*gfc)->filelen=0;
        (*gfc)->bytesreceived=total_byte;
        return EXIT_FAILURE;
    }
    
    int response_len=ptr-buffer+strlen(marker);
    (*gfc)->headererarg = malloc((response_len+1)* sizeof(char));
    strncpy((*gfc)->headererarg,buffer,response_len);
    
    switch ((*gfc)->status)
    {

    case GF_INVALID: {
        return EXIT_FAILURE;
    }
    break;

    case GF_FILE_NOT_FOUND: {
        return EXIT_FAILURE;
    }
    break;

    case GF_ERROR: {
        return EXIT_FAILURE;
    }
    break;

    case GF_OK: {
        (*gfc)->writefunc(ptr,BUFSIZE-response_len,(*gfc)->writearg);
        while (n)
        {
            bzero(buffer,BUFSIZE);

            (*gfc)->writefunc(buffer,BUFSIZE,(*gfc)->writearg);
        }
        return EXIT_SUCCESS;
    }
    break;
    
    default:
        return EXIT_FAILURE;
    }

}

void gfc_set_headererarg(gfcrequest_t **gfc, void *headererarg){
    (*gfc)->headererarg=headererarg;
}

void gfc_set_headererfunc(gfcrequest_t **gfc, void (*headererfunc)(void*, size_t, void *)){
    (*gfc)->headererfunc=headererfunc;
}

void gfc_set_path(gfcrequest_t **gfc, const char* path){
    (*gfc)->req_path = calloc(strlen(path) + 1, sizeof(char));
    if (NULL == path || NULL == (*gfc)->req_path) {
        fprintf(stderr, "%s @ %d: failed to set server\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
    strncpy((*gfc)->req_path, path, strlen(path));
    printf("SET SERVER TO: %s\n", (*gfc)->req_path);
}

void gfc_set_port(gfcrequest_t **gfc, unsigned short port){
    (*gfc)->port=port;
    printf("SET SERVER TO: %s\n", (*gfc)->server);
}

void gfc_set_server(gfcrequest_t **gfc, const char* server){
    // (*gfc)->server=server;
    (*gfc)->server = calloc(strlen(server) + 1, sizeof(char));
    if (NULL == server || NULL == (*gfc)->server) {
        fprintf(stderr, "%s @ %d: failed to set server\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
    strncpy((*gfc)->server, server, strlen(server));
    printf("SET SERVER TO: %s\n", (*gfc)->server);
}

void gfc_set_writearg(gfcrequest_t **gfc, void *writearg){
    (*gfc)->writearg=writearg;
}

void gfc_set_writefunc(gfcrequest_t **gfc, void (*writefunc)(void*, size_t, void *)){
    (*gfc)->writefunc=writefunc;
}

const char* gfc_strstatus(gfstatus_t status){
    const char *strstatus = NULL;

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

