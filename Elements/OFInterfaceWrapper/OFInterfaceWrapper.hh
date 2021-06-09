#ifndef CLICK_OFINTERFACEWRAPPER__HH
#define CLICK_OFINTERFACEWRAPPER__HH 1

#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <cstring>

#include <click/config.h>
#include <click/element.hh>
#include <click/etheraddress.hh>
#include <click/args.hh>
#include <click/router.hh>

#include "../Utilities/openflow.h"

#include "../Utilities/OFFunctions.hh"
#include "../Utilities/OFStructs.hh"
// #include "OFSocketClient.hh"

#include <sys/socket.h>
#include <linux/if_packet.h>

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

class OFInterfaceWrapper : public Element
{
private:
    struct OF::Structs::my_ofp_port *_portStruct;
    struct OF::Structs::my_ofp_port_counter *_portCounter;

    // std::queue<Packet *> _outQueue;

    int _DBlevel = 0;
    std::string _ifname;

    Timer _timer;
    uint32_t _interval;

    uint64_t _seqNummer = 0;
    uint64_t _responseSeqNummer = 0;
    uint8_t _pingTrys = 0;

    bool _VLC;

    void sendPing(uint8_t type, uint64_t seqNummer);
    void sendPortStatusMessage();

public:
    OFInterfaceWrapper();

    OFInterfaceWrapper(OFInterfaceWrapper *&in);
    ~OFInterfaceWrapper();

    const char *class_name() const { return "OFInterfaceWrapper"; }
    const char *port_count() const { return "2/2"; }
    const char *processing() const { return "hh/hh"; }
    const char *flow_code() { return "x/x"; }
    virtual void cleanup(CleanupStage) CLICK_COLD;

    void run_timer(Timer *);

    int initialize(ErrorHandler *errh);
    int configure(Vector<String> &conf, ErrorHandler *errh);
    void push(int inPort, Packet *p);
    // Packet *pull(int port);
    bool sendPacketToInterface(Packet *p);
    bool sendPacketToPipeline(Packet *p);
};

CLICK_ENDDECLS

#endif