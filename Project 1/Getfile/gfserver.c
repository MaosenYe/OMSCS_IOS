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

/* 
 * Modify this file to implement the interface specified in
 * gfserver.h.
 */

void gfs_abort(gfcontext_t *ctx) {
//    free(ctx->parsed_header);
    printf("CLOSING SOCKET %d", ctx->client_socket);
//    close(ctx->client_socket);
}

gfserver_t *gfserver_create() {
    gfserver_t *server = calloc(1, sizeof(gfserver_t));
    return server;
}

ssize_t gfs_send(gfcontext_t *ctx, const void *data, size_t len) {
    ssize_t n;
    const char *ptr = data;
    ssize_t total_written = 0;
    printf("CALLING SEND\n");
    while (len > 0) {
        n = send(ctx->client_socket, ptr, len, 0);
        total_written += n;
        if (n < 0) {
            printf("FAILED TO WRITE TO SOCKET %d\n", ctx->client_socket);
            return -1;
        } else if (n == 0) {
            printf("DONE READING\n");
        }

        printf("WRITING %d BYTES TO CLIENT\n", (int) n);
        ptr += n;
        len -= n;
    }
    return total_written;
}

ssize_t gfs_sendheader(gfcontext_t *ctx, gfstatus_t status, size_t file_len) {
    char *TERMINATOR = "\r\n\r\n";
    const char *strstatus = gfs_strstatus(status);// gfs_strstatus(status); TODO
    int header_len = strlen("GETFILE") + strlen(strstatus) + 25 + strlen(TERMINATOR);
    char header[header_len];
    if (status == GF_OK) {
        sprintf(header, "GETFILE %s %d %s", strstatus, (int) file_len, TERMINATOR);
    } else {
        sprintf(header, "GETFILE %s %s", strstatus, TERMINATOR);
    }

    char file_len_str[1000];
    itoa(file_len, file_len_str, 10);   // here 10 means decimal

    strcpy(header, "GETFILE ");
    strcat(header, strstatus);
    strcat(header, " ");
    if (status == GF_OK)
        strcat(header, file_len_str);
    strcat(header, TERMINATOR);

    printf("WRITING HEADER %s\n", header);
    int n = -1;
    if (status == GF_ERROR) {
        n = (int) send(ctx->client_socket, header, header_len, 0);
        fflush(stdout);
        gfs_abort(ctx);
        return n;
    } else if (status == GF_FILE_NOT_FOUND) {
        n = (int) send(ctx->client_socket, header, header_len, 0);
        fflush(stdout);
        gfs_abort(ctx);
        return n;
    } else {
        n = (int) send(ctx->client_socket, header, header_len, 0);
        fflush(stdout);
        return n;
    }
}

void parse_header(char *header_buffer, int buff_size, struct gfsrequest_header *result) {
    result->path = "";

    char header_copy[buff_size];
    char request_path[buff_size - 12];

    strcpy(header_copy, header_buffer);
    int num_parts = sscanf(header_copy, "GETFILE GET %s\r\n\r\n", request_path);
    if (num_parts == EOF) {
        printf("MALFORMED HEADER\n");
        result->response_status = GF_FILE_NOT_FOUND;
        return;
    }

    result->response_status = GF_OK;
    result->path = request_path;
}

void gfserver_serve(gfserver_t *gfs) {
    // bind server to socket and accept connections
    if ((gfs->port < 1025) || (gfs->port > 65535)) {
        fprintf(stderr, "%s @ %d: invalid port number (%d)\n", __FILE__, __LINE__, gfs->port);
        exit(EXIT_FAILURE);
    }

    struct addrinfo hints, *result, *p;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // get the string length of the port
    int portlen = snprintf(NULL, 0, "%d", gfs->port) + 1;
    char *portstr = malloc((size_t) portlen);
    // convert portno to string
    snprintf(portstr, portlen, "%d", gfs->port);

    int addr_info = getaddrinfo(NULL, portstr, &hints, &result);
    if (addr_info != 0) {
        fprintf(stderr, "Failed to get the address info. %s\n", gai_strerror(addr_info));
        exit(EXIT_FAILURE);
    }

    int sock_fd = -1, newsock_fd;
    socklen_t client_len;
    struct sockaddr_in client_addr;
    int yes = 1;

    for (p = result; p != NULL; p = p->ai_next) {
        sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock_fd < 0) {
            continue;
        }

        if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1) {
            free(portstr);
            perror("setsockopt");
            exit(1);
        }

        if (bind(sock_fd, p->ai_addr, p->ai_addrlen) < 0) {
            free(portstr);
            close(sock_fd);
            perror("Failed to bind server to port");
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "Server bind failed. Could not bind to port %s\n", portstr);
        exit(EXIT_FAILURE);
    }

    free(portstr);
    freeaddrinfo(result);

    if (sock_fd < 0) {
        fprintf(stderr, "Socket creation failed\n");
        exit(EXIT_FAILURE);
    }

    // socket queue length of 10
    listen(sock_fd, gfs->max_pending);
    client_len = sizeof(client_addr);

//    int pid;

    gfcontext_t *gfcontext = calloc(1, sizeof(gfcontext_t));
    gfsrequest_header *header_result = calloc(1, sizeof(gfsrequest_header));
    char *TERMINATOR = "\r\n\r\n";
    while (true) {
        newsock_fd = accept(sock_fd, (struct sockaddr *) &client_addr, &client_len);
        if (newsock_fd < 0) {
            continue;
        }

        gfcontext->client_socket = newsock_fd;


        // handle client
        char client_header_buffer[MAX_PATH_LEN];
        char *header_ptr = client_header_buffer;
        int n = 0;
        bool header_found = false;
        int total_bytes_read = 0;
        while (true) {
            n = recv(gfcontext->client_socket, header_ptr, MAX_PATH_LEN, 0);
            if (n < 0) {
                // TODO: error, send INVALID header
                break;
            }

            if (n == 0) {
                break;
            }

            header_ptr += n;
            total_bytes_read += n;

            if (strstr(client_header_buffer, TERMINATOR) != NULL) {
                printf("HEADER FOUND: %s\n", client_header_buffer);
                header_found = true;
                break;
            }

            printf("READING %d BYTES FROM CLIENT\n", n);
//            printf("PTR IS: %s\n", header_ptr);
//            printf("BUFFER IS: %s\n", client_header_buffer);
        }

        printf("RECIEVED %d BYTES\n", total_bytes_read);
        printf("CLIENT HEADER BUFFER IS %s\n", client_header_buffer);
        parse_header(client_header_buffer, MAX_PATH_LEN, header_result);
        gfcontext->parsed_header = header_result;

        bool valid_path = strncmp(gfcontext->parsed_header->path, "/", 1) == 0;
        if (!header_found || !valid_path) {
            gfcontext->parsed_header->response_status = GF_FILE_NOT_FOUND;
        }
//        if (!header_found || !valid_path) {
//            const char *status = gfs_strstatus(GF_FILE_NOT_FOUND);
//            char response[50];
//            strcpy(response, "GETFILE ");
//            strcat(response, status);
//            strcat(response, TERMINATOR);
//            fflush(stdout);
//            send(gfcontext->client_socket, response, 50, 0);
//        } else {
////            free(header_result);
////            free(gfcontext);
//        }
        printf("CALLING HANDLER WITH: PATH: %s\n", header_result->path);
        fflush(stdout);
        gfs->handler(gfcontext, header_result->path, gfs->handler_arg);
        break;
    }
//    gfs_abort(gfcontext);
//    gfs_abort()
    free(header_result);
//    close(gfcontext->client_socket);
    free(gfcontext);
    free(gfs);
}

void gfserver_set_handlerarg(gfserver_t *gfs, void *arg) {
    gfs->handler_arg = arg;
}

void gfserver_set_handler(gfserver_t *gfs, ssize_t (*handler)(gfcontext_t *, const char *, void *)) {
    gfs->handler = handler;
}

void gfserver_set_maxpending(gfserver_t *gfs, int max_npending) {
    gfs->max_pending = max_npending;
}

void gfserver_set_port(gfserver_t *gfs, unsigned short port) {
    gfs->port = port;
}




//        pid = fork();
//        if (pid < 0) {
////            free(gfcontext);
//            perror("fork failed");
//            exit(EXIT_FAILURE);
//        }
//
//        if (pid == 0) {
//            close(sock_fd);
//            // TODO: handle client
//            handle_gfclient(gfcontext, gfs);
////            free(gfcontext);
//        }
////        else {
////            free(gfcontext);
////            close(newsock_fd);
////        }
//        printf("ABORTING...\n");
//        gfs_abort(gfcontext);
////        exit(EXIT_SUCCESS);

//void handle_gfclient(gfcontext_t *gfcontext, gfserver_t *gfs) {
//    // read from client
//    char *TERMINATOR = "\r\n\r\n";
//    int HEADER_LEN = 4096;
//    char header[HEADER_LEN];
//    gfsrequest_header *result = calloc(1, sizeof(gfsrequest_header));
//
//    int res = read_client_header(gfcontext, header, HEADER_LEN);
//    printf("READ CLIENT HEADER\n");
//    if (res < 0) {
//        // TODO: error occurred
//        result->path = NULL;
//    } else {
//        // get index of TERMINATOR
//        char *p = strstr(header, TERMINATOR);
//        if (p == NULL) {
//            // TODO: malformed header
//            result->path = NULL;
//        } else {
//            u_long terminator_index = p - header;
//            char trimmed_header[HEADER_LEN];
//            for (int i = 0; i < terminator_index; i++) {
//                trimmed_header[i] = header[i];
//            }
//            parse_header(trimmed_header, HEADER_LEN, result);
//            printf("PARSED CLIENT HEADER\n");
//        }
//    }
//
//    gfcontext->parsed_header = result;
//    gfs->handler(gfcontext, result->path, gfs->handler_arg);
//}

//int read_client_header(gfcontext_t *gfcontext, char *header, int header_size) {
//    char *TERMINATOR = "\r\n\r\n";
//    ssize_t n = 0;
//    int total = 0;
//    char buffer[header_size];
//    while (true) {
//        n = recv(gfcontext->client_socket, buffer, (size_t) (header_size - 1), 0);
//        if (n <= 0) {
//            break;
//        }
//        printf("GOT %d BUFFER BYTES: %s\n", (int) n, buffer);
//        total += n;
//        for (int i = 0; i < n; i++) {
//            header[total + i] = buffer[i];
//        }
//
//        if (strstr(header, TERMINATOR) == 0) {
//            break;
//        }
//    }
//
//    if (n == -1) {
//        return -1;
//    }
////    header[buff_size - 1] = '\0';
//    printf("GOT HEADER: %s\n", header);
//    return 0;
//}
