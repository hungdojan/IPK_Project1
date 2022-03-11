#ifndef _RESPONSES_H_
#define _RESPONSES_H_

/** Enumeration CPU values from /proc/stat */
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
 * @brief Send system's hostname to client
 *
 * @param client_socket_fd Client socket file descriptor
 * @return Non-zero value when error occure
 */
int get_hostname(int client_socket_fd);

/**
 * @brief Send system's CPU name to client
 *
 * @param client_socket_fd Client socket file descriptor
 * @return Non-zero value when error occure
 */
int get_cpu_name(int client_socket_fd);

/**
 * @brief Send system's load to client
 *
 * @param client_socket_fd Client socket file descriptor
 * @return Non-zero value when error occure
 */
int get_load(int client_socket_fd);

/**
 * @brief Handle bad request
 *
 * @param client_socket_fd Client socket file descriptor
 * @return Non-zero value when error occure
 */
int get_bad_request(int client_socket_fd);

#endif // _RESPONSES_H_
