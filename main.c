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
 * @brief Zpracovani argumentu programu
 *
 * @param argv  Pocet argumentu
 * @param args  Pole argumentu
 * @param port  Vybrany port
 * @return Nenulova hodnota pri chybe
 */
int argument_handler(int argc, char *argv[], uint16_t *port) {
    // podminka pri nezadani zadneho argumentu
    if (argc < 2)
        ERROR_MSG(1, "Pozadovan port pro naslochani v argumentu\n" "Pouziti:\n./hinfosvc [port]\n");
    // nacteni portu pro naslouchani
    *port = strtol(argv[1], NULL, 10);
    return 0;
}

/**
 * @brief Rizena komunikace mezi serverem a klientem
 *
 * @param client_socket_fd Cislo klientova prirazeneho socketu
 */
int message_handling(int client_socket_fd) {
    int retval = 0;
    const unsigned msg_len = 2048;
    char recv_msg[msg_len], request[msg_len];

    // cteni zpravy od uzivatele
    read(client_socket_fd, recv_msg, msg_len);
#ifdef DEBUG
    printf("%s\n", recv_msg);
#endif
    // nacteni pozadavku
    sscanf(recv_msg, "GET /%s HTTP", request);

    if (!strcmp(request, "hostname")) {
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

    // zpracovani argumentu
    if ( (retval = argument_handler(argc, argv, &port)) )
        return retval;
#ifdef DEBUG
    printf("Port: %d\n", port);
#endif 

    // vytvoreni socketu
    if ((server_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        ERROR_MSG(1, "Chyba pri tvorbe socketu\n");
#ifdef DEBUG
    puts("Socket uspesne vytvoren\n");
#endif

    INIT_SOCKADDR(address, port);

    // pripojeni serveru na port
    if (bind(server_socket_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
        ERROR_MSG(1, "Chyba pri pripojovani serveru na port\n");
# if DEBUG
    printf("Server uspesne pripojen na port %d\n", port);
#endif

    // naslouchani portu
    if (listen(server_socket_fd, 3) < 0)
        ERROR_MSG(1, "Chyba naslouchani portu\n");
#ifdef DEBUG
    puts("Cekani na pripojeni...");
#endif

    // cekani na pripojeni clienta
    unsigned addrlen = sizeof(struct sockaddr_in);

    while ((client_socket_fd = accept(server_socket_fd,
                    (struct sockaddr *) &client,
                    (socklen_t *) &addrlen))) {
#ifdef DEBUG
        puts("Connection accepted");
#endif

        // komunikace serveru s klientem
        retval = message_handling(client_socket_fd);
    }
    if (client_socket_fd < 0)
        ERROR_MSG(1, "Chyba pripojeni klienta\n");

    // uzavreni socketu
    close(server_socket_fd);
    return retval;
}

/* main.c */
