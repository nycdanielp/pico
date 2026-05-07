#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

// Common settings used in most pico_w examples
#define NO_SYS                      1
#define LWIP_SOCKET                 0
#define LWIP_COMPAT_SOCKETS         0
#define LWIP_NETCONN                0
#define LWIP_NETIF_TX_SINGLE_PBUF   1

#define LWIP_TCP                    1
#define TCP_MSS                     1460
#define TCP_WND                     (8 * TCP_MSS)
#define TCP_SND_BUF                 (8 * TCP_MSS)
#define TCP_SND_QUEUELEN            ((4 * (TCP_SND_BUF) + (TCP_MSS - 1)) / (TCP_MSS))

#define LWIP_ND6_NUM_ROUTERS        0
#define LWIP_ND6_NUM_PREFIXES       0

#define LWIP_IGMP                   0
#define LWIP_MDNS_RESPONDER         0

#define LWIP_NUM_NETIF_CLIENT_DATA  0

#define MEM_LIBC_MALLOC             0
#define MEM_ALIGNMENT               4
#define MEM_SIZE                    4000
#define MEMP_NUM_TCP_PCB            5
#define MEMP_NUM_TCP_PCB_LISTEN     1
#define MEMP_NUM_TCP_SEG            32
#define MEMP_NUM_SYS_TIMEOUT        10

#define LWIP_DEBUG                  0

#endif /* __LWIPOPTS_H__ */
