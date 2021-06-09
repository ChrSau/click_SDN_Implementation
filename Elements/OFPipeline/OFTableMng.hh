#ifndef CLICK_OFTABLEMNG__HH
#define CLICK_OFTABLEMNG__HH 1

// Standard Libs
#include <string>
#include <iostream>
#include <iomanip>
#include <vector>
#include <memory>

// OpenFlow Header
#include "../Utilities/openflow.h"

// Click Include
#include <click/config.h>

// Our Classes
#include "OFTable.hh"
#include "../Utilities/OFFunctions.hh"
#include "OFPortMng.hh"
#include "OFPipeline.hh"

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

class OFTableMng // : public Element
{
private:
    int _DBlevel;
    uint8_t numberOfTables;
    std::shared_ptr<class OFPipeline *> pipeline;
    std::shared_ptr<class OFPortMng> portMng;
    // std::vector<class OFTable *> tables;
    std::shared_ptr<std::vector<std::shared_ptr<class OFTable>>> activTables;
    std::shared_ptr<std::vector<std::shared_ptr<class OFTable>>> inactivTables;
    std::vector<Packet *> packetInBuffer;
    uint32_t *counterActiveTablePipeline;
    uint32_t *counterInactiveTablePipeline;

    // void printHex(uint8_t *packet, int lenght, std::string message);

public:
    OFTableMng(int DBlevel, int numberOfTables, std::shared_ptr<class OFPipeline *> pipeline, std::shared_ptr<class OFPortMng> portMng);
    OFTableMng(OFTableMng *in);
    OFTableMng(OFTableMng &in);
    virtual ~OFTableMng();

    // const char *class_name() const { return "OFTableMng"; }
    // const char *port_count() const { return "2/1-"; }
    // const char *processing() const { return "h/h"; }
    virtual void cleanup();

    // int initialize(ErrorHandler *);
    // int configure(Vector<String> &conf, ErrorHandler *errh);
    Packet *push(int, Packet *, uint32_t portNo);
    bool checkPacketInFlow(Packet *p, uint32_t portNo);
    bool sendPacketToSocket(Packet *p, uint32_t portNo);

    int getDBlevel() { return this->_DBlevel; }
    uint8_t getNumberOfTables() { return this->numberOfTables; }
    std::shared_ptr<class OFPipeline *> getPipeline() { return this->pipeline; }
    std::shared_ptr<class OFPortMng> getPortMng() { return this->portMng; }
    std::shared_ptr<std::vector<std::shared_ptr<class OFTable>>>getTables() { return this->activTables; }
    std::vector<Packet *> getPacketInBuffer() { return this->packetInBuffer; }
};

CLICK_ENDDECLS

#endif