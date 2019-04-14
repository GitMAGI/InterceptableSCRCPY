#include "custom/custom_socket.h"
#include "custom/log.h"
#include "custom/util.h"

#if defined _WIN64 || defined _WIN32
    #include <winsock2.h>
    #include <windows.h>
    #include <ws2tcpip.h>
    #include <strings.h>
#else
    #include <unistd.h>
    #include <string.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <SDL2/SDL_thread.h>

int init_custom_socket(struct custom_socket *CustomSocket, uint16_t Port, uint16_t BufferSize, uint16_t ThreadSleepingTime){
    if(!CustomSocket){
        errorLog("CustomSocket cannot be NULL.");
        return -1;
    }
    
    int iResult;
    struct sockaddr_in serv_addr;

    CustomSocket->Port = Port;
    CustomSocket->ThreadSleepingTime = ThreadSleepingTime;
    CustomSocket->BufferSize = BufferSize;

    #if defined _WIN64 || defined _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2,2), &wsaData);
        if(iResult != 0){
            CustomSocket->Connected = false;
            CustomSocket->Stopped = true;
            criticalLog("WSAStartup failed with error: %d", iResult);
            return -1;
        }
    #endif

    // Create a SOCKET for connecting to server
    CustomSocket->ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (CustomSocket->ListenSocket == INVALID_SOCKET) {
        criticalLog("Listening Socket Creation failed");
        CustomSocket->Connected = false;
        CustomSocket->Stopped = true;
        #if defined _WIN64 || defined _WIN32            
            WSACleanup();
        #endif
        return -1;
    }

    // Setup the TCP listening socket
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(Port);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
    iResult = bind(CustomSocket->ListenSocket, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if (iResult < 0){
        criticalLog("Listening Socket bind failed with error: %d", iResult);
        CustomSocket->Connected = false;
        CustomSocket->Stopped = true;
        closesocket(CustomSocket->ListenSocket);
        #if defined _WIN64 || defined _WIN32            
            WSACleanup();
        #endif
        return -1;
    }
    
    iResult = listen(CustomSocket->ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        criticalLog("Listening Socket listen failed");
        CustomSocket->Connected = false;
        CustomSocket->Stopped = true;
        closesocket(CustomSocket->ListenSocket);
        #if defined _WIN64 || defined _WIN32            
            WSACleanup();
        #endif
        return -1;
    }

    CustomSocket->Connected = false;
    CustomSocket->Stopped = false;

    return 0;
}

int accept_and_stream_custom_socket(struct custom_socket *CustomSocket){
    if(!CustomSocket){
        errorLog("CustomSocket must be initialized.");
        return -1;
    }

    // Loop listening and pushing data
    do{
        struct timeval st_call, et_call;
        getTick(&st_call);

        CustomSocket->Connected = false;

        //Log listening on port and ip
        struct sockaddr_in serv_addr_;
        socklen_t serv_addr_len_ = sizeof(struct sockaddr_in);
        getsockname(CustomSocket->ListenSocket, (struct sockaddr *)&serv_addr_, &serv_addr_len_);
        char *serv_ip = inet_ntoa(serv_addr_.sin_addr);
        uint16_t serv_port = htons (serv_addr_.sin_port);
        infoLog("Host listening on %s:%d", serv_ip, serv_port);

        // Accept a client socket
        CustomSocket->ClientSocket = accept(CustomSocket->ListenSocket, NULL, NULL);
        if (CustomSocket->ClientSocket == INVALID_SOCKET) {
            criticalLog("Accepting Socket Accept failed");
            CustomSocket->Connected = false;
            closesocket(CustomSocket->ClientSocket);            
            break;
        }
        CustomSocket->Connected = true;

        //Log Accepted remote ip and port
        struct sockaddr_in clnt_addr_;
        socklen_t clnt_addr_len_ = sizeof(struct sockaddr_in);
        getsockname(CustomSocket->ClientSocket, (struct sockaddr *)&clnt_addr_, &clnt_addr_len_);
        char *clnt_ip = inet_ntoa(clnt_addr_.sin_addr);
        uint16_t clnt_port = htons (clnt_addr_.sin_port);
        infoLog("Accepted %s remote host on port %d", clnt_ip, clnt_port);

        
        uint16_t delay_ = CustomSocket->ThreadSleepingTime; 
        int iSendResult = 0;
        char *pckt = (char*) NULL;
        char *pcktsnt = (char*) NULL;
        do{
            if(!CustomSocket->Packet){
                //debugLog("Client Socket Packet is NULL");
                sleep_custom_socket(delay_);
                continue;
            }
            else{
                if(!pckt){
                    //debugLog("Client Socket pckt is NULL: allocating ...");
                    pckt = (char*) malloc(sizeof(char)*strlen(CustomSocket->Packet));
                    //debugLog("Client Socket pckt is NULL: allocated");
                }                    
                if(!pcktsnt){
                    //debugLog("Client Socket pcktsnt is NULL: allocating ...");
                    pcktsnt = (char*) malloc(sizeof(char)*strlen(CustomSocket->Packet));
                    //debugLog("Client Socket pcktsnt is NULL: allocated");
                }                    
            }

            memcpy(pckt, CustomSocket->Packet, sizeof(char)*strlen(CustomSocket->Packet)); 

            if(memcmp(pckt, pcktsnt, sizeof(char)*strlen(CustomSocket->Packet)) == 0){
                //debugLog("Client Socket Packet already sent");
                sleep_custom_socket(delay_);
                continue;
            }

            iSendResult = send( CustomSocket->ClientSocket, pckt, CustomSocket->BufferSize, 0 );
            if (iSendResult == SOCKET_ERROR) {
                errorLog("Client Socket Sending failed");
                CustomSocket->Connected = false;
                break;
            }
            else {
                //debugLog("Client Socket %d Bytes sent", iSendResult);
                memcpy(pcktsnt, pckt, sizeof(char)*strlen(CustomSocket->Packet));
            }

            sleep_custom_socket(delay_);
        } while(CustomSocket->Connected);

        // shutdown the connection since we're done
        int iResult = shutdown(CustomSocket->ClientSocket, SD_SEND);
        if (iResult == SOCKET_ERROR) {
            criticalLog("Client Socket Shutdown failed");
            closesocket(CustomSocket->ClientSocket);            
            break;
        }

        // cleanup
        closesocket(CustomSocket->ClientSocket);
        getTick(&et_call);
        infoLog("Disconnected remote host %s on port %d. ETA %s", clnt_ip, clnt_port, getETA(st_call, et_call));

    } while(!CustomSocket->Stopped);
}

int sleep_custom_socket(uint16_t delay_){
    #if defined _WIN64 || defined _WIN32
        Sleep(delay_);
    #else
        sleep(delay_);
    #endif

    return 0;
}

int clean_custom_socket(struct custom_socket *CustomSocket){
    if(CustomSocket){
        CustomSocket->Stopped = true;
        CustomSocket->Connected = false;

        // cleanup
        if(CustomSocket->ListenSocket)
            closesocket(CustomSocket->ListenSocket);
    }
    #if defined _WIN64 || defined _WIN32
        WSACleanup();
    #endif
}