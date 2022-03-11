#include "responses.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>

#define MSG_LEN 1024
#define NOF_CPU_VALS 10
#define RESPONSE_HEADER_FORMAT \
"HTTP/1.1 %d %s\r\n"      /* code, state*/ \
"Connection: close\r\n"                    \
"Content-Length: %d\r\n"  /* nof bytes */  \
"Content-Type: text/plain\r\n"             \
"\r\n"                                     \
"%s"                      /* response */

static int send_response(int client_socket_fd, char *response, int code, char *state) {
    char reply[MSG_LEN] = { 0, };
    int response_len = strlen(response);

    // build string message
    snprintf(reply, MSG_LEN, 
            RESPONSE_HEADER_FORMAT,
            code, state, response_len, response);

    // send response
    write(client_socket_fd, reply, MSG_LEN);
    return 0;
}

int get_hostname(int client_socket_fd) {
    FILE *file = NULL;
    if ((file = fopen("/proc/sys/kernel/hostname", "r")) == NULL)
        return 1;

    char response[MSG_LEN] = "";

    // get hostname from the file
    fgets(response, MSG_LEN, file);

    // send response and close file
    send_response(client_socket_fd, response, 200, "OK");
    fclose(file);
    return 0;
}

int get_cpu_name(int client_socket_fd) {
    FILE *file = NULL;
    char file_output[MSG_LEN] = "";
    char response[MSG_LEN]  = "";

    if ((file = fopen("/proc/cpuinfo", "rb")) == NULL)
        return 1;

    // equivalent of egrep "^model name: " command
    // append end-of-line character
    while (fgets(file_output, MSG_LEN, file) != NULL) {
        if (sscanf(file_output, "model name : %[^\n]s", response) > 0)
            break;
    }
    strcat(response, "\n");

    // send response and close file
    send_response(client_socket_fd, response, 200, "OK");
    fclose(file);
    return 0;
}

static int get_cpu_data(unsigned *cpu_vals) {
    FILE *file = NULL;

    if ((file = fopen("/proc/stat", "r")) == NULL)
        return 1;

    // extract data from file
    fscanf(file, "cpu %u %u %u %u %u %u %u %u %u %u",
           &cpu_vals[USER],
           &cpu_vals[NICE],
           &cpu_vals[SYSTEM],
           &cpu_vals[IDLE],
           &cpu_vals[IOWAIT],
           &cpu_vals[IRQ],
           &cpu_vals[SOFTIRQ],
           &cpu_vals[STEAL],
           &cpu_vals[GUESS],
           &cpu_vals[GUESS_NICE]);

    fclose(file);
    return 0;
}

// calculate cpu load
static double calculate_load(unsigned *prev_cpu_vals, unsigned *cpu_vals) {
    // FIXME:
    double prev_idle = prev_cpu_vals[IDLE] + prev_cpu_vals[IOWAIT];
    double prev_non_idle = prev_cpu_vals[USER] + prev_cpu_vals[NICE] + prev_cpu_vals[SYSTEM] +
                        prev_cpu_vals[IRQ] + prev_cpu_vals[SOFTIRQ] + prev_cpu_vals[STEAL];

    double idle = cpu_vals[IDLE] + cpu_vals[IOWAIT];
    double non_idle = cpu_vals[USER] + cpu_vals[NICE] + cpu_vals[SYSTEM] +
                        cpu_vals[IRQ] + cpu_vals[SOFTIRQ] + cpu_vals[STEAL];

    double prev_total = prev_idle + prev_non_idle;
    double total = idle + non_idle;

    double total_load = total - prev_total;
    double idle_load  = idle - prev_idle;
    return (total_load - idle_load) / total_load * 100;
}

int get_load(int client_socket_fd) {
    char response[MSG_LEN]  = "";
    unsigned prev_cpu_vals[CPU_VALS_SIZE] = { 0, };
    unsigned cpu_vals[CPU_VALS_SIZE] = { 0, };

    // get 2 loads from the system (one second apart)
    // and calculate current system's load
    if (get_cpu_data(prev_cpu_vals))    return 1;
    sleep(1);
    if (get_cpu_data(cpu_vals))         return 1;

    // build string message
    snprintf(response, MSG_LEN, "%g%%\n", 
             calculate_load(prev_cpu_vals, cpu_vals));

    // send response
    send_response(client_socket_fd, response, 200, "OK");
    return 0;
}

int get_bad_request(int client_socket_fd) {
    send_response(client_socket_fd, "", 400, "Bad Request");
    return 0;
}

/* responses.c */
