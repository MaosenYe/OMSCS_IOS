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
#include <sys/fcntl.h>
#include <stdbool.h>
#include <sys/time.h>
#include <ctype.h>
#include <math.h>
#include <arpa/inet.h>

#include "gfclient-student.h"

// TODO: check if removing the close socket helps pass the control data test
gfstatus_t gfc_status_from_str(const char *status);

typedef struct gfcresponse_header {
    char *scheme;
//    char *status;
    gfstatus_t status;
    int filelen;
    int buffer_offset;
} gfcresponse_header;

void parse_header(char *header, gfcresponse_header *result) {
    char header_cp[strlen(header) + 1];
    strcpy(header_cp, header);

    fprintf(stdout, "HEADER:\n");
    fputs(header_cp, stdout);
    fprintf(stdout, "\n");

    // extract header info
    char *sep = " ";
    char *header_scheme = strtok(header_cp, sep);
    if (header_scheme == NULL) {
        result->status = GF_INVALID;
        return;
    }
    result->scheme = header_scheme;

    char *header_status = NULL;
    char *header_filelen = NULL;
    if (strcmp(header_scheme, "GETFILE") == 0) {
        header_status = strtok(NULL, sep);
        if (header_status == NULL || strcmp(header_status, "OK") != 0) {
            // malformed header
            result->status = GF_INVALID;
            return;
        }

        gfstatus_t status = gfc_status_from_str(header_status);
        result->status = status;
    } else {
        result->status = GF_INVALID;
        return;
    }

    if (result->status == GF_OK) {
        header_filelen = strtok(NULL, sep);
        int filelen = atoi(header_filelen);
        result->filelen = filelen;
    } else {
        result->filelen = 0;
    }
}

gfstatus_t gfc_status_from_str(const char *status) {
    if (strcmp(status, "OK") == 0) {
        return GF_OK;
    } else if (strcmp(status, "FILE_NOT_FOUND") == 0) {
        return GF_FILE_NOT_FOUND;
    } else if (strcmp(status, "ERROR") == 0) {
        return GF_ERROR;
    } else if (strcmp(status, "INVALID") == 0) {
        return GF_INVALID;
    } else {
        return GF_INVALID;
    }
}

typedef struct gfcrequest_t {
    char *server;
    unsigned short port;
    char *path;

    size_t bytesrecieved;
    size_t filelen;
    gfstatus_t status;

    void *writearg;

    void (*writefunc)(void *, size_t, void *);

    void *headerarg;

    void (*headerfunc)(void *, size_t, void *);
} gfcrequest_t;

// optional function for cleaup processing.
void gfc_cleanup(gfcrequest_t *gfr) {
//    close(socket_fd);
    free(gfr->server);
    free(gfr->path);
    free(gfr);
}

gfcrequest_t *gfc_create() {
    gfcrequest_t *request = (gfcrequest_t *) calloc(1, sizeof(gfcrequest_t));
    return request;
}

size_t gfc_get_bytesreceived(gfcrequest_t *gfr) {
    // not yet implemented
    return gfr->bytesrecieved;
}

size_t gfc_get_filelen(gfcrequest_t *gfr) {
    return gfr->filelen;
}

gfstatus_t gfc_get_status(gfcrequest_t *gfr) {
    return gfr->status;
}

void gfc_global_init() {
    // make socket
}

void gfc_global_cleanup() {

}

void gfr_ok(gfcrequest_t *gfr, gfstatus_t status, size_t file_len) {
    gfr->status = status;
    gfr->filelen = file_len;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}


int gfc_perform(gfcrequest_t *gfr) {

    if (gfr->port < 1025 || gfr->port > 65535) {
        fprintf(stderr, "%s @ %d: invalid port number (%d)\n", __FILE__, __LINE__, gfr->port);
        return -1;
    }

    if (NULL == gfr->server) {
        fprintf(stderr, "%s @ %d: invalid server\n", __FILE__, __LINE__);
        return -1;
    }

    int portlen = snprintf(NULL, 0, "%d", gfr->port) + 1;
    char *portstr = malloc((size_t) portlen);
    snprintf(portstr, portlen, "%d", gfr->port);
    printf("PORT STR IS %s\n", portstr);

    struct addrinfo hints, *servinfo, *p;
    int socket_fd = -1;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // or AF_UNSPEC
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int rv = getaddrinfo(NULL, portstr, &hints, &servinfo);
    if (rv != 0) {
        free(portstr);
        perror("getaddrinfo error\n");
        return 1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        //Initialize socket
        socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (socket_fd < 0) continue;
        //Initialize connection
        rv = connect(socket_fd, p->ai_addr, (socklen_t) p->ai_addrlen);
        if (rv == 0) break;
        close(socket_fd);
        socket_fd = -1;
    }

    if (socket_fd < 0) //Error creating/connecting socket
    {
        free(portstr);
        perror("Error creating/connecting socket \n");
        exit(1);
    }
    free(portstr);

    struct timeval tv;
    tv.tv_sec = 3;
    setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *) &tv, sizeof(struct timeval));

    char s[INET6_ADDRSTRLEN];
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *) p->ai_addr),
              s, sizeof s);
    freeaddrinfo(servinfo);

    char *TERMINATOR = "\r\n\r\n";
    // compose request message
    // request message: GETFILE GET gfr->path\r\n\r\n
    size_t total_len = strlen("GETFILE GET ") + strlen(gfr->path) + strlen(TERMINATOR) + 1;
    char *message = malloc(total_len * sizeof(char));
    fprintf(stdout, "SENDING REQUEST\nGETFILE GET %s%s", gfr->path, TERMINATOR);
    snprintf(message, total_len, "GETFILE GET %s%s", gfr->path, TERMINATOR);

    char *data = message;
    size_t data_len = strlen(data);
    ssize_t n_written;
    while (data_len > 0) {
        n_written = send(socket_fd, data, data_len, 0);
        if (n_written <= 0) {
            free(message);
            fprintf(stderr, "Failed to send request\n");
            return -1;
        }
        data += n_written;
        data_len -= n_written;
    }
    free(message);

    int BUFFER_SIZE = 4096;
    int HEADER_SIZE = BUFFER_SIZE;
    char *buffer = calloc(BUFFER_SIZE, sizeof(char));
    char *header = calloc(HEADER_SIZE, sizeof(char));

    int bytes_to_read = 1;
    int total_body_bytes_read = 0;
    char *header_location = NULL;
    int header_end_index = 0;
    bool found_header = false;
    while (bytes_to_read > 0) {
        int n = 0;

        // read header
        while (true) {
            // reset buffer
            bzero(buffer, BUFFER_SIZE);
            n = recv(socket_fd, buffer, BUFFER_SIZE, 0);
            printf("RECIEVED %d BYTES\n", n);
            printf("TOTAL BYTES READ: %d\n\n", total_body_bytes_read);

            total_body_bytes_read += n;

            if (n < 0) {
                free(buffer);
                free(header);
                return -1;
            }

            if (found_header) {
                break;
            }

            if (n == 0) {
                // end of data, no header found
                gfr->status = GF_INVALID;
                gfr->filelen = 0;
                break;
            }

            HEADER_SIZE += strlen(buffer) + 1;
            header = realloc(header, HEADER_SIZE * sizeof(char));
            strncat(header, buffer, strlen(buffer));
            printf("ACCUMULATED HEADER LENGTH: %d\n", (int) strlen(header));

            // try to extract header
            header_location = strstr(header, TERMINATOR);
            int file_len = 0;
            if (NULL != header_location) {
                found_header = true;
                header_end_index = header_location - header + strlen(TERMINATOR);
                printf("HEADER END INDEX: %d\n", header_end_index);

                printf("FOUND HEADER: %s\n", header);
                // header found
                char *status_str = calloc(15, sizeof(char));
                int num_parts = sscanf(header, "GETFILE %s %d\r\n\r\n", status_str, &file_len);
                if (num_parts == EOF) {
                    // failed to parse header
                    free(status_str);
                    gfr->status = GF_INVALID;
                    gfr->filelen = 0;
//                    gfr_malformed_header(gfr);
                    free(header);
                    free(buffer);
                    close(socket_fd);
                    return -1;
                } else if (num_parts < 2) {
                    gfstatus_t status = gfc_status_from_str(status_str);
                    gfr->status = status;
                    gfr->filelen = 0;
                    if (gfr->status == GF_INVALID) {
                        free(buffer);
                        free(header);
                        return -1;
                    }
//                    if (strcmp(status_str, "ERROR") == 0) {
//                        printf("HEADER IS ERROR\n");
//                        gfr_error_header(gfr);
//                    } else {
//                        gfr_malformed_header(gfr);
//                    }
//                    fflush(stdout);
                    free(status_str);
                    free(header);
                    free(buffer);
                    close(socket_fd);
                    return 0;
//                    break;
                } else {
                    gfstatus_t status = gfc_status_from_str(status_str);
                    if (status != GF_OK) {
                        printf("STATUS IS INVALID OO\n");
                        gfr->status = GF_INVALID;
                        gfr->filelen = 0;
//                        fflush(stdout);
                        free(buffer);
                        free(header);
                        return -1;
                    }

                    gfr_ok(gfr, gfc_status_from_str(status_str), (size_t) file_len);
                    bytes_to_read = file_len;
                    printf("BYTES TO READ NOW AT: %d\n", file_len);
                    if (NULL != gfr->headerfunc) {
                        gfr->headerfunc(header, strlen(header), gfr->headerarg);
                    }
                    total_body_bytes_read -= header_end_index;
                    free(status_str);
                }

                printf("REM LENGTH: %d\n", HEADER_SIZE - header_end_index);
                int rem_len = HEADER_SIZE - header_end_index;
                if (rem_len > 0) {
                    char *rem = calloc(rem_len, sizeof(char));
                    for (int i = 0; i < rem_len; i++) {
                        rem[i] = header[header_end_index + i];
                    }
                    printf("STRLEN REM: %d\n", (int) strlen(rem));
//                    total_body_bytes_read += strlen(rem);
                    gfr->bytesrecieved = total_body_bytes_read;
                    gfr->writefunc(rem, strlen(rem), gfr->writearg);
//                    printf("REM IS: %s\n", rem);
                    free(rem);
//                    if (total_body_bytes_read >= file_len) {
//                        fflush(stdout);
//                        close(socket_fd);
//                        free(header);
//                        free(buffer);
//                        return 0;
//                    }
                }

//                break;
                continue;
            }
        }

        if (n == 0) {
            printf("N IS 0\n");
            if (!found_header) {
//                fflush(stdout);
                close(socket_fd);
                free(buffer);
                free(header);
                return -1;
            } else if (total_body_bytes_read < gfr->filelen) {
                close(socket_fd);
                free(buffer);
                free(header);
                return -1;
            }
            break;
        }

        if (gfr->writefunc != NULL && gfr->status == GF_OK) {
            bytes_to_read -= n;
            printf("BYTES TO READ NOW AT: %d\n", bytes_to_read);
//            gfr->bytesrecieved = (size_t)(total_body_bytes_read - header_end_index);
            gfr->bytesrecieved = total_body_bytes_read;
            gfr->writefunc(buffer, n, gfr->writearg);
//            printf("WRITING BODY: %s\n", buffer);
        } else {
            break;
        }
    }

    fflush(stdout);
    close(socket_fd);
    free(header);
    free(buffer);
    return 0;
}

void gfc_set_headerarg(gfcrequest_t *gfr, void *headerarg) {
    gfr->headerarg = headerarg;
}

void gfc_set_headerfunc(gfcrequest_t *gfr, void (*headerfunc)(void *, size_t, void *)) {
    gfr->headerfunc = headerfunc;
}

void gfc_set_path(gfcrequest_t *gfr, const char *path) {
    gfr->path = calloc(strlen(path) + 1, sizeof(char));
    if (NULL == path || NULL == gfr->path) {
        fprintf(stderr, "%s @ %d: failed to set path\n", __FILE__, __LINE__);
        exit(1);
    }
    strncpy(gfr->path, path, strlen(path));
    printf("SET PATH TO: %s\n", gfr->path);
}

void gfc_set_port(gfcrequest_t *gfr, unsigned short port) {
    gfr->port = port;
}

void gfc_set_server(gfcrequest_t *gfr, const char *server) {
    gfr->server = calloc(strlen(server) + 1, sizeof(char));
    if (NULL == server || NULL == gfr->server) {
        fprintf(stderr, "%s @ %d: failed to set server\n", __FILE__, __LINE__);
        exit(1);
    }
    strncpy(gfr->server, server, strlen(server));
    printf("SET SERVER TO: %s\n", gfr->server);
}

void gfc_set_writearg(gfcrequest_t *gfr, void *writearg) {
    gfr->writearg = writearg;
}

void gfc_set_writefunc(gfcrequest_t *gfr, void (*writefunc)(void *, size_t, void *)) {
    gfr->writefunc = writefunc;
}

const char *gfc_strstatus(gfstatus_t status) {
    const char *strstatus = NULL;

    switch (status) {
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

