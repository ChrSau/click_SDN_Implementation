#ifndef OFOXFIELD_HH
#define OFOXFIELD_HH 1

// Standard Librarys
#include <iostream>
#include <cstring>

// Click Includes
#include <click/config.h>
#include <click/timer.hh>
#include <clicknet/ether.h>
#include <clicknet/udp.h>
#include <clicknet/tcp.h>

// Helper Headers
#include "../Utilities/OFFunctions.hh"
#include "../Utilities/OFStructs.hh"

#include <net/ethernet.h>
// #include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

CLICK_DECLS

class OFOXMField
{
private:
    uint8_t *oxmData;
    uint8_t oxmDataLength;
    uint8_t oxmType;

public:
    OFOXMField(uint8_t *oxmData, uint8_t oxmLength);
    OFOXMField(OFOXMField &in);
    OFOXMField(OFOXMField *in);
    ~OFOXMField();

    bool checkPacketInOXMField(Packet *p, uint32_t portNo);

    uint8_t *getOXMData();
    void setOXMData(uint8_t *oxmData);
    uint8_t getOXMDataLength();
    void setOXMDataLength(uint8_t oxmDataLength);
    uint8_t getOXMType();
    void setOXMType(uint8_t oxmType);
};

CLICK_ENDDECLS

#endif