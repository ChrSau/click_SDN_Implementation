#include "OFResponder.hh"

CLICK_DECLS

OFResponder::OFResponder() {}

OFResponder::~OFResponder() {}

void OFResponder::cleanup(CleanupStage)
{
    delete this->_switchDesc;

    if (this->_lastMessage != nullptr)
        this->_lastMessage->kill();

    if (this->_multipartMsg.size() > 0)
        this->_multipartMsg.clear();

    if (this->_controllers.size() > 0)
        this->_controllers.clear();

    this->run = false;

}

int OFResponder::initialize(ErrorHandler *)
{
    click_chatter("OFResponder: Init successfull\n");
    return 0;
}

int OFResponder::configure(Vector<String> &conf, ErrorHandler *errh)
{
    String controllerIPAddress;
    String myIFName;
    int controllerPort;
    click_chatter("OFResponder start config\n");
    
    this->_switchStatusFlags = 0x0000;
    this->_switchDesc = new OFSwitchDesc("SEW-EURODRIVE GmbH & Co KG",
                                         "Click Router",
                                         "0.0.1",
                                         "None",
                                         "None");

    if (Args(conf, this, errh)
            .read("HOSTIPADDRESS", controllerIPAddress)
            .read("PORT", controllerPort)
            .read("DB", _DBlevel)
            .read("IFNAME", myIFName)
            .complete() < 0)
    {
        return -1;
    }

    if (myIFName.length() > 0)
    {
        unsigned char *result;
        std::cout << "IFNAME: " << myIFName.c_str() << std::endl;
    }

    click_chatter("OFResponder configured\n");

    return 0;
}

void OFResponder::push(int inPort, Packet *p)
{

    if (inPort < 2)
    {
        struct ofp_header *header = (struct ofp_header *)p->data();

        class WritablePacket *response = nullptr;

        if (header->version == OFP_VERSION || header->version == (OFP_VERSION - 1))
        {
            this->_OFVersion = header->version;

            uint16_t packetLength = header->length;
#if __BYTE_ORDER == __LITTLE_ENDIAN
            packetLength = __builtin_bswap16(packetLength);
#endif

            if (p->length() > packetLength && header->type != OFPT_PACKET_OUT)
            {
                class Packet *backToQueue = Packet::make(&p->data()[packetLength], p->length() - packetLength);

                this->push(inPort, backToQueue);

                class Packet *forward = Packet::make(p->data(), packetLength);

                p = forward->clone();

                forward->kill();

                header = (struct ofp_header *)p->data();
            }

            switch (header->type)
            {
            case OFPT_MULTIPART_REPLY:
            {
                if (this->_DBlevel > 10)
                    click_chatter("OFPT_MULTIPART_REPLY\n");

                if (this->_DBlevel > 10)
                    OF::Functions::printHex((uint8_t *)p->data(), p->length(), "Input MULTIPART_REPLY");


                output(0).push(p);

                if (p != nullptr)
                    p->kill();

                p = nullptr;

                break;
            }
            case OFPT_PACKET_IN:
            {
                if (this->_DBlevel > 10)
                    click_chatter("OFPT_MULTIPART_REPLY\n");

                if (this->_DBlevel > 10)
                    OF::Functions::printHex((uint8_t *)p->data(), p->length(), "Input MULTIPART_REPLY");

                output(0).push(p);

                if (p != nullptr)
                    p->kill();

                p = nullptr;

                break;
            }
            case OFPT_HELLO:
            {
                click_chatter("OFPT_HELLO");

                struct ofp_hello *responseData = (ofp_hello *)std::calloc(1, sizeof(struct ofp_hello) + sizeof(struct ofp_hello_elem_versionbitmap) + sizeof(uint32_t));
                responseData->header.type = header->type;
                responseData->header.version = this->_OFVersion;
                responseData->header.length = (sizeof(struct ofp_header) + sizeof(struct ofp_hello_elem_versionbitmap) + sizeof(uint32_t));
                responseData->header.xid = header->xid;

                struct ofp_hello_elem_versionbitmap *helloElem = (ofp_hello_elem_versionbitmap *)std::calloc(1, sizeof(struct ofp_hello_elem_versionbitmap) + sizeof(uint32_t));

                helloElem->length = sizeof(struct ofp_hello_elem_versionbitmap) + sizeof(uint32_t);
                helloElem->type = OFPHET_VERSIONBITMAP;
                uint32_t bitmap = 0x60; // Wir unterstützen die Versionen 1.4 und 1.5

#if CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
                responseData->header.length = __builtin_bswap16(responseData->header.length);
                helloElem->length = __builtin_bswap16(helloElem->length);
                helloElem->type = __builtin_bswap16(helloElem->type);
                bitmap = __builtin_bswap32(bitmap);
#endif

                std::memcpy(helloElem->bitmaps, &bitmap, sizeof(uint32_t));
                std::memcpy(responseData->elements, helloElem, sizeof(struct ofp_hello_elem_versionbitmap) + sizeof(uint32_t));

                response = Packet::make(responseData, sizeof(struct ofp_hello) + sizeof(struct ofp_hello_elem_versionbitmap) + sizeof(uint32_t));

                output(0).push(response);

                std::free(responseData);
                std::free(helloElem);
                responseData = nullptr;
                helloElem = nullptr;

                break;
            }
            case OFPT_FEATURES_REQUEST:
            {
                click_chatter("OFPT_FEATURES_REQUEST \n");

                struct ofp_switch_features responseMsg;
                responseMsg.header.type = OFPT_FEATURES_REPLY;
                responseMsg.header.version = header->version;
                responseMsg.header.xid = header->xid;
                responseMsg.datapath_id = 0x0001c8f750015c2d; // Datapath unique ID. The lower 48-bits are for a MAC address, while the upper 16-bits are implementer-defined.
                responseMsg.n_tables = 0x2;
                responseMsg.n_buffers = 256;     // Max packets buffered at once. ClickRouter Default for Queue
                responseMsg.auxiliary_id = 0x00; // Identify auxiliary connections
                responseMsg.pad[0] = 0x00;
                responseMsg.pad[1] = 0x00;
                responseMsg.capabilities = OFPC_TABLE_STATS | OFPC_FLOW_STATS | OFPC_PORT_STATS | OFPC_QUEUE_STATS | OFPC_GROUP_STATS;
                responseMsg.reserved = 0x00000000;

#if CLICK_BYTE_ORDER == CLICK_BIG_ENDIAN
                responseMsg.header.length = sizeof(struct ofp_switch_features);
#elif CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
                responseMsg.header.length = __builtin_bswap16(sizeof(struct ofp_switch_features));
                responseMsg.datapath_id = __builtin_bswap64(responseMsg.datapath_id);
                responseMsg.n_buffers = __builtin_bswap32(responseMsg.n_buffers);
                responseMsg.capabilities = __builtin_bswap32(responseMsg.capabilities);
#endif

                response = Packet::make(&responseMsg, sizeof(struct ofp_switch_features));

                output(0).push(response);

                break;
            }
            case OFPT_BARRIER_REQUEST:
            {
                click_chatter("OFPT_BARRIER_REQUEST");
                struct ofp_header *responseMsg = (ofp_header *)std::calloc(1, sizeof(struct ofp_header));
                responseMsg->length = sizeof(struct ofp_header);
                responseMsg->type = OFPT_BARRIER_REPLY;
                responseMsg->version = header->version;
                responseMsg->xid = header->xid;
#if CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
                responseMsg->length = __builtin_bswap16(responseMsg->length);
#endif

                response = Packet::make(responseMsg, sizeof(struct ofp_header));

                output(0).push(response);

                std::free(responseMsg);
                responseMsg = nullptr;

                break;
            }
            case OFPT_SET_CONFIG:
            {
                click_chatter("OFPT_SET_CONFIG");

                struct ofp_switch_config *request = (struct ofp_switch_config *)p->data();

#if CLICK_BYTE_ORDER == CLICK_BIG_ENDIAN
                this->_switchStatusFlags = request->flags;
#elif CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
                this->_switchStatusFlags = __builtin_bswap16(request->flags);
#endif

                request = nullptr;

                break;
            }
            case OFPT_GET_CONFIG_REQUEST:
            {
                click_chatter("OFPT_GET_CONFIG_REQUEST");

                struct ofp_switch_config *responseMsg = (ofp_switch_config *)std::calloc(1, sizeof(struct ofp_switch_config));

                responseMsg->header.length = sizeof(struct ofp_switch_config);
                responseMsg->header.type = OFPT_GET_CONFIG_REPLY;
                responseMsg->header.version = header->version;
                responseMsg->header.xid = header->xid;
                responseMsg->flags = OFPC_FRAG_NORMAL | OFPC_FRAG_DROP;
                responseMsg->miss_send_len = 0x00c8;
#if CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
                responseMsg->header.length = __builtin_bswap16(responseMsg->header.length);

                responseMsg->flags = __builtin_bswap16(responseMsg->flags);
                responseMsg->miss_send_len = __builtin_bswap16(responseMsg->miss_send_len);
#endif

                response = Packet::make(responseMsg, sizeof(struct ofp_switch_config));

                output(0).push(response);

                std::free(responseMsg);
                responseMsg = nullptr;

                break;
            }
            case OFPT_ROLE_REQUEST:
            {
                click_chatter("OFPT_ROLE_REQUEST\n");

                struct ofp_role_request *request = (ofp_role_request *)p->data();

#if CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
                request->role = __builtin_bswap32(request->role);
                request->generation_id = __builtin_bswap64(request->generation_id);
#endif

                bool noError = true;

                if (!this->_controllers.empty())
                {
                    if (request->role == OFPCR_ROLE_MASTER)
                    {
                        for (auto controller : this->_controllers)
                        {
                            if (controller.rolle == request->role && controller.generationId != request->generation_id)
                            {
                                noError = false;
                            }
                        }
                    }

                    if (noError)
                    {
                        for (auto controller : this->_controllers)
                        {
                            if (controller.generationId == request->generation_id)
                            {
                                controller.rolle = request->role;
                            }
                        }
                    }
                }
                else
                {
                    struct OF::Structs::controllerData newControllerData;
                    newControllerData.OFVersion = request->header.version;
                    newControllerData.generationId = request->generation_id;
                    newControllerData.rolle = request->role;
                    this->_controllers.push_back(newControllerData);
                }

                if (noError)
                {
                    struct ofp_role_status *responseMsg = (ofp_role_status *)calloc(1, sizeof(struct ofp_role_status));
                    responseMsg->header.length = sizeof(struct ofp_role_status);
                    responseMsg->header.type = OFPT_ROLE_REPLY;
                    responseMsg->header.version = this->_OFVersion;
                    responseMsg->header.xid = request->header.xid;
                    responseMsg->role = request->role;
                    responseMsg->generation_id = request->generation_id;
#if CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
                    responseMsg->header.length = __builtin_bswap16(responseMsg->header.length);
                    responseMsg->role = __builtin_bswap32(responseMsg->role);
                    responseMsg->generation_id = __builtin_bswap64(responseMsg->role);
#endif

                    response = Packet::make(responseMsg, sizeof(struct ofp_role_status));

                    output(0).push(response);

                    std::free(responseMsg);
                    responseMsg = nullptr;
                }
                else
                {
                    struct ofp_error_msg *responseMsg = (ofp_error_msg *)calloc(1, sizeof(struct ofp_error_msg));
                    responseMsg->header.length = sizeof(struct ofp_error_msg);
                    responseMsg->header.type = OFPT_ERROR;
                    responseMsg->header.version = this->_OFVersion;
                    responseMsg->header.xid = request->header.xid;
                    responseMsg->code = OFPRRFC_STALE;
                    responseMsg->type = OFPET_ROLE_REQUEST_FAILED;
#if CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
                    responseMsg->header.length = __builtin_bswap16(responseMsg->header.length);
                    responseMsg->code = __builtin_bswap16(responseMsg->code);
                    responseMsg->type = __builtin_bswap16(responseMsg->type);
#endif

                    response = Packet::make(responseMsg, sizeof(struct ofp_error_msg));

                    output(0).push(response);

                    std::free(responseMsg);
                    responseMsg = nullptr;
                }

                request = nullptr;

                break;
            }
            case OFPT_FLOW_MOD:
            {
                click_chatter("OFPT_FLOW_MOD\n");

                output(2).push(p);

                break;
            }
            case OFPT_GROUP_MOD:
            {
                click_chatter("OFPT_GROUP_MOD\n");

                break;
            }
            case OFPT_PACKET_OUT:
            {

                click_chatter("OFPT_PACKET_OUT\n");

                output(2).push(p);

                break;
            }
            case OFPT_MULTIPART_REQUEST:
            {

                click_chatter("Multipart Request");
                struct ofp_multipart_request *request = (ofp_multipart_request *)p->data();

#if CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
                request->type = __builtin_bswap16(request->type);
                request->flags = __builtin_bswap16(request->flags);
#endif

                if (request->flags > 0)
                {
                    if (!this->_multipartMsg.empty())
                    {
                        if ((this->_multipartMsg.back().header.xid == request->header.xid) && this->_multipartMsg.back().type == request->type)
                        {
                            this->_multipartMsg.push_back(*request);
                        }
                        else
                        {
                            struct ofp_error_msg responseMsg;
                            responseMsg.header.type = OFPT_ERROR;
                            responseMsg.header.version = request->header.version;
                            responseMsg.header.xid = request->header.xid;
                            responseMsg.code = OFPBRC_BAD_MULTIPART;
                            responseMsg.type = OFPET_BAD_REQUEST;

#if CLICK_BYTE_ORDER == CLICK_BIG_ENDIAN
                            responseMsg.length = sizeof(struct ofp_error_msg);
#elif CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
                            responseMsg.header.length = __builtin_bswap16(sizeof(struct ofp_error_msg));
                            responseMsg.code = __builtin_bswap16(responseMsg.code);
                            responseMsg.type = __builtin_bswap16(responseMsg.type);
#endif

                            response = Packet::make(&responseMsg, sizeof(struct ofp_error_msg));

                            output(0).push(response);

                            this->_multipartMsg.clear();
                        }
                    }
                    else
                    {
                        this->_multipartMsg.push_back(*request);
                    }
                }
                else
                {
                    this->_multipartMsg.push_back(*request);

                    switch (this->_multipartMsg[0].type)
                    {
                    case OFPMP_TABLE_FEATURES:
                    {
                        click_chatter("OFPMP_TABLE_FEATURES\n");

                        output(2).push(p);

                        this->_multipartMsg.clear();
                        break;
                    }
                    case OFPMP_DESC:
                    {
                        click_chatter("OFPMP_DESC\n");

                        struct ofp_multipart_reply *responseMsg = (ofp_multipart_reply *)std::calloc(1, sizeof(struct ofp_multipart_reply) + sizeof(struct ofp_desc));

                        responseMsg->header.type = OFPT_MULTIPART_REPLY;
                        responseMsg->header.version = this->_multipartMsg[0].header.version;
                        responseMsg->header.xid = this->_multipartMsg[0].header.xid;
                        responseMsg->header.length = sizeof(struct ofp_multipart_reply) + sizeof(struct ofp_desc);

                        responseMsg->type = OFPMP_DESC;
                        responseMsg->flags = 0x0000;

                        std::memcpy(responseMsg->body, this->_switchDesc->getSwitchDesc(), sizeof(struct ofp_desc));

#if CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
                        responseMsg->flags = __builtin_bswap16(responseMsg->flags);
                        responseMsg->type = __builtin_bswap16(responseMsg->type);

                        responseMsg->header.length = __builtin_bswap16(responseMsg->header.length);
#endif

                        response = Packet::make(responseMsg, sizeof(struct ofp_multipart_reply) + sizeof(struct ofp_desc));

                        output(0).push(response);

                        std::free(responseMsg);
                        responseMsg = nullptr;

                        this->_multipartMsg.clear();

                        break;
                    }
                    case OFPMP_PORT_DESC:
                    {

                        click_chatter("Port Description\n");

                        output(2).push(p);

                        this->_multipartMsg.clear();

                        break;
                    }
                    case OFPMP_AGGREGATE_STATS:
                    {

                        click_chatter("OFPMP_AGGREGATE_STATS\n");

                        output(2).push(p);

                        this->_multipartMsg.clear();

                        break;
                    }
                    default:
                    {

                        click_chatter("Keine Ahnung Multipart\n");

                        this->_multipartMsg.clear();

                        break;
                    }
                    }
                }

                request = nullptr;
                break;
            }
            case OFPT_ECHO_REQUEST:
            {
                click_chatter("OFPT_ECHO_REPLY");
                struct ofp_header responseData;
                responseData.type = OFPT_ECHO_REPLY;
                responseData.version = OFP_VERSION - 1;
                responseData.length = 0x0800; //sizeof(struct ofp_header);
                responseData.xid = header->xid;

                response = Packet::make(&responseData, sizeof(struct ofp_header));

                output(0).push(response);

                break;
            }
            default:
            {
                click_chatter("Keine Ahnung welcher Header\n");

                OF::Functions::printHexAndCharToError((uint8_t *)p->data(), p->length(), "Keine Ahnung welcher Header: ");
                break;
            }
            }

            this->_lastMessage = p;
        }
        else
        {
            OF::Structs::click_port_comm_header *header = (OF::Structs::click_port_comm_header *)p->data();

            if (header->identifier == OF::Structs::CPCI_TO_RESPONDER)
            {
                switch (header->type)
                {
                case OF::Structs::CPCT_PORT_STATUS:
                {
                    WritablePacket *portStatus = Packet::make(header->data, header->length);

                    ofp_port_status *message = (ofp_port_status *)portStatus->data();

                    message->header.version = this->_OFVersion;

                    message = nullptr;

                    output(0).push(portStatus);

                    break;
                }
                default:
                    break;
                }
            }
            else
            {
                click_chatter("OpenFlow Version must be 1.4 or higher --> send to Controller as Paket_in");
                this->sendPacketToSocket(p, OFPP_CONTROLLER);
                OF::Functions::printHexAndCharToError((uint8_t *)p->data(), p->length());
            }

            header = nullptr;
        }


        header = nullptr;
    }

    if (p != nullptr)
        p->kill();
}

Packet *OFResponder::pull(int port)
{
    return NULL;
}

bool OFResponder::getRun() { return this->run; }

bool OFResponder::sendPacketToSocket(Packet *p, uint32_t portNo)
{
    class WritablePacket *response = NULL;

    uint32_t packetSize = sizeof(struct ofp_packet_in) + p->length() + 2 + sizeof(OXM_OF_IN_PORT) + 4;

    struct ofp_packet_in *packetInMsg = (struct ofp_packet_in *)std::calloc(1, packetSize);

    packetInMsg->header.length = packetSize;
    packetInMsg->header.type = OFPT_PACKET_IN;
    packetInMsg->header.version = 0x05;
    packetInMsg->header.xid = 0x0000;

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

    output(0).push(response);
    response->kill();
    response = nullptr;

    std::free(packetInMsg);
    packetInMsg = nullptr;

    p->kill();
    return true;
}

CLICK_ENDDECLS
ELEMENT_REQUIRES(OFSwitchDesc)

//export our element
EXPORT_ELEMENT(OFResponder)