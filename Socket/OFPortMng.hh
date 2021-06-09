#ifndef CLICK_OFPORTMNG__HH
#define CLICK_OFPORTMNG__HH


//our includes
#include "../openflow.h"
#include "OFPortMng.hh"
#include "OFPhysicalPort.hh"
#include "../OFHelper.hh"
#include "Packet.hh"

// Standard Libs
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <queue>

// OS Libs
#include <dirent.h>
#include <sys/types.h>


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
    std::vector<OFPhysicalPort *> _OFPhysicalPorts;
    std::queue<myPacket *> outputQueue;
    std::vector<std::vector<uint8_t>> ownIpAddr;

    const uint8_t broadcastPort[6] = {0xff,
                                      0xff,
                                      0xff,
                                      0xff,
                                      0xff,
                                      0xff};

    const uint8_t multicastPort[3] = {0x01,
                                      0x00,
                                      0x5e};

    const uint8_t ip6MultiCastPort[2] = {0x33,
                                         0x33};

public:
    OFPortMng(int DBlevel);
    virtual ~OFPortMng();

    virtual void cleanup();

    myPacket *selected(int fd, int mask, uint32_t *outport);

    myPacket *push(int, myPacket *);

    std::vector<OFPhysicalPort *> getPorts() { return this->_OFPhysicalPorts; }
};

#endif