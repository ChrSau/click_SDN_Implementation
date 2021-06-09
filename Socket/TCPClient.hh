#ifndef TCP_CLIENT_HH
#define TCP_CLIENT_HH

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>

#include "../OFHelper.hh"


// #include "../OFHelper.hh"


class TCPClient
{
  private:
    std::string address;
    int port;
    struct sockaddr_in server;

  public:
    int sock;
    TCPClient();
    bool setup(std::string address, int port);
    bool Send(uint8_t* data, size_t length);
    bool receive(uint8_t* buffer, int size, uint32_t *len);
    // std::string read();
    void exit();
};

#endif
