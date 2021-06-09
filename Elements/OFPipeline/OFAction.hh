#ifndef OFACTION_HH
#define OFACTION_HH 1

#include <iostream>
#include <memory>


#include <click/config.h>
#include <click/packet.hh>
#include <clicknet/ether.h>
#include <clicknet/udp.h>
#include <clicknet/tcp.h>

#include "../Utilities/OFFunctions.hh"
#include "../Utilities/OFStructs.hh"
#include "../Utilities/openflow.h"
#include "OFTableMng.hh"
#include "OFPortMng.hh"
#include "OFPhysicalPort.hh"

CLICK_DECLS

class OFAction
{
private:
    std::shared_ptr<class OFTableMng> tableMng;
    std::shared_ptr<class OFPortMng> portMng;

    std::unique_ptr<uint8_t[]> actionData;
    uint16_t actionDataLength;

public:
    OFAction(uint8_t *data, uint16_t dataLength, std::shared_ptr<OFTableMng> tableMng, std::shared_ptr<OFPortMng> portMng);
    OFAction(OFAction *in);
    OFAction(OFAction &in);

    ~OFAction();

    bool applyActionOnPacket(Packet *p, uint32_t portNo);

    uint8_t *getActionData();
    uint16_t getActionDataLength();
    std::shared_ptr<OFTableMng> getTableMng();
    std::shared_ptr<OFPortMng> getPortMng();
};

CLICK_ENDDECLS

#endif