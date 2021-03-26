#ifndef _net_h
#define _net_h
#ifdef _WIN32

#include <winsock2.h>
#include <windows.h>
#define close closesocket
#define BAD_SOCKET INVALID_SOCKET
typedef SOCKET net_socket_t;

#else

#include <netdb.h>
#include <unistd.h>
#define BAD_SOCKET -1
typedef int net_socket_t;

#endif

#include <stddef.h>

#define NET_CMDSIZE offsetof(netcmd_t, sender)
#define NET_PAYLOADSIZE 4096

typedef enum cmdtype {
    CMDTYPE_ACK,
    CMDTYPE_MAX
} cmdtype_t;

typedef struct netcmd {
    int type : 15;
    int safe : 1;
    int node : 16;
    char payload[NET_PAYLOADSIZE];

    struct sockaddr_in sender;
    netcmd_t * next;
    netcmd_t * last;
} netcmd_t;

typedef void(*net_recv_callback_t)(const netcmd_t * cmd);

void net_bind(int offline);

void net_close(void);

void net_send(int type, const void * buffer, int size, int safe, const struct sockaddr_in * addr, int node);

int net_recv(void);

#endif