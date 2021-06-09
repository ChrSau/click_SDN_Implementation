#ifndef CLICK_OFRESPONDER__HH
#define CLICK_OFRESPONDER__HH 1

#include <iostream>
#include <vector>
#include <string>
#include <array>
#include <stdint.h>

#include <click/config.h>
#include <click/element.hh>
#include <click/args.hh>
#include <click/router.hh>


#include "OFSwitchDesc.hh"
#include "../Utilities/OFFunctions.hh"
#include "../Utilities/OFStructs.hh"
#include "../Utilities/openflow.h"

CLICK_DECLS

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

class OFResponder : public Element
{
private:
    int _DBlevel = 0;
    int _AD;
    int _headroom;
    class Packet *_lastMessage = nullptr;
    std::vector<ofp_multipart_request> _multipartMsg;
    int _outputPortCount;
    class OFSwitchDesc *_switchDesc;
    uint8_t _OFVersion = OFP_VERSION;

    // TODO: sollten in extra Klassen ausgelagert werden.
    uint16_t _switchStatusFlags;

    std::vector<OF::Structs::controllerData> _controllers;
    std::vector<class OFSocketClient *> _sockets;
    // std::vector<std::thread> _socketThreads;

    bool run = true;

public:
    OFResponder();
    ~OFResponder();

    const char *class_name() const { return "OFResponder"; }
    const char *port_count() const { return "2-/4-"; }
    const char *processing() const { return "h/hhhl"; }
    const char *flow_code() { return "x/x"; }
    virtual void cleanup(CleanupStage) CLICK_COLD;

    int initialize(ErrorHandler *errh);
    int configure(Vector<String> &conf, ErrorHandler *errh);
    void push(int inPort, Packet *p);
    Packet *pull(int port);
    bool sendPacketToSocket(Packet *p, uint32_t portNo);

    bool getRun();
};

CLICK_ENDDECLS

#endif