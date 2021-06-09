#ifndef CLICK_OFTABLEFLOWENTRIE__HH
#define CLICK_OFTABLEFLOWENTRIE__HH
#include <string>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cstdint>
#include <cstring>
#include "../openflow.h"
#include "../OFHelper.hh"
#include "Packet.hh"

// #include <click/config.h>
// #include <click/args.hh>
// #include <click/timer.hh>
// #include <clicknet/ether.h>
// #include <clicknet/udp.h>

// CLICK_DECLS

class OFTableFlowEntry
{
private:
    OF::Structs::my_ofp_flow_data *flowData;

public:
    OFTableFlowEntry(uint64_t Match_Fields, uint64_t Priority, ofp_instruction_header* Instructions, uint64_t Timeouts, uint64_t Cookie, uint64_t Flags);
    // OFTable();
    ~OFTableFlowEntry();

void cleanup();

    uint64_t getReceivedPacketsCounter();
    uint64_t getReceivedBytesCounter();
    uint32_t getDuration_s_Counter();
    uint64_t getCookie();
    uint32_t getDuration_ns_Counter();

    bool checkPacketOnFlow(myPacket *p);
};

// CLICK_ENDDECLS
#endif