#ifndef CLICK_OFTABLEMNG__HH
#define CLICK_OFTABLEMNG__HH
// Standard Libs
#include <string>
#include <iostream>
#include <iomanip>
#include <vector>
#include <array>
#include <cstdlib>

// OpenFlow Header
#include "../openflow.h"
// Click Include
// #include <click/config.h>
// Helper Classes
#include "OFTable.hh"
#include "../OFHelper.hh"
#include "Packet.hh"

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

class OFTableMng // : public Element
{
private:
    int _DBlevel;
    uint8_t numberOfTables;
    std::vector<OFTable *> tables;
    std::vector<myPacket *> packetInBuffer;

    // void printHex(uint8_t *packet, int lenght, std::string message);

public:
    OFTableMng(int DBlevel, int numberOfTables);
    ~OFTableMng();

    // const char *class_name() const { return "OFTableMng"; }
    // const char *port_count() const { return "2/1-"; }
    // const char *processing() const { return "h/h"; }
    virtual void cleanup();

    // int initialize(ErrorHandler *);
    // int configure(Vector<String> &conf, ErrorHandler *errh);
    myPacket *push(int, myPacket *, uint32_t portNo);
};

// CLICK_ENDDECLS

#endif