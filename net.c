#include <stdio.h>
#include "net.h"

#define NET_PORT (*(unsigned short * )"co")

int net_offline;
net_socket_t net_socket;
netcmd_t * safe_cmds;

void net_bind(int offline) {
    unsigned short port;
    struct sockaddr_in sock;

    if ((net_offline = offline) == 1)
        return;
    if ((net_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        goto failure;

    memset(&sock, 0, sizeof(sock));
    sock.sin_family = AF_INET;
    sock.sin_addr.s_addr = INADDR_ANY;
    sock.sin_port = htons(port);
    while (bind(net_socket, &sock, sizeof(sock)) == -1)
        sock.sin_port = htons(++port);
    printf("Binded socket to port number %d!\n", port);
    return;

failure:
    printf("Network binding failure!\n");
    net_offline = 1;
    return;
}

void net_close(void) {
    netcmd_t * cmd;
    for (cmd = safe_cmds; cmd != NULL; cmd = cmd->next)
        free(cmd);
    close(net_socket);
}

void net_send(int type, const void * buffer, int size, int safe, const struct sockaddr_in * addr, int node) {
    netcmd_t * cmd;
    if (net_offline)
        return;

    cmd = malloc(sizeof(netcmd_t));
    memcpy(cmd->payload, buffer, (size > NET_PAYLOADSIZE ? NET_PAYLOADSIZE : size));
    cmd->safe = safe;
    cmd->node = node;
    cmd->type = type;
    sendto(net_socket, &cmd, NET_CMDSIZE, 0, addr, sizeof(struct sockaddr_in));
    if (safe) {
        cmd->last = NULL;
        cmd->next = safe_cmds;
        if (safe_cmds != NULL)
            safe_cmds->last = cmd;
        safe_cmds = cmd;
    }
    else
        free(cmd);
}

int net_recv(void) {
    netcmd_t cmd;
    int from_len;
    struct sockaddr_in from;
    if (net_offline)
        return;

    recvfrom(net_socket, &cmd, NET_CMDSIZE, 0, (struct sockaddr * )&from, &from_len);
    
}