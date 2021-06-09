#ifndef CLICK_OFPIPELINE__HH
#define CLICK_OFPIPELINE__HH 1

// Standard Libs
#include <string>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

// #include <linux/ip.h>
// #include <net/ethernet.h>

// OpenFlow Header
#include "../Utilities/openflow.h"

// Click Include
#include <click/config.h>
#include <click/element.hh>
#include <click/router.hh>

// Helper Classes
#include "OFTableMng.hh"
#include "OFPortMng.hh"
#include "../Utilities/OFFunctions.hh"
#include "../Utilities/OFStructs.hh"

#define PRIOWLAN 0x07
#define PRIOVLC 0x08
#define PRIOETHER 0x01 // Just for the endpoint

#define WLANPORT 0x01
#define VLCPORT 0x03
#define ETHERPORT 0x02

#define ETH213 0xb8, 0x27, 0xeb, 0xd5, 0x42, 0x27
#define ETH212 0xb8, 0x27, 0xeb, 0x8e, 0x0d, 0x85

#define IP212 0xd4
#define IP213 0xd5

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

class OFPipeline : public Element
{
private:
    int _DBlevel;
    class OFTableMng *tablesMng;
    class OFPortMng *portsMng;
    bool vlcPortStatus;

    Timer _timer;
    uint32_t _interval;
    uint8_t *createMessage(uint16_t *messageLength, uint16_t priority, uint16_t timeout, uint8_t *etherAddr, uint8_t etherLength, uint32_t outport);

public:
    OFPipeline();
    ~OFPipeline();

    // importend click Functions
    const char *class_name() const { return "OFPipeline"; }
    const char *port_count() const { return "1-/1-"; }
    const char *processing() const { return "h/h"; }

    virtual void cleanup(CleanupStage) CLICK_COLD;
    int initialize(ErrorHandler *);
    int configure(Vector<String> &conf, ErrorHandler *errh);
    void push(int, Packet *);

    void run_timer(Timer *);

    // PortMng functions
    void selected(int fd, int mask);

    bool sendToSocket(Packet *p);

    int getDBlevel();
    OFTableMng *getTablesMng();
    OFPortMng *getPortsMng();

    bool getVLCPortStatus() { return this->vlcPortStatus; }
};

CLICK_ENDDECLS

#endif