#ifndef CLICK_OFTABLE__HH
#define CLICK_OFTABLE__HH
// Standard Libs
#include <string>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <array>
#include <cstring>
// OpenFlow Header
#include "../openflow.h"
// // Click Include
// #include <click/config.h>
// #include <click/args.hh>
// #include <click/timer.hh>
// #include <clicknet/ether.h>
// #include <clicknet/udp.h>
// Helper Classes
#include "OFTableFlowEntry.hh"
#include "Packet.hh"
#include "../OFHelper.hh"

// CLICK_DECLS

class OFTable
{
private:
    std::vector<OFTableFlowEntry> flowEntries;

    struct OF::Structs::ofp_table_features_v5 *OFTableStruct;

    // Table Stats
    uint64_t PacketLookups;
    uint64_t PacketMatches;

public:
    OFTable(OFTable &in);
    OFTable(uint8_t tableNo);
    ~OFTable();

    void cleanup();

    struct OF::Structs::ofp_table_features_v5 *getOFTableStruct();
    uint16_t getOFTableStructSize();

    bool checkPacketInTable(myPacket *p);

    bool addFlow(uint64_t Match_Fields, uint64_t Priority, ofp_instruction_header *Instructions, uint64_t Timeouts, uint64_t Cookie, uint64_t Flags);
    bool deleteFlow(uint64_t id);
    uint64_t getPacketCountOfFlow(uint64_t id);
    uint64_t getByteCountOfFlow(uint64_t id);
    uint32_t getFlowCount();

    uint32_t getMaxEntries();

    std::vector<OFTableFlowEntry> getFlowEntries();
    uint64_t getPacketLookups();
    uint64_t getPacketMatches();
};

// CLICK_ENDDECLS
#endif