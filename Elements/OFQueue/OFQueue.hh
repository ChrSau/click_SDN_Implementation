#ifndef CLICK_OFQUEUE__HH
#define CLICK_OFQUEUE__HH 1

//click includes
#include <click/config.h>
#include <click/element.hh>
#include <click/args.hh>

//our includes
#include "OFPortMng.hh"
#include "OFPhysicalPort.hh"
#include "../Utilities/OFFunctions.hh"
#include "../Utilities/openflow.h"

// Standard Libs
#include <iostream>
#include <queue>
#include <string>

CLICK_DECLS

/*
=c

OFQueue(DB)

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

class OFQueue : public Element
{
private:
    int _DBlevel;
    uint32_t maxQueueSize;
    std::queue<OF::Packet *> outputQueue;

    struct OF::Structs::ofp_queue_counter *counter;

    void printHex(uint8_t *packet, int lenght, std::string message);

public:
    OFQueue();
    ~OFQueue();

    const char *class_name() const { return "OFQueue"; }
    const char *port_count() const { return "1-/1-"; }
    const char *processing() const { return "hh/lhh"; }

    virtual Packet *pull(int);

    int initialize(ErrorHandler *);
    int configure(Vector<String> &conf, ErrorHandler *errh);
    void cleanup(CleanupStage) CLICK_COLD;
    virtual void push(int, Packet *);

    void openFlowMessage(Packet *);
    void normalMessage(Packet *);
};

CLICK_ENDDECLS

#endif