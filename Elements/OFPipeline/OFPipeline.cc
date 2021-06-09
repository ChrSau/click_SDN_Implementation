#include "OFPipeline.hh"

CLICK_DECLS

OFPipeline::OFPipeline() : _timer(this), _interval(5000), vlcPortStatus(true) {}
OFPipeline::~OFPipeline() {}

void OFPipeline::cleanup(CleanupStage)
{
    if (this->portsMng)
    {
        this->portsMng->cleanup();
        delete this->portsMng;
        this->portsMng = nullptr;
    }
    if (this->tablesMng)
    {
        this->tablesMng->cleanup();
        delete this->tablesMng;
        this->tablesMng = nullptr;
    }
}
int OFPipeline::initialize(ErrorHandler *)
{

    // WritablePacket *addFlowPacket;
    uint16_t messageLength = 0;
    uint8_t etherAddrress212[] = {ETH212};
    uint8_t etherAddrress213[] = {ETH213};
    uint8_t *message = nullptr;

    message = this->createMessage(&messageLength, PRIOETHER, this->_interval / 1000, etherAddrress213, ETH_ALEN, ETHERPORT);
    this->tablesMng->push(0, Packet::make(message, messageLength), 0);
    std::free(message);
    message = nullptr;
    // addFlowPacket = Packet::make(this->addFlowMessage212ToVLCFromEthernet, sizeof(this->addFlowMessage212ToVLCFromEthernet));
    message = this->createMessage(&messageLength, PRIOVLC, this->_interval / 1000, etherAddrress212, ETH_ALEN, VLCPORT);
    this->tablesMng->push(0, Packet::make(message, messageLength), 0);
    std::free(message);
    message = nullptr;
    // addFlowPacket = Packet::make(this->addFlowMessage212ToWLANFromEthernet, sizeof(this->addFlowMessage212ToWLANFromEthernet));
    message = this->createMessage(&messageLength, PRIOWLAN, this->_interval / 1000, etherAddrress212, ETH_ALEN, WLANPORT);
    this->tablesMng->push(0, Packet::make(message, messageLength), 0);
    std::free(message);
    message = nullptr;
    // addFlowPacket = Packet::make(this->addFlowMessage212ToEtherFromWLAN, sizeof(this->addFlowMessage212ToEtherFromWLAN));
    message = this->createMessage(&messageLength, PRIOETHER, this->_interval / 1000, etherAddrress212, ETH_ALEN, ETHERPORT);
    this->tablesMng->push(0, Packet::make(message, messageLength), 0);
    std::free(message);
    message = nullptr;
    // addFlowPacket = Packet::make(this->addFlowMessage213ToVLCFromEthernet, sizeof(this->addFlowMessage213ToVLCFromEthernet));
    message = this->createMessage(&messageLength, PRIOVLC, this->_interval / 1000, etherAddrress213, ETH_ALEN, VLCPORT);
    this->tablesMng->push(0, Packet::make(message, messageLength), 0);
    std::free(message);
    message = nullptr;
    // addFlowPacket = Packet::make(this->addFlowMessage213ToWLANFromEthernet, sizeof(this->addFlowMessage213ToWLANFromEthernet));
    message = this->createMessage(&messageLength, PRIOWLAN, this->_interval / 1000, etherAddrress213, ETH_ALEN, WLANPORT);
    this->tablesMng->push(0, Packet::make(message, messageLength), 0);
    std::free(message);
    message = nullptr;

    this->_timer.initialize(this);
    if (_interval != 0)
        this->_timer.schedule_after_msec(_interval);

    click_chatter("OFPipeline: Init successfull\n");

    return EXIT_SUCCESS;
}
int OFPipeline::configure(Vector<String> &conf, ErrorHandler *errh)
{

    click_chatter("OFPortMng start config\n");

    int numberOfTables = 2;

    if (Args(conf, this, errh)
            .read_mp("DB", _DBlevel)
            .read("NUMBEROFTABLES", numberOfTables)
            .complete() < 0)
    {
        return -1;
    }

    // Init PortMng
    this->portsMng = new OFPortMng(this->_DBlevel, errh, this);

    // Init TableMng
    this->tablesMng = new OFTableMng(this->_DBlevel, numberOfTables, std::make_shared<class OFPipeline *>(this), std::make_shared<class OFPortMng>(this->portsMng));

    return EXIT_SUCCESS;
}
void OFPipeline::push(int inPort, Packet *p)
{
    Packet *response = nullptr;

    if (inPort > 0)
    {
        struct OF::Structs::click_port_comm_header *clickHeader = (struct OF::Structs::click_port_comm_header *)p->data();

        if (this->_DBlevel > 10)
            OF::Functions::printHexAndChar((uint8_t *)p->data(), p->length(), "OFPipeline::push()");

        if (clickHeader->identifier == OF::Structs::CPCI_TO_PIPELINE)
        {
            switch (clickHeader->type)
            {
            case OF::Structs::CPCT_PACKET_IN:
            {
                WritablePacket *packetIn = Packet::make(clickHeader->data, clickHeader->length);
                packetIn->set_packet_type_anno(p->packet_type_anno());

                ether_header *etherHeader = (ether_header *)packetIn->data();
                if (etherHeader->ether_type == ETHERTYPE_IP)
                {
                    click_ip *ipHeader = (click_ip *)(packetIn->data() + ETHER_HDR_LEN);
                    if (ipHeader->ip_ttl > 0)
                    {
                        ipHeader->ip_ttl--;

                        response = this->portsMng->selected(inPort, packetIn);

                        if (response != nullptr)
                        {
                            if (this->_DBlevel > 10)
                                OF::Functions::printHexAndChar((uint8_t *)response->data(), response->length(), "Response :");

                            this->tablesMng->checkPacketInFlow(response->clone(), inPort);

                            response->kill();
                            response = nullptr;
                        }

                        if (packetIn != nullptr)
                            packetIn->kill();
                    }
                    ipHeader = nullptr;
                }
                else
                {
                    response = this->portsMng->selected(inPort, packetIn);

                    if (response != nullptr)
                    {
                        if (this->_DBlevel > 10)
                            OF::Functions::printHexAndChar((uint8_t *)response->data(), response->length(), "Response :");

                        this->tablesMng->checkPacketInFlow(response->clone(), inPort);

                        response->kill();
                        response = nullptr;
                    }

                    if (packetIn != nullptr)
                        packetIn->kill();
                }
                etherHeader = nullptr;

                break;
            }
            case OF::Structs::CPCT_PORT_DESC:
            {
                response = this->portsMng->push(inPort, p);
                break;
            }
            case OF::Structs::CPCT_PORT_STATUS:
            {
                clickHeader->identifier = OF::Structs::CPCI_TO_RESPONDER;
                OF::Structs::ofp_port_status *portStatus = (OF::Structs::ofp_port_status *)clickHeader->data;

#if CLICK_BYTE_ORDER == CLICK_BIG_ENDIAN

                portStatus->desc.port_no = inPort;

#elif CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN

                portStatus->desc.port_no = __builtin_bswap32(inPort);

#endif

                this->vlcPortStatus = portStatus->desc.state == OFPPS_LIVE ? true : false;

                response = Packet::make(p->data(), p->length());

                break;
            }

            default:
                break;
            }
        }

        clickHeader = nullptr;

        if (p != nullptr)
            p->kill();
    }
    else
    {
        struct ofp_header *OFHeader = (struct ofp_header *)p->data();

        switch (OFHeader->type)
        {
        // To TableMng
        case OFPT_FLOW_MOD:
        {
            std::cout << "OFPT_FLOW_MOD" << std::endl;
            response = this->tablesMng->push(inPort, p->clone(), 0);
            break;
        }
        // To PortMng
        case OFPT_PACKET_OUT:
        {
            response = this->portsMng->push(inPort, p->clone());
            break;
        }
        // MultipartMsg
        case OFPT_MULTIPART_REQUEST:
        {
            struct ofp_multipart_request *multipartRequest = (struct ofp_multipart_request *)p->data();

            switch (multipartRequest->type)
            {
            // To TableMng
            case OFPMP_AGGREGATE_STATS:
            {
                response = this->tablesMng->push(0, p->clone(), 0);
                break;
            }
            case OFPMP_TABLE_FEATURES:
            {
                response = this->tablesMng->push(0, p->clone(), 0);
                break;
            }
            // To PortMng
            case OFPMP_PORT_DESC:
            {
                response = this->portsMng->push(0, p->clone());
                break;
            }
            case OFPMP_TABLE_STATS:
            {
                response = this->tablesMng->push(0, p->clone(), 0);
                break;
            }
            case OFPMP_GROUP_FEATURES:
            {

                ofp_multipart_request *request = (ofp_multipart_request *)p->data();

                uint16_t payloadLength = sizeof(ofp_group_features);
                uint16_t messageLength = sizeof(ofp_multipart_reply) + payloadLength;
                ofp_multipart_reply *responseMsg = (ofp_multipart_reply *)std::calloc(1, messageLength);

                responseMsg->header.type = OFPT_MULTIPART_REPLY;
                responseMsg->header.version = request->header.version;
                responseMsg->header.xid = request->header.xid;

#if CLICK_BYTE_ORDER == CLICK_BIG_ENDIAN
                responseMsg->header.length = messageLength;
                responseMsg->type = OFPMP_GROUP_FEATURES;
                responseMsg->flags = 0;
#elif CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
                responseMsg->header.length = __builtin_bswap16(messageLength);
                responseMsg->type = __builtin_bswap16(OFPMP_GROUP_FEATURES);
                responseMsg->flags = __builtin_bswap16(0);
#endif

                ofp_group_features *payload = (ofp_group_features *)std::calloc(1, payloadLength);

#if CLICK_BYTE_ORDER == CLICK_BIG_ENDIAN
                payload->types = OFPGT_ALL;
                payload->capabilities = OFPGFC_SELECT_WEIGHT | OFPGFC_SELECT_LIVENESS | OFPGFC_CHAINING | OFPGFC_CHAINING_CHECKS;
                for (auto &&max_group : payload->max_groups)
                {
                    max_group = 2;
                }
                for (auto &&action : payload->actions)
                {
                    action = 1 << OFPAT_OUTPUT;
                }
#elif CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
                payload->types = __builtin_bswap32(OFPGT_ALL);
                payload->capabilities = __builtin_bswap32(OFPGFC_SELECT_WEIGHT | OFPGFC_SELECT_LIVENESS | OFPGFC_CHAINING | OFPGFC_CHAINING_CHECKS);
                for (auto &&max_group : payload->max_groups)
                {
                    max_group = __builtin_bswap32(2);
                }
                for (auto &&action : payload->actions)
                {
                    action = __builtin_bswap32(1 << OFPAT_OUTPUT);
                }
#endif

                std::memcpy(responseMsg->body, payload, payloadLength);

                response = Packet::make(responseMsg, messageLength);

                std::free(responseMsg);
                responseMsg = nullptr;

                std::free(payload);
                payload = nullptr;

                break;
            }

            default:
                click_chatter("ERROR Pipeline.push() Multipart Msg");
                OF::Functions::printHexAndCharToError((uint8_t *)p->data(), p->length());
                break;
            }

            break;
        }

        default:
            click_chatter("ERROR Pipeline.push()");
            OF::Functions::printHexAndCharToError((uint8_t *)p->data(), p->length());
            break;
        }

        p->kill();
        p = nullptr;
    }

    if (response != nullptr)
    {
        if (_DBlevel > 10)
            OF::Functions::printHex((uint8_t *)response->data(), response->length(), "Out from Pipeline: ");
        output(0).push(response);
        response->kill();
        response = nullptr;
    }
}

// PortMng functions
void OFPipeline::selected(int fd, int mask)
{
    uint32_t portNo = OFPP_LOCAL;
    Packet *p = this->portsMng->selected(fd, mask, &portNo);
    if (p != NULL || p != nullptr)
    {
        this->tablesMng->checkPacketInFlow(p->clone(), portNo);
        p->kill();
    }
    p = nullptr;
}

bool OFPipeline::sendToSocket(Packet *p)
{
    output(0).push(p);
    p->kill();
    return true;
}

void OFPipeline::run_timer(Timer *)
{
    uint16_t messageLength = 0;
    uint8_t etherAddrress212[] = {ETH212};
    uint8_t etherAddrress213[] = {ETH213};
    uint8_t *message = nullptr;

    message = this->createMessage(&messageLength, PRIOETHER, this->_interval / 1000, etherAddrress213, ETH_ALEN, ETHERPORT);
    this->tablesMng->push(0, Packet::make(message, messageLength), 0);
    std::free(message);
    message = nullptr;

    if (this->vlcPortStatus)
    {
        message = this->createMessage(&messageLength, PRIOVLC, this->_interval / 1000, etherAddrress212, ETH_ALEN, VLCPORT);
        this->tablesMng->push(0, Packet::make(message, messageLength), 0);
        std::free(message);
        message = nullptr;
    }

    message = this->createMessage(&messageLength, PRIOWLAN, this->_interval / 1000, etherAddrress212, ETH_ALEN, WLANPORT);
    this->tablesMng->push(0, Packet::make(message, messageLength), 0);
    std::free(message);
    message = nullptr;

    message = this->createMessage(&messageLength, PRIOETHER, this->_interval / 1000, etherAddrress212, ETH_ALEN, ETHERPORT);
    this->tablesMng->push(0, Packet::make(message, messageLength), 0);
    std::free(message);
    message = nullptr;

    if (this->vlcPortStatus)
    {
        message = this->createMessage(&messageLength, PRIOVLC, this->_interval / 1000, etherAddrress213, ETH_ALEN, VLCPORT);
        this->tablesMng->push(0, Packet::make(message, messageLength), 0);
        std::free(message);
        message = nullptr;
    }

    message = this->createMessage(&messageLength, PRIOWLAN, this->_interval / 1000, etherAddrress213, ETH_ALEN, WLANPORT);
    this->tablesMng->push(0, Packet::make(message, messageLength), 0);
    std::free(message);
    message = nullptr;

    this->_timer.schedule_after_msec(_interval);
}

uint8_t *OFPipeline::createMessage(uint16_t *messageLength, uint16_t priority, uint16_t timeout, uint8_t *etherAddr, uint8_t etherLength, uint32_t outport)
{

    if (messageLength == nullptr)
    {
        std::cerr << "ERROR: messageLength = null" << std::endl;
        return NULL;
    }

    const uint8_t paddingAllignSize = 8;
    const uint8_t sizeOfOXMMessage = sizeof(OF::Structs::my_ofp_oxm_header) + etherLength;
    const uint8_t ofpMatchPaddingSize = paddingAllignSize - ((sizeof(ofp_match) - 4 /*Padding*/ + sizeOfOXMMessage) % paddingAllignSize); // Should be 2 for 1 OXM Field with Ethernet DST
    const uint16_t sizeOfAction = sizeof(ofp_action_output);
    const uint16_t sizeOfInstructionAndAction = sizeof(ofp_instruction_actions) + sizeOfAction;
    const uint16_t sizeOfMessage = sizeof(ofp_flow_mod) + sizeOfInstructionAndAction + sizeOfOXMMessage - 2 /* Padding of ofp_match Struct */;

    ofp_flow_mod *flowMessage = (ofp_flow_mod *)std::calloc(1, sizeOfMessage);
    OF::Structs::my_ofp_oxm_header *oxmHeader = (OF::Structs::my_ofp_oxm_header *)std::calloc(1, sizeOfOXMMessage);

    ofp_instruction_actions *instructionHeader = (ofp_instruction_actions *)std::calloc(1, sizeOfInstructionAndAction);

    ofp_action_output *actionHeader = (ofp_action_output *)std::calloc(1, sizeOfAction);

#if CLICK_BYTE_ORDER == CLICK_BIG_ENDIAN
    flowMessage->header.length = sizeOfMessage;
    flowMessage->header.type = OFPT_FLOW_MOD;
    flowMessage->header.version = OFP_VERSION == 0x05 ? OFP_VERSION : 0x05; // Openflow version 1.4 (0x05)
    // flowMessage->header.xid = 0x00000000;
    flowMessage->cookie = 0x0020000000000000;
    // flowMessage->cookie_mask = 0x0000000000000000;
    flowMessage->table_id = 0x00;
    flowMessage->command = OFPFC_ADD;
    flowMessage->idle_timeout = timeout;
    // flowMessage->hard_timeout = 0x0000;
    flowMessage->priority = priority;
    flowMessage->buffer_id = OFP_NO_BUFFER;
    flowMessage->out_port = outport;
    flowMessage->out_group = OFPG_ANY;
    // flowMessage->flags = 0x0000;
    // flowMessage->importance = 0x0000;
    flowMessage->match.length = sizeof(ofp_match) - 4 /* Padding of Match Struct */ + sizeOfOXMMessage);
    flowMessage->match.type = OFPMT_OXM;

    oxmHeader->oxm_class = OFPXMC_OPENFLOW_BASIC;
    oxmHeader->oxm_field_WM = (OFPXMT_OFB_ETH_DST << 1); // Has Mask = False
    oxmHeader->oxm_length = etherLength;
    std::memcpy(oxmHeader->oxm_value, etherAddr, etherLength);

    std::memcpy(flowMessage->match.oxm_fields, oxmHeader, sizeOfOXMMessage);

    std::free(oxmHeader);
    oxmHeader = nullptr;

    actionHeader->len = sizeOfAction;
    actionHeader->max_len = OFPCML_NO_BUFFER;
    actionHeader->type = OFPAT_OUTPUT;
    actionHeader->port = outport;

    instructionHeader->len = sizeOfInstructionAndAction;
    instructionHeader->type = OFPIT_APPLY_ACTIONS;

    std::memcpy(instructionHeader->actions, actionHeader, sizeOfAction);

    std::free(actionHeader);
    actionHeader = nullptr;

    std::memcpy((((uint8_t *)&flowMessage->match) + sizeOfOXMMessage + sizeof(ofp_match) - 4 + ofpMatchPaddingSize), instructionHeader, sizeOfInstructionAndAction);

    std::free(instructionHeader);
    instructionHeader = nullptr;

#elif CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
    flowMessage->header.length = __builtin_bswap16(sizeOfMessage);
    flowMessage->header.type = OFPT_FLOW_MOD;
    flowMessage->header.version = OFP_VERSION == 0x05 ? OFP_VERSION : 0x05; // Openflow version 1.4 (0x05)
    // flowMessage->header.xid = 0x00000000;
    flowMessage->cookie = __builtin_bswap64(0x0020000000000000);
    // flowMessage->cookie_mask = 0x0000000000000000;
    flowMessage->table_id = 0x00;
    flowMessage->command = OFPFC_ADD;
    flowMessage->idle_timeout = __builtin_bswap16(timeout);
    // flowMessage->hard_timeout = 0x0000;
    flowMessage->priority = __builtin_bswap16(priority);
    flowMessage->buffer_id = __builtin_bswap32(OFP_NO_BUFFER);
    flowMessage->out_port = __builtin_bswap32(outport);
    flowMessage->out_group = __builtin_bswap32(OFPG_ANY);
    // flowMessage->flags = 0x0000;
    // flowMessage->importance = 0x0000;
    flowMessage->match.length = __builtin_bswap16(sizeof(ofp_match) - 4 /* Padding of Match Struct */ + sizeOfOXMMessage);
    flowMessage->match.type = __builtin_bswap16(OFPMT_OXM);

    oxmHeader->oxm_class = __builtin_bswap16(OFPXMC_OPENFLOW_BASIC);
    oxmHeader->oxm_field_WM = (OFPXMT_OFB_ETH_DST << 1); // Has Mask = False
    oxmHeader->oxm_length = etherLength;
    std::memcpy(oxmHeader->oxm_value, etherAddr, etherLength);

    std::memcpy(flowMessage->match.oxm_fields, oxmHeader, sizeOfOXMMessage);

    std::free(oxmHeader);
    oxmHeader = nullptr;

    actionHeader->len = __builtin_bswap16(sizeOfAction);
    actionHeader->max_len = __builtin_bswap16(OFPCML_NO_BUFFER);
    actionHeader->type = __builtin_bswap16(OFPAT_OUTPUT);
    actionHeader->port = __builtin_bswap32(outport);

    instructionHeader->len = __builtin_bswap16(sizeOfInstructionAndAction);
    instructionHeader->type = __builtin_bswap16(OFPIT_APPLY_ACTIONS);

    std::memcpy(instructionHeader->actions, actionHeader, sizeOfAction);

    std::free(actionHeader);
    actionHeader = nullptr;

    std::memcpy((((uint8_t *)&flowMessage->match) + sizeOfOXMMessage + sizeof(ofp_match) - 4 + ofpMatchPaddingSize), instructionHeader, sizeOfInstructionAndAction);

    std::free(instructionHeader);
    instructionHeader = nullptr;

#endif

    *messageLength = sizeOfMessage;

    return (uint8_t *)flowMessage;
}

int OFPipeline::getDBlevel() { return this->_DBlevel; }
OFTableMng *OFPipeline::getTablesMng() { return this->tablesMng; }
OFPortMng *OFPipeline::getPortsMng() { return this->portsMng; }

CLICK_ENDDECLS
ELEMENT_REQUIRES(OFTableMng OFPortMng)
//export our element
EXPORT_ELEMENT(OFPipeline)
