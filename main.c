#include <string.h>     // strlen
#include <stdio.h>      // puts, fprintf
#include <stdlib.h>     // strtol
#include <stdint.h>     // uint16_t
#include <arpa/inet.h>  // struct sockaddr_in, socket, bind, listen, accept, ...
#include <unistd.h>     // read, write
#include "responses.h"

#define INIT_SOCKADDR(addr, port) \
    do { \
    addr.sin_family = AF_INET; \
    addr.sin_addr.s_addr = INADDR_ANY; \
    addr.sin_port = htons(port); \
    } while (0)

#define ERROR_MSG(code, ... ) \
    do { \
        fprintf(stderr, __VA_ARGS__); \
        return code;                          \
    } while(0)

/**
 * @brief Arguments processing
 *
 * @param argc  Arguments counter
 * @param argv  Vector of arguments
 * @param port  User defined port [out]
 * @return Non-zero value when error occured
 */
int argument_handler(int argc, char *argv[], uint16_t *port) {
    // check for port argument
    if (argc < 2)
        ERROR_MSG(1, "Server port is required (value 1024 - 65536)\n"
                     "Usage: ./hinfosvc [port]\n");
    // load port from arguments
    *port = strtol(argv[1], NULL, 10);
    return 0;
}

/**
 * @brief Answer client's request
 *
 * @param client_socket_fd Client's socket file descriptor
 */
int message_handling(int client_socket_fd) {
    int retval = 0;
    const unsigned msg_len = 2048;
    char recv_msg[msg_len], request[msg_len], host[msg_len];

    // memory safe array initialization
    memset(recv_msg, 0, msg_len);
    memset(request, 0, msg_len);

    // load client's request
    read(client_socket_fd, recv_msg, msg_len);
#ifdef DEBUG
    printf("%s\n", recv_msg);
#endif

    // validate header and process response
    int nof_loaded_strings = sscanf(recv_msg, "GET /%s HTTP/1.1\r\nHost: %s", request, host);

    if (nof_loaded_strings < 2) {
        retval = get_bad_request(client_socket_fd);
    } else if (!strcmp(request, "hostname")) {
        retval = get_hostname(client_socket_fd);
    } else if (!strcmp(request, "cpu-name")) {
        retval = get_cpu_name(client_socket_fd);
    } else if (!strcmp(request, "load")) {
        retval = get_load(client_socket_fd);
    } else {    // neznamy prikaz
        retval = get_bad_request(client_socket_fd);
    }
    return retval;
}

int main(int argc, char *argv[]) {
    int retval;
    uint16_t port;
    int server_socket_fd, client_socket_fd;
    struct sockaddr_in address, client;

    // get port from arguments
    if ( (retval = argument_handler(argc, argv, &port)) )
        return retval;
#ifdef DEBUG
    printf("Port: %d\n", port);
#endif 

    // initialize socket
    if ((server_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        ERROR_MSG(1, "Socket initialization error\n");
#ifdef DEBUG
    puts("Socket initialization success\n");
#endif

    INIT_SOCKADDR(address, port);

    // bind socket to port
    if (bind(server_socket_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
        ERROR_MSG(1, "Socket binding error\n");
#ifdef DEBUG
    printf("Socket successfully bind to port %d\n", port);
#endif

    // list to port, queue size = 3
    if (listen(server_socket_fd, 3) < 0)
        ERROR_MSG(1, "Socket listening error\n");
#ifdef DEBUG
    puts("Waiting for connection...");
#endif

    // waiting for client's connection
    unsigned addrlen = sizeof(struct sockaddr_in);

    while ((client_socket_fd = accept(server_socket_fd,
                    (struct sockaddr *) &client,
                    (socklen_t *) &addrlen))) {
#ifdef DEBUG
        puts("Connection accepted");
#endif

        // communication handling
        retval = message_handling(client_socket_fd);
    }
    if (client_socket_fd < 0)
        ERROR_MSG(1, "Client's communication error\n");

    // close socket
    close(server_socket_fd);
    return retval;
}

/* main.c */
