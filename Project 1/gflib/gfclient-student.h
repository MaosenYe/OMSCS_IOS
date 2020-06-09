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


 #endif // __GF_CLIENT_STUDENT_H__