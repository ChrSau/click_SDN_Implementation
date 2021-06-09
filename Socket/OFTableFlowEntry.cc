#include "OFTableFlowEntry.hh"

// CLICK_DECLS

OFTableFlowEntry::OFTableFlowEntry(uint64_t Match_Fields, uint64_t Priority, ofp_instruction_header *Instructions, uint64_t Timeouts, uint64_t Cookie, uint64_t Flags)
{
    this->flowData = (OF::Structs::my_ofp_flow_data*)std::calloc(1, sizeof(struct OF::Structs::my_ofp_flow_data) + Instructions->len);
    this->flowData->Match_Fields = Match_Fields;
    this->flowData->Priority = Priority;
    this->flowData->Timeouts = Timeouts;
    this->flowData->Cookie = Cookie;
    this->flowData->Flags = Flags;
    std::memcpy(this->flowData->Instructions, Instructions, Instructions->len);
}

OFTableFlowEntry::~OFTableFlowEntry()
{
    // std::cout << "Deconstructor OFTableFlowEntry" << std::endl;
}

uint64_t OFTableFlowEntry::getReceivedPacketsCounter() { return this->flowData->Counter.ReceivedPacketsCounter; }

uint64_t OFTableFlowEntry::getReceivedBytesCounter() { return this->flowData->Counter.ReceivedBytesCounter; }

uint32_t OFTableFlowEntry::getDuration_s_Counter() { return this->flowData->Counter.Duration_s_Counter; }

uint32_t OFTableFlowEntry::getDuration_ns_Counter() { return this->flowData->Counter.Duration_ns_Counter; }

uint64_t OFTableFlowEntry::getCookie() { return this->flowData->Cookie; }

bool OFTableFlowEntry::checkPacketOnFlow(myPacket *p)
{
    return false;
}

void OFTableFlowEntry::cleanup() {
    std::free(this->flowData);
    this->flowData = nullptr;
}

// CLICK_ENDDECLS
// ELEMENT_PROVIDES(OFTableFlowEntry)