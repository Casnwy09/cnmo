#include <stdio.h>
#include <string.h>
#include "SDL2/SDL_net.h"
#include "net.h"

int net_offline;
int net_current_tick;
int net_current_id;
UDPsocket net_socket;
UDPpacket * net_packet;
netcmd_t * safe_cmds;
extern net_recvfunc_t net_recvfuncs[];

void net_bind(int offline) {
    if (SDLNet_Init() == -1) {
        printf("Couldn't initialize the SDL_Net subsystem!\n");
        offline = 1;
    }
    if ((net_offline = offline) == 1)
        return;
    
    net_current_tick = 0;
    if ((net_socket = SDLNet_UDP_Open(0)) == NULL) {
        printf("Could not bind a socket!\n");
        net_offline = 1;
        return;
    }
    net_current_tick = 0;
    net_current_id = 0;

    net_packet = SDLNet_AllocPacket(NET_CMDSIZE);
    return;
}

void net_unbind(void) {
    netcmd_t * cmd, * next;
    cmd = safe_cmds;
    while (cmd != NULL) {
        next = cmd->next;
        free(cmd);
        cmd = next;
    }
    SDLNet_FreePacket(net_packet);
    SDLNet_UDP_Close(net_socket);
    SDLNet_Quit();
}

void net_send(int type, const void * buffer, int size, int safe, const netaddr_t * addr, int node) {
    netcmd_t cmd_buf;
    netcmd_t * cmd;
    if (net_offline)
        return;

    if (safe > 0)
        cmd = malloc(sizeof(netcmd_t));
    else
        cmd = &cmd_buf;
    memcpy(cmd->payload, buffer, (size > NET_PAYLOADSIZE ? NET_PAYLOADSIZE : size));
    cmd->safe = safe;
    cmd->node = node;
    cmd->type = type;
    cmd->id = net_current_id++;
    memcpy(net_packet->data, cmd, NET_CMDSIZE);
    net_packet->len = NET_CMDSIZE;
    SDLNet_Write32(&net_packet->address.host, &addr->host);
    SDLNet_Write16(&net_packet->address.port, &addr->port);
    SDLNet_UDP_Send(net_socket, -1, net_packet);
    if (safe > 0) { 
        /* Instead of setting these fields to info about us, the sender, make it about who we are sending
         * the data to instead, so that we can resend this packet if we don't get an ack. */
        cmd->node = node;
        memcpy(&cmd->sender, addr, sizeof(netaddr_t));
        cmd->last = NULL;
        cmd->next = safe_cmds;
        if (safe_cmds != NULL)
            safe_cmds->last = cmd;
        safe_cmds = cmd;
    }
}

int net_recv(void) {
    netcmd_t * rover;
    netcmd_t cmd;
    int recieved;
    if (net_offline)
        return;

    recieved = 0;
    for (recieved = 0; recieved < 144 && SDLNet_UDP_Recv(net_socket, net_packet) == 1; recieved++) {
        memset(&cmd, 0, sizeof(cmd));
        memcpy(&cmd, net_packet->data, net_packet->len);
        cmd.sender.host = SDLNet_Read32(net_packet->address.host);
        cmd.sender.port = SDLNet_Read16(net_packet->address.port);
        cmd.next = NULL;
        cmd.last = NULL;
        
        if (cmd.type == CMDTYPE_NULL)
            continue;
        if (cmd.safe) {
            if (cmd.type == CMDTYPE_ACK) {
                /* Erase the safe cmd from the safe cmds */
                for (rover = safe_cmds; rover != NULL; rover = rover->next) {
                    if (rover->id == cmd.id) {
                        if (rover->next)
                            rover->next->last = rover->last;
                        if (rover->last)
                            rover->last->next = rover->next;
                        else
                            safe_cmds = NULL;
                        free(rover);
                        break;
                    }
                }
            }
            else {
                /* Resend the command to the sender */
                net_send(CMDTYPE_ACK, cmd.payload, NET_PAYLOADSIZE, -1, &cmd.sender, cmd.node);
            }
        }
        else
            net_recvfuncs[cmd.type - CMDTYPE_ACK - 1](&cmd);
    }
}

void net_tick(void) {
    netcmd_t * cmd;
    
    net_current_tick++;
    if (net_current_tick % 16 == 0)
        for (cmd = safe_cmds; cmd != NULL; cmd = cmd->next)
            net_send(cmd->type, cmd->payload, NET_PAYLOADSIZE, -1, &cmd->sender, cmd->node);
}

void net_addrstr(netaddr_t * addr, const char * host, unsigned short port) {
    IPaddress ip;
    if (SDLNet_ResolveHost(&ip, host, port) == -1)
        printf("Could not resolve host %s, on port %d!\n", host, port);
    addr->host = SDLNet_Read32(ip.host);
    addr->port = port;
}