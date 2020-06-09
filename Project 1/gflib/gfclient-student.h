/*
 *  This file is for use by students to define anything they wish.  It is used by the gf client implementation
 */
 #ifndef __GF_CLIENT_STUDENT_H__
 #define __GF_CLIENT_STUDENT_H__
 
 #include "gfclient.h"
 #include "gf-student.h"
 #define scheme "GETFILE"
 #define method " GET "
 #define marker "\r\n\r\n"

 /* Callbacks ========================================================= */
  char* request_head(char* path){
  size_t total_len = strlen("GETFILE GET ") + strlen(path) + strlen(marker) + 1;
  char *message = malloc(total_len * sizeof(char));
  fprintf(stdout, "SENDING REQUEST\nGETFILE GET %s%s",path, marker);
  snprintf(message, total_len, "GETFILE GET %s%s",path, marker);
  return message;
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
 #endif // __GF_CLIENT_STUDENT_H__