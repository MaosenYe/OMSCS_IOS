/*
 *  This file is for use by students to define anything they wish.  It is used by the gf server implementation
 */
#ifndef __GF_SERVER_STUDENT_H__
#define __GF_SERVER_STUDENT_H__

#include "gfserver.h"
#include "gf-student.h"

#define MAX_PATH_LEN 4096

typedef struct gfsrequest_header {
    char *path;
    gfstatus_t response_status;
} gfsrequest_header;

typedef struct gfserver_t {
    int port;
    int max_pending;

    void *handler_arg;

    ssize_t (*handler)(gfcontext_t *, const char *, void *);
} gfserver_t;

typedef struct gfcontext_t {
    int client_socket;
    gfsrequest_header *parsed_header;
} gfcontext_t;

void handle_gfclient(gfcontext_t *client_context, gfserver_t *gfs);

int read_client_header(gfcontext_t *gfcontext, char *buffer, int buff_size);

void parse_header(char *header_buffer, int buff_size, struct gfsrequest_header *result);

int write_response(int socket_fd, gfsrequest_header *header);

const char *gfs_strstatus(gfstatus_t status) {
    const char *strstatus = NULL;

    switch (status) {
        default:
            strstatus = "FILE_NOT_FOUND";
            break;

        case GF_INVALID:
            strstatus = "INVALID";
            break;

        case GF_FILE_NOT_FOUND:
            strstatus = "FILE_NOT_FOUND";
            break;

        case GF_ERROR:
            strstatus = "ERROR";
            break;

        case GF_OK:
            strstatus = "OK";
            break;
    }

    return strstatus;
}

/* A utility function to reverse a string  */
void reverse(char str[], int length) {
    int begin = 0;
    int end = length - 1;
    while (begin < end) {
        char s =*(str + begin);
        char e = *(str + end);
        char t = s;
        s = e;
        e = t;
        begin++;
        end--;
    }
}

// Implementation of itoa()
// from https://www.geeksforgeeks.org/implement-itoa/
char *itoa(int num, char *str, int base) {
    int i = 0;
    bool isNegative = false;

    /* Handle 0 explicitely, otherwise empty string is printed for 0 */
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    // In standard itoa(), negative numbers are handled only with
    // base 10. Otherwise numbers are considered unsigned.
    if (num < 0 && base == 10) {
        isNegative = true;
        num = -num;
    }

    // Process individual digits
    while (num != 0) {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num = num / base;
    }

    // If number is negative, append '-'
    if (isNegative)
        str[i++] = '-';

    str[i] = '\0'; // Append string terminator

    // Reverse the string
    reverse(str, i);

    return str;
}


#endif // __GF_SERVER_STUDENT_H__