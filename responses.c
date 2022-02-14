#include "responses.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <time.h>
#include <string.h>

#define MSG_LEN 1024
#define NOF_CPU_VALS 10
#define RESPONSE_HEADER_FORMAT \
"HTTP/1.1 %d %s\n"      /* code, state*/    \
"Date: %s\n"            /* date */          \
"Server: Apache\n"                          \
"Last-Modified: %s\n"   /* date */          \
"Content-Length: %d\n"  /* nof bytes */     \
"Content-Type: text/plain\n"                \
"\n"                                        \
"%s"                    /* response */

static int send_response(int client_socket_fd, char *response, int code, char *state) {
    time_t t = time(NULL);
    struct tm time_info = *localtime(&t);
    char reply[MSG_LEN] = { 0, };
    char time_buffer[80] = { 0, };
    int response_len = strlen(response);

    // nastaveni formatu data
    strftime(time_buffer, 80, "%a, %d %b %Y %H:%M:%S %Z", &time_info);

    snprintf(reply, MSG_LEN, 
            RESPONSE_HEADER_FORMAT,
            code, state, time_buffer, time_buffer,
            response_len, response);

    write(client_socket_fd, reply, MSG_LEN);
    return 0;
}

int get_hostname(int client_socket_fd) {
    FILE *file = NULL;
    if ((file = fopen("/proc/sys/kernel/hostname", "r")) == NULL)
        return 1;

    char response[MSG_LEN] = "";

    // precteni jmena hosta
    fgets(response, MSG_LEN, file);

    // odeslani klientovi a zavreni souboru
    send_response(client_socket_fd, response, 200, "OK");
    fclose(file);
    return 0;
}

int get_cpu_name(int client_socket_fd) {
    // zjistuji pres popen, kde volam:
    FILE *file = NULL;
    char file_output[MSG_LEN] = "";
    char response[MSG_LEN]  = "";

    if ((file = fopen("/proc/cpuinfo", "rb")) == NULL)
        return 1;

    // ekvivalent grep; cte soubor, dokud nenarazi na radek zacinajici:
    // model name : ...
    while (fgets(file_output, MSG_LEN, file) != NULL) {
        if (sscanf(file_output, "model name : %[^\n]s", response) > 0)
            break;
    }
    strcat(response, "\n");

    // odeslani klientovi a zavreni souboru
    send_response(client_socket_fd, response, 200, "OK");
    fclose(file);
    return 0;
}

static int get_cpu_data(unsigned *cpu_vals) {
    FILE *file = NULL;

    // kontrola chyby pri otevirani souboru /proc/stat
    if ((file = fopen("/proc/stat", "r")) == NULL)
        return 1;

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

// spocita zatizeni procesoru
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

    if (get_cpu_data(prev_cpu_vals))    return 1;
    sleep(1);   // cekani jedne sekundy
    if (get_cpu_data(cpu_vals))         return 1;

    snprintf(response, MSG_LEN, "%g%%\n", 
             calculate_load(prev_cpu_vals, cpu_vals));

    // odeslani klientovi a zavreni souboru
    send_response(client_socket_fd, response, 200, "OK");
    return 0;
}

int get_bad_request(int client_socket_fd) {
    send_response(client_socket_fd, "", 400, "Bad Request");
    return 0;
}

/* responses.c */
