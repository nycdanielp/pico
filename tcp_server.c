#include "tcp_server.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#define TCP_PORT 1234
#define BUF_SIZE 256

static struct tcp_pcb *server_pcb;
static struct tcp_pcb *client_pcb = NULL;
static char line_buffer[BUF_SIZE];
static uint16_t line_pos = 0;

extern void cli_process_command(const char *cmd); // Implemented in cli.c

// Allows CLI to print back to the TCP client
void tcp_printf(const char *fmt, ...) {
    if (!client_pcb) return;

    char buf[BUF_SIZE];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    tcp_write(client_pcb, buf, strlen(buf), TCP_WRITE_FLAG_COPY);
    tcp_output(client_pcb);
}

static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (!p) {
        // Connection closed
        tcp_close(tpcb);
        client_pcb = NULL;
        return ERR_OK;
    }

    char *data = (char *)p->payload;
    for (uint16_t i = 0; i < p->len; i++) {
        char c = data[i];
        if (c == '\r' || c == '\n') {
            if (line_pos > 0) {
                line_buffer[line_pos] = '\0';
                cli_process_command(line_buffer); // Parse command
                line_pos = 0;
            }
            tcp_printf("NanoOS> "); // Prompt
        } else if (line_pos < BUF_SIZE - 1) {
            line_buffer[line_pos++] = c;
        }
    }

    tcp_recved(tpcb, p->tot_len);
    pbuf_free(p);
    return ERR_OK;
}

static err_t tcp_server_accept(void *arg, struct tcp_pcb *client, err_t err) {
    if (err != ERR_OK || client == NULL) return ERR_VAL;
    
    // Accept only one client at a time for simplicity
    if (client_pcb != NULL) {
        tcp_abort(client);
        return ERR_ABRT;
    }

    client_pcb = client;
    tcp_recv(client, tcp_server_recv);
    
    tcp_printf("\r\n--- Welcome to NanoOS (Pico 2 W) ---\r\nNanoOS> ");
    return ERR_OK;
}

void tcp_server_init(void) {
    server_pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!server_pcb) return;

    if (tcp_bind(server_pcb, IP_ANY_TYPE, TCP_PORT) != ERR_OK) return;

    server_pcb = tcp_listen(server_pcb);
    tcp_accept(server_pcb, tcp_server_accept);
}
