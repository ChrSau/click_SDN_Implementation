#ifndef CLICK_OFTABLEFLOWENTRIE__HH
#define CLICK_OFTABLEFLOWENTRIE__HH 1

#include <string>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cstdint>
#include <cstring>
#include <chrono>
#include <memory>

#include "../Utilities/openflow.h"

#include "../Utilities/OFFunctions.hh"
#include "OFPipeline.hh"
#include "OFTableMng.hh"
#include "OFPortMng.hh"
#include "OFOXMField.hh"
#include "OFAction.hh"

#include <click/config.h>

CLICK_DECLS

class OFTableFlowEntry
{
private:
    std::shared_ptr<class OFPipeline *> pipeline;
    std::shared_ptr<class OFTableMng> tableMng;
    std::shared_ptr<class OFPortMng> portMng;
    class OFAction *action;
    // struct OF::Structs::my_ofp_flow_data *flowData = nullptr;
    std::vector<std::shared_ptr<class OFOXMField>> oxmFields;

    uint64_t ReceivedPacketsCounter = 0;
    uint64_t ReceivedBytesCounter = 0;
    uint32_t Duration_s_Counter = 0;
    uint32_t Duration_ns_Counter = 0;
    uint64_t Timeouts = 0;
    uint64_t Cookie = 0;
    uint64_t Flags = 0;
    uint16_t Priority = 0;

    bool checkPacketOnInstruction(Packet *p);
    bool checkPacketOnAction(Packet *p);

public:
    OFTableFlowEntry(uint8_t *matchStruct,
                     uint16_t matchStructLength,
                     uint16_t Priority,
                     struct ofp_instruction_header *Instructions,
                     uint16_t Timeouts,
                     uint64_t Cookie,
                     uint64_t Flags,
                     std::shared_ptr<class OFPipeline *> pipeline,
                     std::shared_ptr<class OFTableMng> tableMng,
                     std::shared_ptr<class OFPortMng> portMng);

    OFTableFlowEntry(OFTableFlowEntry *&in);

    ~OFTableFlowEntry();

    void cleanup();

    void setTimeout(int64_t timeout);

    bool checkPacketOnFlow(Packet *p, uint32_t portNo);

    bool checkAlive() { return true; }

    std::shared_ptr<class OFPipeline *> getPipeline();
    std::shared_ptr<class OFTableMng> getTableMng();
    std::shared_ptr<class OFPortMng> getPortMng();
    class OFAction *getAction();
    std::vector<std::shared_ptr<class OFOXMField>> getOxmFields();

    uint64_t getReceivedPacketsCounter() { return this->ReceivedPacketsCounter; }
    uint64_t getReceivedBytesCounter() { return this->ReceivedBytesCounter; }
    uint32_t getDuration_s_Counter() { return this->Duration_s_Counter; }
    uint32_t getDuration_ns_Counter() { return this->Duration_ns_Counter; }
    uint64_t getTimeouts() { return this->Timeouts; }
    uint64_t getCookie() { return this->Cookie; }
    uint64_t getFlags() { return this->Flags; }
    uint16_t getPriority() { return this->Priority; }

    bool operator<(const OFTableFlowEntry &Flow) const
    {
        return (this->Priority < Flow.Priority);
    }

    bool operator>(const OFTableFlowEntry &Flow) const
    {
        return (this->Priority > Flow.Priority);
    }

    bool operator()(const OFTableFlowEntry &Flow1, const OFTableFlowEntry &Flow2) const
    {
        return (Flow1.Priority > Flow2.Priority);
    }
};

CLICK_ENDDECLS
#endif