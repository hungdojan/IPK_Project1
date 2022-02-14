#ifndef _RESPONSES_H_
#define _RESPONSES_H_

/** Vycet typu CPU hodnost z /proc/stat */
enum cpu_value_types {
    USER,
    NICE,
    SYSTEM,
    IDLE,
    IOWAIT,
    IRQ,
    SOFTIRQ,
    STEAL,
    GUESS,
    GUESS_NICE,
    CPU_VALS_SIZE
};

/**
 * @brief Posila klientovi jmeno hosta
 *
 * @param client_socket_fd Cislo klientova socketu
 * @return Nenulove cislo v pripade chyby
 */
int get_hostname(int client_socket_fd);

/**
 * @brief Posila klientovi jmeno procesoru hosta
 *
 * @param client_socket_fd Cislo klientova socketu
 * @return Nenulove cislo v pripade chyby
 */
int get_cpu_name(int client_socket_fd);

/**
 * @brief Posila klientovi aktualni zatez systemu hosta
 *
 * @param client_socket_fd Cislo klientova socketu
 * @return Nenulove cislo v pripade chyby
 */
int get_load(int client_socket_fd);

/**
 * @brief Chybovy zprava pro klienta pri neznamem pozadavku
 *
 * @param client_socket_fd  Cislo klientova socketu
 * @return Nenulove cislo v pripade chyby
 */
int get_bad_request(int client_socket_fd);

#endif // _RESPONSES_H_
