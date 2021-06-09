#include "OFTable.hh"

// #include <click/config.h>

// CLICK_DECLS

OFTable::OFTable(OFTable &in)
{
    this->OFTableStruct = (OF::Structs::ofp_table_features_v5 *)std::malloc(in.getOFTableStructSize());

    std::memcpy(this->OFTableStruct, in.getOFTableStruct(), in.getOFTableStructSize());

    this->flowEntries = in.getFlowEntries();
    this->PacketLookups = in.getPacketLookups();
    this->PacketMatches = in.getPacketLookups();
}

OFTable::OFTable(uint8_t tableNo)
{
    uint16_t tableStructLength = sizeof(struct OF::Structs::ofp_table_features_v5) + sizeof(OF::Data::tableFeaturesProbertys);

    this->OFTableStruct = (OF::Structs::ofp_table_features_v5 *)std::calloc(1, tableStructLength);

    this->OFTableStruct->capabilities = 0x00000000;
    this->OFTableStruct->table_id = tableNo;
    this->OFTableStruct->metadata_match = 0xffffffffffffffff;
    this->OFTableStruct->metadata_write = 0xffffffffffffffff;
    this->OFTableStruct->max_entries = this->flowEntries.max_size() > 10000 ? 10000 : this->flowEntries.max_size();
    this->OFTableStruct->length = tableStructLength;
    std::string tableName = "Table No. ";
    std::string tableNoString = std::to_string(tableNo);
    tableName += tableNoString;
    tableName.resize(OFP_MAX_TABLE_NAME_LEN - 1);

    std::memcpy(this->OFTableStruct->name, tableName.c_str(), OFP_MAX_TABLE_NAME_LEN);

#if __BYTE_ORDER == __LITTLE_ENDIAN
    this->OFTableStruct->length = __builtin_bswap16(this->OFTableStruct->length);
    this->OFTableStruct->capabilities = __builtin_bswap32(this->OFTableStruct->capabilities);
    this->OFTableStruct->metadata_match = __builtin_bswap64(this->OFTableStruct->metadata_match);
    this->OFTableStruct->metadata_write = __builtin_bswap64(this->OFTableStruct->metadata_write);
    this->OFTableStruct->max_entries = __builtin_bswap32(this->OFTableStruct->max_entries);
#endif

    std::memcpy(this->OFTableStruct->properties, OF::Data::tableFeaturesProbertys, sizeof(OF::Data::tableFeaturesProbertys));
}

OFTable::~OFTable()
{
    std::cout << "Cleenup OFTable" << std::endl;
    if (this->OFTableStruct != nullptr && this->OFTableStruct != NULL)
        std::free(this->OFTableStruct);

    this->OFTableStruct = nullptr;

    for (auto flow : this->flowEntries)
    {
        flow.cleanup();
    }

    this->flowEntries.clear();
}

void OFTable::cleanup()
{
}

uint32_t OFTable::getMaxEntries()
{
    return this->OFTableStruct->max_entries;
}

struct OF::Structs::ofp_table_features_v5 *OFTable::getOFTableStruct()
{
    return this->OFTableStruct;
}

uint16_t OFTable::getOFTableStructSize()
{
#if __BYTE_ORDER == __BIG_ENDIAN
    return this->OFTableStruct->length;
#elif __BYTE_ORDER == __LITTLE_ENDIAN
    return __builtin_bswap16(this->OFTableStruct->length);
#endif
}

bool OFTable::deleteFlow(uint64_t id)
{
    if (id != 0)
    {
        for (int i = this->flowEntries.size() - 1; i >= 0; i--)
        {
            if (this->flowEntries[i].getCookie() == id)
            {
                this->flowEntries[i].~OFTableFlowEntry();
                this->flowEntries.erase(this->flowEntries.begin() + i);
            }
        }
    }
    else
    {
        this->flowEntries.clear();
    }

    return true;
}

uint64_t OFTable::getPacketCountOfFlow(uint64_t id)
{
    uint64_t packetCount = 0;

    if (id == 0)
    {
        for (auto flow : this->flowEntries)
        {
            packetCount += flow.getReceivedPacketsCounter();
        }
    }
    return packetCount;
}
uint64_t OFTable::getByteCountOfFlow(uint64_t id)
{
    uint64_t byteCount = 0;

    if (id == 0)
    {
        for (auto flow : this->flowEntries)
        {
            byteCount += flow.getReceivedBytesCounter();
        }
    }
    return byteCount;
}
uint32_t OFTable::getFlowCount()
{
    return this->flowEntries.size();
}

bool OFTable::checkPacketInTable(myPacket *p)
{
    for (auto flowEntry : this->flowEntries)
    {
        if (flowEntry.checkPacketOnFlow(p))
            return true;
    }
    return false;
}

bool OFTable::addFlow(uint64_t Match_Fields, uint64_t Priority, ofp_instruction_header *Instructions, uint64_t Timeouts, uint64_t Cookie, uint64_t Flags)
{
    OFTableFlowEntry newEntry(Match_Fields, Priority, Instructions, Timeouts, Cookie, Flags);
    this->flowEntries.push_back(newEntry);
    std::cout << "addFlow / FlowCount: " << this->flowEntries.size() << std::endl;
}

std::vector<OFTableFlowEntry> OFTable::getFlowEntries()
{
    return this->flowEntries;
}
uint64_t OFTable::getPacketLookups()
{
    return this->PacketLookups;
}
uint64_t OFTable::getPacketMatches()
{
    return this->PacketMatches;
}

// CLICK_ENDDECLS
// // ELEMENT_REQUIRES(OFTableFlowEntry)
// ELEMENT_PROVIDES(OFTable)