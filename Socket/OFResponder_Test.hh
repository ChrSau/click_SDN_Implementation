#ifndef CLICK_OFRESPONDER__HH
#define CLICK_OFRESPONDER__HH

// #include <click/element.hh>
// #include <click/timer.hh>
#include "../openflow.h"
#include <vector>
#include <string>
#include <array>
#include <iostream>
#include <sys/types.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <mutex>
#include "OFSwitchDesc.hh"
#include "../OFHelper.hh"
#include "Packet.hh"
#include "OFPipeline.hh"
#include "TCPClient.hh"

// CLICK_DECLS

/*
=c

VLCTrafficLightDirectControll(DB,AD)

=s

TODO:
Initializes and maintains initilization of a connected VLC-module.
Sends the initializatione message with TICK period.
DB gives debug level and number of printed debug messages
AD is the local address.
The packet creates a CamKin-Parameter-Update-Message and does not require the VLC-Wrapper.
However, the CanKinChecksumAdder is still needed.

=d

---

*/

class OFResponder
{
private:
    int _DBlevel;
    int _AD;
    int headroom;
    class myPacket *lastMessage;
    std::vector<ofp_multipart_request> multipartMsg;
    int _outputPortCount;
    class OFSwitchDesc *switchDesc;
    uint8_t OFVersion = OFP_VERSION;

    std::queue<myPacket *> *socketQueue;
    std::mutex *queueLock;

    OFPipeline *pipeline;
    TCPClient *tcp;

    uint16_t switchStatusFlags;

    std::vector<OF::Structs::controllerData> controllers;

public:
    OFResponder(OFPipeline *, TCPClient *, std::queue<myPacket *> *socketQueue, std::mutex *queueLock);
    ~OFResponder();

    const char *class_name() const { return "OFResponder"; }
    const char *port_count() const { return "1/3"; }
    const char *processing() const { return "h/h"; }
    myPacket *simple_action(myPacket *);
};

// CLICK_ENDDECLS

#endif