#ifndef CLICK_OFPIPELINE__HH
#define CLICK_OFPIPELINE__HH
// Standard Libs
#include <string>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <thread>

// OpenFlow Header
#include "../openflow.h"

// Helper Classes
#include "OFTableMng.hh"
#include "OFPortMng.hh"
#include "../OFHelper.hh"
#include "Packet.hh"
#include "TCPClient.hh"

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

class OFPipeline
{
private:
    int _DBlevel;
    // uint8_t numberOfTables;
    OFTableMng *tablesMng;
    OFPortMng *portsMng;
    TCPClient *tcp;
    std::vector<std::thread> threads;

public:
    OFPipeline(TCPClient *tcp, int DBLevel);
    ~OFPipeline();

    myPacket *simple_action(int port, myPacket *p);

    // PortMng functions
    void selected(int fd, int mask);
};

#endif