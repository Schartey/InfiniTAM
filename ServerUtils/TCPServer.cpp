#include "TCPServer.h"

#include <memory.h>

#define DEFAULT_BUFLEN 1024
#define MESSAGE_LENGTH_IND 4

namespace ServerUtils
{
    TCPServer::TCPServer(std::string host_, int port_) : host(host_), port(port_)
    {
        ServerSocket = INVALID_SOCKET;
    }

    int TCPServer::Init()
    {
        int iResult;

    #ifdef _WIN32

        // Initialize Winsock
        iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (iResult != 0) {
            printf("WSAStartup failed: %d\n", iResult);
            return 1;
        }
        printf("Socket Startup successful\n");
    #endif
        // Create a SOCKET for the server to listen for client connections

        ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        if (ServerSocket == INVALID_SOCKET) {
    #ifdef _WIN32
            //freeaddrinfo(result);
            WSACleanup();
    #endif
            return 1;
        }

        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        iResult = bind(ServerSocket, reinterpret_cast<sockaddr*>(&address), sizeof(address));
        if (iResult == SOCKET_ERROR) {
            PrintError("Bind Failed");
            Close(ServerSocket);
            return 1;
        }
        printf("Server Socket Bound.\n");

        if (listen(ServerSocket, SOMAXCONN) == SOCKET_ERROR) {
            PrintError("Listen failed");
            Close(ServerSocket);
        }

        printf("Server Socket waiting for Connections.\n");

        running = true;

        ListenThread = std::thread(&TCPServer::Listen, this);

        return 0;
    }

    void TCPServer::Listen()
    {
        while (running)
        {
            // Accept a client socket
            SOCKET ClientSocket = accept(ServerSocket, NULL, NULL);

            if (ClientSocket == INVALID_SOCKET) {
                PrintError("Accept failed");
                break;
            }
            else {
                printf("Client connected");

                Send(ClientSocket, greetingMessage.c_str());

                TCPSocket* SocketWrapper = new TCPSocket();
                SocketWrapper->socket = ClientSocket;
                std::thread socketThread(&TCPServer::SocketReceive, this, SocketWrapper);
                socketThread.detach();
                SocketWrapper->socketThread = &socketThread;

                ClientSockets.push_back(SocketWrapper);
            }
        }
    }

    void TCPServer::SocketReceive(TCPSocket* socket)
    {
        unsigned long iResult;
        char recvbuf[DEFAULT_BUFLEN];
        int recvbuflen = DEFAULT_BUFLEN;
        std::vector<char> buffer = socket->buffer;

        do {
            iResult = recv(socket->socket, recvbuf, recvbuflen, 0);

            if (iResult > 0) {

                char* data = new char[iResult];
                memcpy(data, &recvbuf, iResult);
                buffer.insert(buffer.end(), data, data + iResult);

                // If buffer and received length is at least as great  as length indicator
                if (!(buffer.size() < MESSAGE_LENGTH_IND))
                {
                    unsigned long msgSize = 0;//short(static_cast<short>(buffer[0]) << 8*(MESSAGE_LENGTH_IND-1));
                    for (int i = 0; i < MESSAGE_LENGTH_IND; i++)
                    {
                        msgSize = msgSize | buffer[i] << 8 * (MESSAGE_LENGTH_IND - (i + 1));
                    }
                    printf("Data: %d", msgSize);

                    // While we have full messages with length indicator and PGM Metadata in the buffer
                    while (buffer.size() >= MESSAGE_LENGTH_IND + msgSize)
                    {
                        char* message = new char[msgSize];
                        memcpy(message, &(buffer[MESSAGE_LENGTH_IND]), msgSize * sizeof(char));
                        buffer.erase(buffer.begin(), buffer.begin() + MESSAGE_LENGTH_IND + msgSize);

                        this->ProcessMsg(socket, message, msgSize);
                    }
                }
            }
            else if (iResult == 0) {
                printf("Connection closing...\n");
                break;
            }
            else {
                PrintError("Recv failed");
                break;
            }
        } while ((iResult > 0) && running);

        Close(socket->socket);
    }

    void TCPServer::Send(SOCKET ClientSocket, const char* data)
    {
        int iSendResult;
        printf("Bytes : %d\n", strlen(data));

        iSendResult = send(ClientSocket, data, strlen(data), 0);
        if (iSendResult == SOCKET_ERROR) {
            PrintError("Send failed");
            Close(ClientSocket);
        }
        printf("Text sent: %s - Bytes : %d\n", data, iSendResult);
    }

    void TCPServer::Close(SOCKET Socket)
    {
        int iResult;
    #ifdef _WIN32
        iResult = closesocket(Socket);
    #else
        iResult = shutdown(Socket, SHUT_RD);
    #endif
        if (iResult == SOCKET_ERROR)
            PrintError("Socket closing failed");
    #ifdef _WIN32
        WSACleanup();
    #endif
    }

    void TCPServer::ProcessMsg(TCPSocket* socket, char* msg, unsigned long length)
    {
        printf("Msg: %s\n", msg);
    }

    void TCPServer::PrintError(const char* msg)
    {
    #ifdef _WIN32
        printf("Error: %s - %ld\n", msg, WSAGetLastError());
    #else
        printf("Error: %s - %s\n", msg, strerror(errno));
    #endif
    }

    bool TCPServer::IsRunning()
    {
        return running;
    }

    void TCPServer::SetGreetingMessage(std::string message)
    {
        this->greetingMessage = message;
    }

    TCPServer::~TCPServer()
    {
        Close(this->ServerSocket);
        this->ListenThread.join();

        for (TCPSocket* tcpSocket : this->ClientSockets) {
            Close(tcpSocket->socket);
        }
    }
}
