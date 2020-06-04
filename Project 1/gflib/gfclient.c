
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

struct gfcrequest_t{
    char    *server;
    char    *req_path;
    unsigned short  port;
    void *writefunc;
    void *headerfunc;
    void *writearg;
    void *headerarg;
    size_t  bytesreceived;
    size_t  filelen;
    gfstatus_t status;
    FILE    *file;
}
// optional function for cleaup processing.
void gfc_cleanup(gfcrequest_t **gfr){
    bzero(*gfr,sizeof(*gfr));
}

gfcrequest_t *gfc_create(){
    // dummy for now - need to fill this part in
    gfcrequest_t **gfr;
    (*gfr)->path="workload.txt";
    (*gfr)->port="20502"
    (*gfr)->server="127.0.0.1";
    return *gfr;
}

size_t gfc_get_bytesreceived(gfcrequest_t **gfr){
    // not yet implemented
    return (*gfr)->bytesreceived;
}

size_t gfc_get_filelen(gfcrequest_t **gfr){
    // not yet implemented
    return (*gfr)->filelen;
}

gfstatus_t gfc_get_status(gfcrequest_t **gfr){
    // not yet implemented
    return (*gfr)->status;
}

void gfc_global_init(){
}

void gfc_global_cleanup(){
}

int gfc_perform(gfcrequest_t **gfr){
    //Socket connect
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[BUFSIZE];
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
     errno=ESOCKTNOSUPPORT;
     return EXIT_FAILURE;
    }
    server = gethostbyname((*gfr)->server);
    if (server == NULL) {
     errno=EHOSTUNREACH;
     return EXIT_FAILURE;
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons((*gfr)->port);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
     errno=ENOTCONN;
     return EXIT_FAILURE;        
    }
    bzero(buffer,BUFSIZE);
    strcpy((char*) buffer ,(*gfr)->headerarg);
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

    return -1;
}

void gfc_set_headerarg(gfcrequest_t **gfr, void *headerarg){
    (*gfr)->headerarg=headerarg;
}

void gfc_set_headerfunc(gfcrequest_t **gfr, void (*headerfunc)(void*, size_t, void *)){
    (*gfr)->headerfunc=headerfunc;
}

void gfc_set_path(gfcrequest_t **gfr, const char* path){
    (*gfr)->req_path=path;
}

void gfc_set_port(gfcrequest_t **gfr, unsigned short port){
    (*gfr)->port=port;
}

void gfc_set_server(gfcrequest_t **gfr, const char* server){
    (*gfr)->server=server;
}

void gfc_set_writearg(gfcrequest_t **gfr, void *writearg){
    (*gfr)->writearg=writearg;
}

void gfc_set_writefunc(gfcrequest_t **gfr, void (*writefunc)(void*, size_t, void *)){
    (*gfr)->writefunc=writefunc;
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

