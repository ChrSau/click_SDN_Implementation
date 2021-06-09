#ifndef CLICK_OFTABLE__HH
#define CLICK_OFTABLE__HH 1

// Standard Libs
#include <string>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <array>
#include <cstring>
#include <algorithm>
#include <mutex>

// OpenFlow Header
#include "../Utilities/openflow.h"
#include "../Utilities/OFConstants.hh"
#include "../Utilities/OFFunctions.hh"
#include "../Utilities/OFStructs.hh"

// Click Include
#include <click/config.h>

// Our Classes
#include "OFTableFlowEntry.hh"
#include "OFPipeline.hh"
#include "OFPortMng.hh"
#include "OFTable.hh"

CLICK_DECLS

class OFTable
{
private:
    std::vector<std::shared_ptr<class OFTableFlowEntry>> flowEntries;

    struct OF::Structs::ofp_table_features_v5 *OFTableStruct;

    // Table Stats
    uint64_t PacketLookups = 0;
    uint64_t PacketMatches = 0;

    std::shared_ptr<class OFPipeline *> pipeline;
    std::shared_ptr<class OFTableMng> tableMng;
    std::shared_ptr<class OFPortMng> portMng;

    std::mutex flowMutex;

public:
    OFTable(uint8_t tableNo, std::shared_ptr<class OFPipeline *> pipeline, std::shared_ptr<class OFTableMng> tableMng, std::shared_ptr<class OFPortMng> portMng);
    OFTable(OFTable &in);
    OFTable(OFTable *in);
    ~OFTable();

    void cleanup();

    uint16_t getOFTableStructSize();

    bool checkPacketInTable(Packet *p, uint32_t portNo);

    bool addFlow(uint8_t *matchStruct, uint16_t matchStructLength, uint16_t Priority, ofp_instruction_header *Instructions, uint64_t Timeouts, uint64_t Cookie, uint64_t Flags);
    bool deleteFlow(uint64_t id);
    uint64_t getPacketCountOfFlow(uint64_t id);
    uint64_t getByteCountOfFlow(uint64_t id);
    uint32_t getFlowCount();
    uint32_t getMaxEntries();

    std::vector<std::shared_ptr<class OFTableFlowEntry>> getFlowEntries();
    struct OF::Structs::ofp_table_features_v5 *getOFTableStruct();
    uint64_t getPacketLookups();
    uint64_t getPacketMatches();
    std::shared_ptr<class OFPipeline *> getPipeline();
    std::shared_ptr<class OFTableMng> getTableMng();
    std::shared_ptr<class OFPortMng> getPortMng();
    bool deleteOldFlows();

    bool checkAlive();
};

CLICK_ENDDECLS
#endif