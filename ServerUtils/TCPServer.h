#pragma once

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#endif

#include <thread>
#include <vector>
#include <atomic>


namespace ServerUtils
{
    struct TCPSocket
    {
        SOCKET socket;
        std::thread* socketThread;
        std::vector<char> buffer;
    };

    class TCPServer
    {
    private:
    #ifdef _WIN32
        WSADATA wsaData;
        struct addrinfo *result = NULL, *ptr = NULL;
    #else

    #endif
        //Not used yet
        std::string host;
        int port;
        sockaddr_in address;

        SOCKET ServerSocket;
        std::vector<TCPSocket*> ClientSockets;

        std::thread ListenThread;

        std::atomic<bool> running;
        int dataReceived = 0;
        bool awaitingBinary = false;

        std::string greetingMessage;

    private:
        void SocketReceive(TCPSocket* socket);

    protected:
        void Send(SOCKET ClientSocket, const char* data);
        void Close(SOCKET socket);
        void PrintError(const char* msg);

    public:
        TCPServer(std::string host_, int port_);
        virtual ~TCPServer();

        virtual int Init();

        void Listen();
        bool IsRunning();

        void SetGreetingMessage(std::string greetingMessage);

        virtual void ProcessMsg(TCPSocket* socket, char* msg, unsigned long length);
    };
}
