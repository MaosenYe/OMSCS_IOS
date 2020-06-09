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


const char* gfs_strstatus(gfstatus_t status){
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

#endif // __GF_SERVER_STUDENT_H__