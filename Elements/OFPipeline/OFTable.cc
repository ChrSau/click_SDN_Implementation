#include "OFTable.hh"

CLICK_DECLS

OFTable::OFTable(uint8_t tableNo, std::shared_ptr<class OFPipeline *> pipeline, std::shared_ptr<class OFTableMng> tableMng, std::shared_ptr<class OFPortMng> portMng)
{
    this->pipeline = std::make_shared<class OFPipeline *>(*pipeline.get());
    this->tableMng = std::make_shared<class OFTableMng>(tableMng.get());
    this->portMng = std::make_shared<class OFPortMng>(portMng.get());

    uint16_t tableStructLength = sizeof(struct OF::Structs::ofp_table_features_v5) + sizeof(OF::Data::tableFeaturesProbertys);

    this->OFTableStruct = (struct OF::Structs::ofp_table_features_v5 *)std::calloc(1, tableStructLength);

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

#if CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
    this->OFTableStruct->length = __builtin_bswap16(this->OFTableStruct->length);
    this->OFTableStruct->capabilities = __builtin_bswap32(this->OFTableStruct->capabilities);
    this->OFTableStruct->metadata_match = __builtin_bswap64(this->OFTableStruct->metadata_match);
    this->OFTableStruct->metadata_write = __builtin_bswap64(this->OFTableStruct->metadata_write);
    this->OFTableStruct->max_entries = __builtin_bswap32(this->OFTableStruct->max_entries);
#endif

    std::memcpy(this->OFTableStruct->properties, OF::Data::tableFeaturesProbertys, sizeof(OF::Data::tableFeaturesProbertys));
}

OFTable::OFTable(OFTable &in)
{
    for (auto &&flow : in.getFlowEntries())
    {
        this->flowEntries.push_back(flow);
    }

    uint16_t tableStructLength = sizeof(struct OF::Structs::ofp_table_features_v5) + sizeof(OF::Data::tableFeaturesProbertys);
    this->OFTableStruct = (OF::Structs::ofp_table_features_v5 *)std::calloc(1, tableStructLength);
    std::memcpy(this->OFTableStruct, in.getOFTableStruct(), tableStructLength);

    this->PacketLookups = in.getPacketLookups();
    this->PacketMatches = in.getPacketMatches();
    this->pipeline = std::make_shared<class OFPipeline *>(*in.getPipeline().get());
    this->tableMng = std::make_shared<class OFTableMng>(in.getTableMng().get());
    this->portMng = std::make_shared<class OFPortMng>(in.getPortMng().get());
}

OFTable::OFTable(OFTable *in)
{
    for (auto &&flow : in->getFlowEntries())
    {
        this->flowEntries.push_back(flow);
    }

    uint16_t tableStructLength = sizeof(struct OF::Structs::ofp_table_features_v5) + sizeof(OF::Data::tableFeaturesProbertys);
    this->OFTableStruct = (OF::Structs::ofp_table_features_v5 *)std::calloc(1, tableStructLength);
    std::memcpy(this->OFTableStruct, in->getOFTableStruct(), tableStructLength);

    this->PacketLookups = in->getPacketLookups();
    this->PacketMatches = in->getPacketMatches();
    this->pipeline = std::make_shared<class OFPipeline *>(*in->getPipeline().get());
    this->tableMng = std::make_shared<class OFTableMng>(in->getTableMng().get());
    this->portMng = std::make_shared<class OFPortMng>(in->getPortMng().get());
}

OFTable::~OFTable()
{
    std::free(this->OFTableStruct);
    this->OFTableStruct = nullptr;

    this->flowEntries.clear();
}

void OFTable::cleanup()
{
    this->~OFTable();
}

uint32_t OFTable::getMaxEntries()
{
    return this->OFTableStruct->max_entries;
}

uint16_t OFTable::getOFTableStructSize()
{
#if CLICK_BYTE_ORDER == CLICK_BIG_ENDIAN
    return this->OFTableStruct->length;
#elif CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
    return __builtin_bswap16(this->OFTableStruct->length);
#endif
}

bool OFTable::deleteFlow(uint64_t id)
{

    std::cout << "delete Flow" << std::endl;
    if (id != 0)
    {
        for (int i = this->flowEntries.size() - 1; i >= 0; i--)
        {
            if (this->flowEntries[i]->getCookie() == id)
            {
                this->flowEntries[i]->~OFTableFlowEntry();
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

uint64_t OFTable::getPacketCountOfFlow(uint64_t )
{
    uint64_t packetCount = 0;

    for (auto flow : this->flowEntries)
    {
        packetCount += flow->getReceivedPacketsCounter();
    }
    return packetCount;
}
uint64_t OFTable::getByteCountOfFlow(uint64_t )
{
    uint64_t byteCount = 0;

    for (auto flow : this->flowEntries)
    {
        byteCount += flow->getReceivedBytesCounter();
    }
    return byteCount;
}
uint32_t OFTable::getFlowCount()
{
    return this->flowEntries.size();
}

bool OFTable::checkPacketInTable(Packet *p, uint32_t portNo)
{
    if (this->flowEntries.size() > 0)
    {
        for (int i = this->flowEntries.size() - 1; i >= 0; i--)
        {
            int64_t timeout = this->flowEntries[i]->getTimeouts();

            if ((timeout >= std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count()) && (timeout != -1))
            {
                continue;
            }

            bool checkPacketOnFlow = this->flowEntries[i]->checkPacketOnFlow(p->clone(), portNo);

            if (checkPacketOnFlow)
                return true;
        }
    }
    if (p != nullptr)
        p->kill();

    return false;
}

bool OFTable::addFlow(uint8_t *matchStruct,
                      uint16_t matchStructLength,
                      uint16_t Priority,
                      struct ofp_instruction_header *Instructions,
                      uint64_t Timeouts,
                      uint64_t Cookie,
                      uint64_t Flags)
{
    class OFTableFlowEntry *newEntry = new OFTableFlowEntry(matchStruct,
                                                            matchStructLength,
                                                            Priority,
                                                            Instructions,
                                                            Timeouts,
                                                            Cookie,
                                                            Flags,
                                                            this->pipeline,
                                                            this->tableMng,
                                                            this->portMng);

    this->flowEntries.push_back(std::make_shared<class OFTableFlowEntry>(newEntry));
    std::cout << "With prio: " << std::to_string(newEntry->getPriority()) << std::endl;
    if (newEntry != nullptr)
    {
        delete newEntry;
        newEntry = nullptr;
    }

    std::cout << "addFlow / FlowCount: " << std::dec << this->flowEntries.size() << std::endl;

    std::sort(this->flowEntries.begin(), this->flowEntries.end(), [](std::shared_ptr<OFTableFlowEntry> Flow1, std::shared_ptr<OFTableFlowEntry> Flow2) {
        return (Flow1->getPriority() < Flow2->getPriority());
    });

    for (size_t i = 0; i < this->flowEntries.size(); i++)
    {
        std::cout << std::dec << (int)this->flowEntries[i]->getPriority() << ", ";
    }
    std::cout << std::endl;

    return true;
}

bool OFTable::deleteOldFlows()
{
    std::vector<std::shared_ptr<class OFTableFlowEntry>> temp(this->flowEntries);

    this->flowEntries.clear();

    for (size_t i = 0; i < temp.size(); i++)
    {
        int64_t timeout = temp[i]->getTimeouts();
        if ((timeout < std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count()) && (timeout != -1))
        {
            continue;
        }
        this->flowEntries.push_back(temp[i]);
    }

    return true;
}

// getter
std::vector<std::shared_ptr<class OFTableFlowEntry>> OFTable::getFlowEntries() { return this->flowEntries; }
struct OF::Structs::ofp_table_features_v5 *OFTable::getOFTableStruct() { return this->OFTableStruct; }
uint64_t OFTable::getPacketLookups() { return this->PacketLookups; }
uint64_t OFTable::getPacketMatches() { return this->PacketMatches; }
std::shared_ptr<class OFPipeline *> OFTable::getPipeline() { return this->pipeline; }
std::shared_ptr<class OFTableMng> OFTable::getTableMng() { return this->tableMng; }
std::shared_ptr<class OFPortMng> OFTable::getPortMng() { return this->portMng; }

bool OFTable::checkAlive()
{
    if (this->flowEntries.size() > 0)
    {
        if (this->flowEntries[0]->checkAlive())
            return true;
        else
            return false;
    }
    return false;
}

CLICK_ENDDECLS
ELEMENT_REQUIRES(OFTableFlowEntry OFPipeline OFTable)
ELEMENT_PROVIDES(OFTable)