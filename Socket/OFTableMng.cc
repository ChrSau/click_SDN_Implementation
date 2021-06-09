// //click includes
// #include <click/config.h>
// #include <click/args.hh>
// #include <click/timer.hh>
// #include <clicknet/ether.h>
// #include <clicknet/udp.h>

// //our includes
// #include "openflow.h"
#include "OFTableMng.hh"

// Standard Libs

// CLICK_DECLS

OFTableMng::OFTableMng(int DBlevel, int numberOfTables)
{

    this->_DBlevel = DBlevel;
    this->numberOfTables = numberOfTables;

    for (size_t i = 0; i < this->numberOfTables; i++)
    {
        OFTable *table = new OFTable(i);
        this->tables.push_back(table);
    }
}

OFTableMng::~OFTableMng()
{
    if (_DBlevel > 10)
        printf("OFTableMng cleanup\n");

    for (auto table : this->tables)
    {
        delete table;
    }

    for (auto buffer : this->packetInBuffer)
    {
        buffer->kill();
    }

    this->packetInBuffer.clear();
    this->tables.clear();
}

void OFTableMng::cleanup()
{

}

myPacket *OFTableMng::push(int port, myPacket *p, uint32_t portNo)
{
    class myPacket *response = new myPacket();

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

#if __BYTE_ORDER == __LITTLE_ENDIAN
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

            if (request->table_id == OFPTT_ALL && (request->command == OFPFC_DELETE || request->command == OFPFC_DELETE_STRICT))
            {
                for (auto table : this->tables)
                {
                    table->deleteFlow(request->cookie);
                }
            }
            else if (request->command == OFPFC_ADD)
            {
                if (request->match.length == 4)
                {
                    uint8_t *requestInstruction = (uint8_t *)p->data();
                    requestInstruction += sizeof(struct ofp_flow_mod) + request->match.length + 4 /* Padding */ - sizeof(struct ofp_match);

                    struct ofp_instruction_header *requestInstructionHeader = (ofp_instruction_header *)requestInstruction;

#if __BYTE_ORDER == __LITTLE_ENDIAN
                    requestInstructionHeader->type = __builtin_bswap16(requestInstructionHeader->type);
                    requestInstructionHeader->len = __builtin_bswap16(requestInstructionHeader->len);
#endif
                    OF::Functions::printHex((uint8_t *)requestInstructionHeader, requestInstructionHeader->len, "InstructionHeader: ");

                    if (requestInstructionHeader->type == OFPIT_APPLY_ACTIONS)
                    {
                        std::cout << "OFPFC_ADD" << std::endl;

                        struct ofp_instruction_actions *requestInstructionActions = (struct ofp_instruction_actions *)std::calloc(1, requestInstructionHeader->len);

                        std::memcpy(requestInstructionActions, requestInstructionHeader, requestInstructionHeader->len);

                        this->tables[request->table_id]->addFlow(0, request->importance, requestInstructionHeader, (request->hard_timeout >= request->idle_timeout) ? request->hard_timeout : request->idle_timeout, request->cookie, request->flags);

                        std::free(requestInstructionActions);
                        requestInstructionActions = nullptr;
                    }
                    requestInstruction = nullptr;
                    requestInstructionHeader = nullptr;
                }
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
#if __BYTE_ORDER == __LITTLE_ENDIAN
                requestMsg->cookie = __builtin_bswap64(requestMsg->cookie);
                requestMsg->cookie_mask = __builtin_bswap64(requestMsg->cookie_mask);
#endif

                int messageLength = sizeof(struct ofp_multipart_reply) + sizeof(struct OF::Structs::my_ofp_aggregate_stats_reply);

                struct ofp_multipart_reply *responseMsg = (ofp_multipart_reply *)std::calloc(1, (int)messageLength);

                struct OF::Structs::my_ofp_aggregate_stats_reply *aggregateMsg = (OF::Structs::my_ofp_aggregate_stats_reply *)calloc(1, sizeof(struct OF::Structs::my_ofp_aggregate_stats_reply));

                uint64_t byteCount = 0;
                uint64_t packetCount = 0;
                uint32_t flowCount = 0;

                for (auto table : this->tables)
                {
                    byteCount += table->getByteCountOfFlow(requestMsg->cookie);
                    packetCount += table->getPacketCountOfFlow(requestMsg->cookie);
                    flowCount += table->getFlowCount();
                }

                aggregateMsg->byte_count = byteCount;
                aggregateMsg->packet_count = packetCount;
                aggregateMsg->flow_count = flowCount;

                std::cout << "FlowCount: " << aggregateMsg->flow_count << std::endl;

                responseMsg->header.type = OFPT_MULTIPART_REPLY;
                responseMsg->header.version = request->header.version;
                responseMsg->header.xid = request->header.xid;
                responseMsg->header.length = messageLength;

                responseMsg->type = OFPMP_AGGREGATE_STATS;
                responseMsg->flags = 0;
#if __BYTE_ORDER == __LITTLE_ENDIAN
                responseMsg->header.length = __builtin_bswap16(responseMsg->header.length);
                responseMsg->type = __builtin_bswap16(responseMsg->type);
                responseMsg->flags = __builtin_bswap16(responseMsg->flags);

                aggregateMsg->byte_count = __builtin_bswap64(aggregateMsg->byte_count);
                aggregateMsg->packet_count = __builtin_bswap64(aggregateMsg->packet_count);
                aggregateMsg->flow_count = __builtin_bswap32(aggregateMsg->flow_count);
#endif

                std::memcpy(responseMsg->body, aggregateMsg, sizeof(struct OF::Structs::my_ofp_aggregate_stats_reply));

                response->make((uint8_t *)responseMsg, messageLength);

                // if (this->_DBlevel > 10)
                OF::Functions::printHex((uint8_t *)response->data(), response->length(), "Output TableMng OFPMP_AGGREGATE_STATS: ");

                // output(1).push(response);

                std::free(aggregateMsg);
                std::free(responseMsg);
                aggregateMsg = nullptr;
                responseMsg = nullptr;

                requestMsg = nullptr;

                break;
            }
            case OFPMP_TABLE_FEATURES:
            {
                uint16_t payloadLenght = 0;

                for (auto table : this->tables)
                {
                    payloadLenght += table->getOFTableStructSize();
                }

                uint16_t messageLenght = sizeof(ofp_multipart_reply) + payloadLenght;

                struct ofp_multipart_reply *responseMsg = (ofp_multipart_reply *)std::calloc(1, (int)messageLenght);

                if (_DBlevel > 10)
                    std::cout << "Message Length: " << messageLenght << std::endl;

                uint16_t helperLenght = 0;
                for (auto table : this->tables)
                {
                    if (_DBlevel > 10)
                    {
                        OF::Functions::printHexAndChar((uint8_t *)table->getOFTableStruct(), table->getOFTableStructSize(), "getOFTableStruct(): ");
                        std::cout << "Helperlength: " << helperLenght << std::endl;
                    }
                    std::memcpy(responseMsg->body + helperLenght, table->getOFTableStruct(), table->getOFTableStructSize());

                    helperLenght += table->getOFTableStructSize();
                }

                responseMsg->header.type = OFPT_MULTIPART_REPLY;
                responseMsg->header.version = request->header.version;
                responseMsg->header.xid = request->header.xid;
                responseMsg->header.length = messageLenght;

                responseMsg->type = OFPMP_TABLE_FEATURES;
                responseMsg->flags = 0;
#if __BYTE_ORDER == __LITTLE_ENDIAN
                responseMsg->header.length = __builtin_bswap16(responseMsg->header.length);
                responseMsg->type = __builtin_bswap16(responseMsg->type);
                responseMsg->flags = __builtin_bswap16(responseMsg->flags);
#endif

                response->make((uint8_t *)responseMsg, messageLenght);

                if (this->_DBlevel > 10)
                    OF::Functions::printHex((uint8_t *)response->data(), response->length(), "Output TableMng OFPMP_TABLE_FEATURES: ");

                // output(1).push(response);

                std::free(responseMsg);
                responseMsg = nullptr;
            }
            }
            request = nullptr;
        }
        default:
        {
            // printf("keine Ahnung TableMng\n");
            break;
        }
        }

        requestHeader = nullptr;
    }
    else if (port == 1)
    {
        bool routablePacket = true;
        // printf("Packet from Port");
        for (auto table : this->tables)
        {
            if (!table->checkPacketInTable(p))
            {
                routablePacket = false;
            }
        }
        if (!routablePacket)
        {
            // printf("Packet_IN_Message\n");

            uint32_t bufferID = this->packetInBuffer.size();
            this->packetInBuffer.push_back(p->clone());

            uint32_t packetSize = sizeof(struct ofp_packet_in) + p->length() + 2 + sizeof(OXM_OF_IN_PORT) + 4;

            struct ofp_packet_in *packetInMsg = (struct ofp_packet_in *)calloc(1, packetSize);

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
            packetInMsg->match.length = 12; //sizeof(struct ofp_match);

#if __BYTE_ORDER == __LITTLE_ENDIAN
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

#if __BYTE_ORDER == __LITTLE_ENDIAN
            oxmClass = __builtin_bswap16(oxmClass);
            oxmValue = __builtin_bswap32(oxmValue);
#endif

            std::memcpy(((uint8_t *)&packetInMsg->match) + 4, &oxmClass, sizeof(oxmClass));
            std::memcpy(((uint8_t *)&packetInMsg->match) + 6, &oxmField, sizeof(oxmField));
            std::memcpy(((uint8_t *)&packetInMsg->match) + 7, &oxmLength, sizeof(oxmLength));
            std::memcpy(((uint8_t *)&packetInMsg->match) + 8, &oxmValue, sizeof(oxmValue));

            std::memcpy(((uint8_t *)&packetInMsg->match) + 18, p->data(), p->length());

            // response = Packet::make(packetInMsg, packetSize);

            // output(1).push(response);

            std::free(packetInMsg);
            packetInMsg = nullptr;
        }
    }

    // output(0).push(p);

    p->kill();

    return response;

    // return p;
}

// CLICK_ENDDECLS
// ELEMENT_REQUIRES(OFTable)
// //export our element
// ELEMENT_PROVIDES(OFTableMng)
