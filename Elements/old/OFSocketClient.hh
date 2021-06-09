#ifndef CLICK_OFSOCKETCLIENT__HH
#define CLICK_OFSOCKETCLIENT__HH 1

#include <iostream>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if.h>
#include <thread>

// Click Includes
#include <click/config.h>

#include "../Utilities/OFFunctions.hh"
#include "OFResponder.hh"

CLICK_DECLS

class OFSocketClient
{
private:
    std::string address;
    int port;
    struct sockaddr_in server;
    int sock;
    std::string interfaceName;
    static const int default_snaplen = 2048;
    static const int default_headroom = Packet::default_headroom;

    class OFResponder *responder;

public:
    OFSocketClient();
    OFSocketClient(OFResponder *responder);
    OFSocketClient(std::string address, int port, std::string interfaceName, OFResponder *responder);
    OFSocketClient(std::string address, int port, int sock, std::string interfaceName, OFResponder *responder);
    OFSocketClient(OFSocketClient &in);
    ~OFSocketClient();

    bool connectToRemote(std::string address, int port, std::string interfaceName);
    bool connectToRemote();
    bool Send(uint8_t *data, size_t length);
    bool Send(Packet *p);
    void receive();
    void exit();

    std::string getAddress();
    int getPort();
    sockaddr_in getServer();
    int getSock();
    OFResponder *getResponder();
    std::string getInterfaceName();
};

CLICK_ENDDECLS

#endif
