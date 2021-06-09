#include "OFTableMng.hh"

CLICK_DECLS

OFTableMng::OFTableMng(int DBlevel, int numberOfTables, std::shared_ptr<class OFPipeline *> _pipeline, std::shared_ptr<class OFPortMng> portMng)
{
    this->pipeline = std::make_shared<class OFPipeline *>(*_pipeline.get());
    this->portMng = std::make_shared<class OFPortMng>(portMng.get());
    this->_DBlevel = DBlevel;
    this->numberOfTables = numberOfTables;

    std::vector<std::shared_ptr<class OFTable>> mVector;

    for (size_t i = 0; i < this->numberOfTables; i++)
    {
        class OFTable *table = new OFTable(i, this->pipeline, std::make_shared<class OFTableMng>(this), this->portMng);
        mVector.push_back(std::make_shared<class OFTable>(table));
        delete table;
        table = nullptr;
    }
    this->activTables = std::make_shared<std::vector<std::shared_ptr<class OFTable>>>(std::move(mVector));
}
OFTableMng::OFTableMng(OFTableMng *in)
{
    this->_DBlevel = in->getDBlevel();
    this->numberOfTables = in->getNumberOfTables();
    this->pipeline = std::make_shared<class OFPipeline *>(*in->getPipeline().get());
    this->portMng = std::make_shared<class OFPortMng>(in->getPortMng().get());
    this->activTables = in->getTables();
    this->packetInBuffer = in->getPacketInBuffer();
}
OFTableMng::OFTableMng(OFTableMng &in)
{
    this->_DBlevel = in.getDBlevel();
    this->numberOfTables = in.getNumberOfTables();
    this->pipeline = std::make_shared<class OFPipeline *>(*in.getPipeline().get());
    this->portMng = std::make_shared<class OFPortMng>(in.getPortMng().get());
    this->activTables = in.getTables();
    this->packetInBuffer = in.getPacketInBuffer();
}

OFTableMng::~OFTableMng()
{
}

void OFTableMng::cleanup()
{

    if (_DBlevel > 10)
        click_chatter("OFTableMng cleanup\n");

    for (auto buffer : this->packetInBuffer)
    {
        buffer->kill();
    }

    if (this->packetInBuffer.size() > 0)
    {
        for (auto &&packet : this->packetInBuffer)
        {
            packet->kill();
        }
        this->packetInBuffer.clear();
    }
    if (this->activTables->size() > 0)
        this->activTables->clear();

    if (this->inactivTables != nullptr)
        if (this->inactivTables->size() > 0)
            this->inactivTables->clear();
}

Packet *OFTableMng::push(int port, Packet *p, uint32_t portNo)
{
    class WritablePacket *response = NULL;

    if (_DBlevel > 10)
        std::cout << "In Port: " << port << std::endl;

    if (port == 0)
    {
        if (this->_DBlevel > 10)
        {
            std::cout << "TableMng" << std::endl;
            OF::Functions::printHex((uint8_t *)p->data(), p->length(), "Input TableMng: ");
        }

        struct ofp_header *requestHeader = (ofp_header *)p->data();

        switch (requestHeader->type)
        {
        case OFPT_FLOW_MOD:
        {

            struct ofp_flow_mod *request = (ofp_flow_mod *)p->data();

#if CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
            request->cookie = __builtin_bswap64(request->cookie);
            request->cookie_mask = __builtin_bswap64(request->cookie_mask);
            request->idle_timeout = __builtin_bswap16(request->idle_timeout);
            request->hard_timeout = __builtin_bswap16(request->hard_timeout);
            request->priority = __builtin_bswap16(request->priority);
            request->buffer_id = __builtin_bswap32(request->buffer_id);
            request->out_port = __builtin_bswap32(request->out_port);
            request->out_group = __builtin_bswap32(request->out_group);
            request->flags = __builtin_bswap16(request->flags);
            request->importance = __builtin_bswap16(request->importance);
            request->match.length = __builtin_bswap16(request->match.length);
#endif

            if (request->command == OFPFC_ADD)
            {
                uint16_t timeout = (request->hard_timeout != 0) && (request->idle_timeout != 0) ? request->hard_timeout >= request->idle_timeout ? request->idle_timeout : request->hard_timeout : request->hard_timeout != 0 ? request->hard_timeout : request->idle_timeout;

                uint16_t matchAndInstructionLength = p->length() - sizeof(struct ofp_flow_mod) + sizeof(struct ofp_match);
                uint8_t *matchAndInstruction = static_cast<uint8_t *>(std::malloc(matchAndInstructionLength));
                std::memcpy(matchAndInstruction, &request->match, matchAndInstructionLength);

                this->inactivTables = std::make_shared<std::vector<std::shared_ptr<class OFTable>>>(std::vector<std::shared_ptr<class OFTable>>());

                for (size_t i = 0; i < this->activTables->size(); i++)
                {
                    class OFTable *inTable = new OFTable(this->activTables->at(i).get());
                    this->inactivTables->push_back(std::make_shared<class OFTable>(inTable));
                    delete inTable;
                }

                for (int i = this->inactivTables->size() - 1; i >= 0; i--)
                {
                    this->inactivTables->at(i)->deleteOldFlows();
                }

                this->inactivTables->at(request->table_id)->addFlow(matchAndInstruction, matchAndInstructionLength, request->priority, NULL, timeout, request->cookie, request->flags);

                std::shared_ptr<std::vector<std::shared_ptr<class OFTable>>> tempTables = this->activTables;
                this->activTables = this->inactivTables;
                this->inactivTables = tempTables;

                tempTables = nullptr;
            }

            request = nullptr;
            break;
        }
        case OFPT_MULTIPART_REQUEST:
        {
            struct ofp_multipart_request *request = (ofp_multipart_request *)p->data();

            switch (request->type)
            {
            case OFPMP_AGGREGATE_STATS:
            {
                struct ofp_aggregate_stats_request *requestMsg = (ofp_aggregate_stats_request *)p->data();
#if CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
                requestMsg->cookie = __builtin_bswap64(requestMsg->cookie);
                requestMsg->cookie_mask = __builtin_bswap64(requestMsg->cookie_mask);
#endif

                int messageLength = sizeof(struct ofp_multipart_reply) + sizeof(struct OF::Structs::my_ofp_aggregate_stats_reply);

                struct ofp_multipart_reply *responseMsg = (ofp_multipart_reply *)std::calloc(1, (int)messageLength);

                struct OF::Structs::my_ofp_aggregate_stats_reply *aggregateMsg = (OF::Structs::my_ofp_aggregate_stats_reply *)std::calloc(1, sizeof(struct OF::Structs::my_ofp_aggregate_stats_reply));

                uint64_t byteCount = 0;
                uint64_t packetCount = 0;
                uint32_t flowCount = 0;

                for (auto table : *this->activTables)
                {
                    byteCount += table->getByteCountOfFlow(requestMsg->cookie);
                    packetCount += table->getPacketCountOfFlow(requestMsg->cookie);
                    flowCount += table->getFlowCount();
                }

                aggregateMsg->byte_count = byteCount;
                aggregateMsg->packet_count = packetCount;
                aggregateMsg->flow_count = flowCount;

                responseMsg->header.type = OFPT_MULTIPART_REPLY;
                responseMsg->header.version = request->header.version;
                responseMsg->header.xid = request->header.xid;
                responseMsg->header.length = messageLength;

                responseMsg->type = OFPMP_AGGREGATE_STATS;
                responseMsg->flags = 0;
#if CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
                responseMsg->header.length = __builtin_bswap16(responseMsg->header.length);
                responseMsg->type = __builtin_bswap16(responseMsg->type);
                responseMsg->flags = __builtin_bswap16(responseMsg->flags);

                aggregateMsg->byte_count = __builtin_bswap64(aggregateMsg->byte_count);
                aggregateMsg->packet_count = __builtin_bswap64(aggregateMsg->packet_count);
                aggregateMsg->flow_count = __builtin_bswap32(aggregateMsg->flow_count);
#endif

                std::memcpy(responseMsg->body, aggregateMsg, sizeof(struct OF::Structs::my_ofp_aggregate_stats_reply));

                response = Packet::make(responseMsg, messageLength);

                if (this->_DBlevel > 10)
                    OF::Functions::printHexAndChar((uint8_t *)response->data(), response->length(), "Output TableMng OFPMP_AGGREGATE_STATS: ");

                std::free(aggregateMsg);
                std::free(responseMsg);
                aggregateMsg = nullptr;
                responseMsg = nullptr;

                requestMsg = nullptr;

                break;
            }
            case OFPMP_TABLE_FEATURES:
            {
                uint16_t payloadLength = 0;

                for (auto table : *this->activTables)
                {
                    payloadLength += table->getOFTableStructSize();
                }

                uint16_t messageLenght = sizeof(ofp_multipart_reply) + payloadLength;

                struct ofp_multipart_reply *responseMsg = (ofp_multipart_reply *)std::calloc(1, (int)messageLenght);

                uint16_t helperLenght = 0;
                for (auto table : *this->activTables)
                {
                    std::memcpy(responseMsg->body + helperLenght, table->getOFTableStruct(), table->getOFTableStructSize());

                    helperLenght += table->getOFTableStructSize();
                }

                responseMsg->header.type = OFPT_MULTIPART_REPLY;
                responseMsg->header.version = request->header.version;
                responseMsg->header.xid = request->header.xid;
                responseMsg->header.length = messageLenght;

                responseMsg->type = OFPMP_TABLE_FEATURES;
                responseMsg->flags = 0;
#if CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
                responseMsg->header.length = __builtin_bswap16(responseMsg->header.length);
                responseMsg->type = __builtin_bswap16(responseMsg->type);
                responseMsg->flags = __builtin_bswap16(responseMsg->flags);
#endif

                response = Packet::make(responseMsg, messageLenght);

                if (this->_DBlevel > 10)
                    OF::Functions::printHexAndChar((uint8_t *)response->data(), response->length(), "Output TableMng OFPMP_TABLE_FEATURES: ");

                std::free(responseMsg);
                responseMsg = nullptr;
                break;
            }
            case OFPMP_TABLE_STATS:
            {
                uint16_t payloadLength = sizeof(ofp_table_stats) * this->activTables->size();
                uint16_t messageLength = sizeof(ofp_multipart_reply) + payloadLength;

                ofp_multipart_reply *responseMsg = (ofp_multipart_reply *)std::calloc(1, messageLength);

                responseMsg->header.type = OFPT_MULTIPART_REPLY;
                responseMsg->header.version = request->header.version;
                responseMsg->header.xid = request->header.xid;

#if CLICK_BYTE_ORDER == CLICK_BIG_ENDIAN
                responseMsg->header.length = messageLength;
                responseMsg->type = OFPMP_TABLE_STATS;
                responseMsg->flags = 0;
#elif CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
                responseMsg->header.length = __builtin_bswap16(messageLength);
                responseMsg->type = __builtin_bswap16(OFPMP_TABLE_STATS);
                responseMsg->flags = __builtin_bswap16(0);
#endif

                for (size_t i = 0; i < this->activTables->size(); i++)
                {
                    ofp_table_stats *tableStats = (ofp_table_stats *)std::calloc(1, sizeof(ofp_table_stats));

                    tableStats->table_id = i;
#if CLICK_BYTE_ORDER == CLICK_BIG_ENDIAN
                    tableStats->active_count = this->activTables->at(i)->getFlowCount();
                    tableStats->lookup_count = this->activTables->at(i)->getPacketLookups();
                    tableStats->matched_count = this->activTables->at(i)->getPacketMatches();
#elif CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
                    tableStats->active_count = __builtin_bswap32(this->activTables->at(i)->getFlowCount());
                    tableStats->lookup_count = __builtin_bswap64(this->activTables->at(i)->getPacketLookups());
                    tableStats->matched_count = __builtin_bswap64(this->activTables->at(i)->getPacketMatches());
#endif

                    std::memcpy(responseMsg->body + (sizeof(ofp_table_stats) * i), tableStats, sizeof(ofp_table_stats));

                    std::free(tableStats);
                    tableStats = nullptr;
                }

                response = Packet::make(responseMsg, messageLength);

                if (this->_DBlevel > 10)
                    OF::Functions::printHexAndChar((uint8_t *)response->data(), response->length(), "Output TableMng OFPMP_TABLE_STATS: ");

                std::free(responseMsg);
                responseMsg = nullptr;

                break;
            }
            }
            request = nullptr;
            break;
        }
        default:
        {
            click_chatter("ERROR: keine Ahnung TableMng");
            OF::Functions::printHexAndCharToError((uint8_t *)p->data(), p->length());
            break;
        }
        }

        requestHeader = nullptr;
    }
    else if (port == 1)
    {
        this->checkPacketInFlow(p->clone(), portNo);
    }

    p->kill();

    return response;
}

bool OFTableMng::checkPacketInFlow(Packet *p, uint32_t portNo)
{
    for (auto table : *this->activTables)
    {
        table->checkPacketInTable(p->clone(), portNo);
    }

    p->kill();

    return true;
}

bool OFTableMng::sendPacketToSocket(Packet *p, uint32_t portNo)
{
    class WritablePacket *response = NULL;

    uint32_t bufferID = this->packetInBuffer.size();
    this->packetInBuffer.push_back(p->clone());

    uint8_t packetInMessagePadding = 2;
    uint8_t matchPadding = 4;

    uint32_t packetSize = sizeof(struct ofp_packet_in) + p->length() + packetInMessagePadding + sizeof(OXM_OF_IN_PORT) + matchPadding;

    struct ofp_packet_in *packetInMsg = (struct ofp_packet_in *)std::calloc(1, packetSize);

    packetInMsg->header.length = packetSize;
    packetInMsg->header.type = OFPT_PACKET_IN;
    packetInMsg->header.version = 0x05;
    packetInMsg->header.xid = bufferID;

    packetInMsg->buffer_id = OFP_NO_BUFFER;
    packetInMsg->reason = OFPR_TABLE_MISS;
    packetInMsg->total_len = p->length();
    packetInMsg->cookie = 0;
    packetInMsg->table_id = 0;

    packetInMsg->match.type = OFPMT_OXM;
    packetInMsg->match.length = sizeof(struct ofp_match) + 4 /* Padding */;

#if CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
    packetInMsg->header.length = __builtin_bswap16(packetInMsg->header.length);
    packetInMsg->header.xid = __builtin_bswap32(packetInMsg->header.xid);

    packetInMsg->buffer_id = __builtin_bswap32(packetInMsg->buffer_id);
    packetInMsg->total_len = __builtin_bswap16(packetInMsg->total_len);
    packetInMsg->cookie = __builtin_bswap64(packetInMsg->cookie);

    packetInMsg->match.type = __builtin_bswap16(packetInMsg->match.type);
    packetInMsg->match.length = __builtin_bswap16(packetInMsg->match.length);
#endif

    uint16_t oxmClass = OFPXMC_OPENFLOW_BASIC;
    uint8_t oxmField = OFPXMT_OFB_IN_PORT;
    oxmField = oxmField << 1;
    oxmField += 0b0;
    uint8_t oxmLength = 0x4;
    uint32_t oxmValue = portNo;

#if CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
    oxmClass = __builtin_bswap16(oxmClass);
    oxmValue = __builtin_bswap32(oxmValue);
#endif

    std::memcpy(((uint8_t *)&packetInMsg->match) + 4, &oxmClass, sizeof(oxmClass));
    std::memcpy(((uint8_t *)&packetInMsg->match) + 6, &oxmField, sizeof(oxmField));
    std::memcpy(((uint8_t *)&packetInMsg->match) + 7, &oxmLength, sizeof(oxmLength));
    std::memcpy(((uint8_t *)&packetInMsg->match) + 8, &oxmValue, sizeof(oxmValue));

    std::memcpy(((uint8_t *)&packetInMsg->match) + 18, p->data(), p->length());

    response = Packet::make(packetInMsg, packetSize);

    (*(this->pipeline.get()))->sendToSocket(response);
    response->kill();

    std::free(packetInMsg);
    packetInMsg = nullptr;

    p->kill();
    return true;
}

CLICK_ENDDECLS
ELEMENT_REQUIRES(OFTable)
//export our element
ELEMENT_PROVIDES(OFTableMng)
