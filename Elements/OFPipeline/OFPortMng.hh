#ifndef CLICK_OFPORTMNG__HH
#define CLICK_OFPORTMNG__HH 1

//click includes
#include <click/config.h>

//our includes
#include "../Utilities/openflow.h"
#include "OFPhysicalPort.hh"
#include "OFPipeline.hh"
#include "../Utilities/OFFunctions.hh"
#include "../Utilities/OFStructs.hh"

// Standard Libs
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <queue>

// OS Libs
#include <dirent.h>
#include <sys/types.h>

CLICK_DECLS

/*
=c

OFPortMng(DB)

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

class OFPortMng // : public Element
{
private:
    int _DBlevel;
    int numberOfPorts = 0;
    class OFPipeline *_pipeline;
    uint32_t xid = 0;
    uint8_t version = 0;
    std::vector<OF::Structs::my_ofp_port> portDesc;

public:
    OFPortMng(int DBlevel, ErrorHandler *errh, OFPipeline *pipeline);
    OFPortMng(OFPortMng *in);
    OFPortMng(OFPortMng &in);
    virtual ~OFPortMng();
    virtual void cleanup();

    Packet *selected(int fd, int mask, uint32_t *outport);
    Packet *selected(int outport, Packet *p);

    Packet *push(int, Packet *);

    int getDBlevel() { return this->_DBlevel; }
    int getNumberOfPorts();
    OFPipeline *getPipeline() { return this->_pipeline; }

    bool broadAndMulticast(Packet *p, class OFPhysicalPort *_InPort);
};

CLICK_ENDDECLS

#endif