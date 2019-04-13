#ifndef CUSTOM_SOCKET_H
#define CUSTOM_SOCKET_H

#if defined _WIN64 || defined _WIN32
    #include <winsock2.h>
    #include <windows.h>
    #include <ws2tcpip.h>
#else
    #include <unistd.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

struct custom_socket{
    SOCKET ClientSocket;
    SOCKET ListenSocket;
    uint16_t Port;
    uint16_t BufferSize;
    uint16_t ThreadSleepingTime;
    bool Connected;
    bool Stopped;
    char* Packet;
};

int init_custom_socket(struct custom_socket *CustomSocket, uint16_t Port, uint16_t BufferSize, uint16_t ThreadSleepingTime);
int accept_and_stream_custom_socket(struct custom_socket *CustomSocket);
int clean_custom_socket(struct custom_socket *CustomSocket);

#endif