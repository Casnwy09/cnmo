#ifndef _net_h
#define _net_h
#include <stddef.h>

#define NET_CMDSIZE offsetof(netcmd_t, sender)
#define NET_PAYLOADSIZE (256 - sizeof(int) * 2)

typedef enum cmdtype {
    CMDTYPE_NULL,
    CMDTYPE_ACK,
    CMDTYPE_MAX
} cmdtype_t;

typedef struct netaddr {
    unsigned int host;
    unsigned short port;
} netaddr_t;

typedef struct netcmd {
    int type : 15;
    int safe : 1;
    int node : 16;
    int id;
    char payload[NET_PAYLOADSIZE];

    netaddr_t sender;
    netcmd_t * next;
    netcmd_t * last;
} netcmd_t;

typedef void(*net_recvfunc_t)(const netcmd_t * cmd);

void net_bind(int offline);

void net_unbind(void);

void net_send(int type, const void * buffer, int size, int safe, const netaddr_t * addr, int node);

int net_recv(void);

void net_tick(void);

void net_addrstr(netaddr_t * addr, const char * host, unsigned short port);

#endif