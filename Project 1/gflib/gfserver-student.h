/*
 *  This file is for use by students to define anything they wish.  It is used by the gf server implementation
 */
#ifndef __GF_SERVER_STUDENT_H__
#define __GF_SERVER_STUDENT_H__

#include "gf-student.h"
#include "gfserver.h"

 #define BUFSIZE 4096
 #define PATH_BUFFER_SIZE 2080
 #define scheme "GETFILE"
 #define method " GET "
 #define marker "\r\n\r\n"

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
    gfh_error_t (*handler)(gfcontext_t **, const char *, void*);
};
void parse_header(char *path_buffer, gfcontext_t *ctx);
char* gfs_strstatus(gfstatus_t status);


#endif // __GF_SERVER_STUDENT_H__