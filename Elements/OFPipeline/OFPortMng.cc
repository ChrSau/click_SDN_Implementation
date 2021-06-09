#include "OFPortMng.hh"

CLICK_DECLS

#define PATHTODEVICES "/sys/class/net/" /* Network Devices on Linux */

OFPortMng::OFPortMng(int DBlevel, ErrorHandler *, OFPipeline *pipeline)
{
    this->_DBlevel = DBlevel;

    this->_pipeline = pipeline;

    click_chatter("OFPortMng configured\n");
}

OFPortMng::OFPortMng(OFPortMng *in)
{
    this->_DBlevel = in->getDBlevel();
    this->_pipeline = in->getPipeline();
    this->numberOfPorts = (this->_pipeline->ninputs() - 1);
}
OFPortMng::OFPortMng(OFPortMng &in)
{
    this->_DBlevel = in.getDBlevel();
    this->_pipeline = in.getPipeline();
    this->numberOfPorts = (this->_pipeline->ninputs() - 1);
}

OFPortMng::~OFPortMng()
{
    this->cleanup();
}

void OFPortMng::cleanup()
{
    this->_pipeline = nullptr;
}

Packet *OFPortMng::push(int inPort, Packet *p)
{
    class WritablePacket *response = NULL;

    if (inPort > 0)
    {
        OF::Structs::click_port_comm_header *header = (OF::Structs::click_port_comm_header *)p->data();

        if (header->identifier == OF::Structs::CPCI_TO_PIPELINE)
        {
            switch (header->type)
            {
            case OF::Structs::CPCT_PORT_DESC:
            {
                struct OF::Structs::my_ofp_port newPortDesc;
                std::memcpy(&newPortDesc, header->data, header->length);
                newPortDesc.port_no = inPort;

#if CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
                newPortDesc.port_no = __builtin_bswap32(newPortDesc.port_no);
#endif
                this->portDesc.push_back(newPortDesc);

                if (this->portDesc.size() == ((size_t)this->_pipeline->noutputs() - 1))
                {
                    this->numberOfPorts++;

                    int multipartLength = sizeof(struct ofp_multipart_reply) + (header->length * this->portDesc.size());
                    struct ofp_multipart_reply *multipartResponse = (struct ofp_multipart_reply *)std::calloc(1, multipartLength);

                    for (size_t i = 0; i < this->portDesc.size(); i++)
                    {
#if CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
                        std::memcpy(multipartResponse->body + (i * sizeof(OF::Structs::my_ofp_port)), &this->portDesc[i], __builtin_bswap16(this->portDesc[i].length));
#elif CLICK_BYTE_ORDER == CLICK_BIG_ENDIAN
                        std::memcpy(multipartResponse->body + (i * sizeof(OF::Structs::my_ofp_port)), &this->portDesc[i], this->portDesc[i].length);
#endif
                    }

                    multipartResponse->header.type = OFPT_MULTIPART_REPLY;
                    multipartResponse->header.version = this->version != 0 ? this->version : 0x05;
                    multipartResponse->header.xid = this->xid;
                    multipartResponse->header.length = multipartLength;

                    multipartResponse->flags = this->numberOfPorts + 1 != this->_pipeline->ninputs() ? OFPMPF_REPLY_MORE : 0x0000;
                    multipartResponse->type = OFPMP_PORT_DESC;

#if CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
                    multipartResponse->header.length = __builtin_bswap16(multipartResponse->header.length);
                    multipartResponse->type = __builtin_bswap16(multipartResponse->type);
                    multipartResponse->flags = __builtin_bswap16(multipartResponse->flags);
#endif

                    response = Packet::make(multipartResponse, multipartLength);

                    this->_pipeline->output(0).push(response);

                    std::free(multipartResponse);
                    multipartResponse = nullptr;

                    response->kill();
                    response = nullptr;
                }

                break;
            }
            default:
                std::cerr << "ERROR: wrong click header type" << std::endl;
                break;
            }
        }
        else
        {
            p->kill();
        }

        header = nullptr;
    }
    else
    {

        struct ofp_header *requestHeader = (struct ofp_header *)p->data();

        switch (requestHeader->type)
        {
        case OFPT_PACKET_OUT:
        {
            struct OF::Structs::ofp_packet_out_v4 *request = (struct OF::Structs::ofp_packet_out_v4 *)p->data();

#if CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
            request->in_port = __builtin_bswap32(request->in_port);
#endif

            if (request->actions_len > 0 && request->in_port == OFPP_ANY)
            {
                if (this->_DBlevel > 10)
                    OF::Functions::printHexAndChar((uint8_t *)p->data(), p->length(), "PortMng OFPT_PACKET_OUT beginn: ");

                struct ofp_action_output *actionRequest = (struct ofp_action_output *)request->actions;

                uint16_t actionRequestLength = actionRequest->len;
#if CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
                actionRequestLength = __builtin_bswap16(actionRequestLength);
                actionRequest->port = __builtin_bswap32(actionRequest->port);
#endif

                if (_DBlevel > 10)
                    OF::Functions::printHex((uint8_t *)actionRequest, actionRequestLength, "actionRequest ");

                uint8_t *requestData = (uint8_t *)p->data();
                requestData += sizeof(struct OF::Structs::ofp_packet_out_v4) + actionRequestLength;
                uint16_t requestDataLength = p->length() - (sizeof(struct OF::Structs::ofp_packet_out_v4) + actionRequestLength);

                uint32_t headerLength = sizeof(struct OF::Structs::click_port_comm_header) + requestDataLength;
                struct OF::Structs::click_port_comm_header *header = (OF::Structs::click_port_comm_header *)std::calloc(1, headerLength);

                header->identifier = OF::Structs::CPCI_FROM_PIPELINE;
                header->type = OF::Structs::CPCT_PACKET_OUT;
                header->length = requestDataLength;
                std::memcpy(header->data, requestData, requestDataLength);

                if (_DBlevel > 10)
                    OF::Functions::printHex((uint8_t *)header, headerLength, "requestData ");

                response = Packet::make(header, headerLength);

                if (_DBlevel > 10)
                    OF::Functions::printHex((uint8_t *)response->data(), response->length(), "response: ");

                if (this->_pipeline->port_active(true, actionRequest->port))
                {
                    this->_pipeline->output(actionRequest->port).push(response);
                }

                if (response != nullptr)
                    response->kill();
                response = nullptr;

                requestData = nullptr;

                std::free(header);
                header = nullptr;

                actionRequest = nullptr;
            }
            else if (request->in_port != OFPP_ANY && request->actions->type == OFPAT_OUTPUT)
            {
                struct ofp_action_output *outPutActionRequest = (struct ofp_action_output *)request->actions;
#if CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
                outPutActionRequest->port = __builtin_bswap32(outPutActionRequest->port);
#endif

                uint32_t outPacketLength = p->length() - sizeof(struct OF::Structs::ofp_packet_out_v4) - sizeof(struct ofp_action_output);

                uint32_t headerLength = sizeof(struct OF::Structs::click_port_comm_header) + outPacketLength;
                struct OF::Structs::click_port_comm_header *header = (OF::Structs::click_port_comm_header *)std::calloc(1, headerLength);

                header->identifier = OF::Structs::CPCI_FROM_PIPELINE;
                header->type = OF::Structs::CPCT_PACKET_OUT;
                header->length = outPacketLength;

                std::memcpy(header->data, outPutActionRequest + 1, outPacketLength);

                if (outPutActionRequest->port == OFPP_FLOOD)
                {

                    for (size_t i = 1; i < (size_t)this->_pipeline->noutputs(); i++)
                    {
                        if (i != request->in_port)
                        {
                            this->_pipeline->output(i).push(Packet::make(header, headerLength));
                        }
                    }
                }
                else
                {
                    for (size_t i = 1; i < (size_t)this->_pipeline->noutputs(); i++)
                    {
                        if (i == outPutActionRequest->port)
                        {
                            this->_pipeline->output(i).push(Packet::make(header, headerLength));
                            break;
                        }
                    }
                }
                outPutActionRequest = nullptr;

                std::free(header);
                header = nullptr;
            }
            else
            {
                std::cerr << "Action Length <= 0 || in_port != OFPP_ANY" << std::endl;
            }

            request = nullptr;

            break;
        }
        case OFPT_MULTIPART_REQUEST:
        {
            ofp_multipart_request *request = (ofp_multipart_request *)p->data();
            if (request->type == OFPMP_PORT_DESC)
            {
                if (_DBlevel > 10)
                    OF::Functions::printHex((uint8_t *)p->data(), p->length(), "Input PortMng: ");

                this->xid = request->header.xid;
                this->version = request->header.version;

                OF::Structs::click_port_comm_header *header = (OF::Structs::click_port_comm_header *)std::calloc(1, sizeof(OF::Structs::click_port_comm_header));

                header->identifier = OF::Structs::CPCI_FROM_PIPELINE;
                header->type = OF::Structs::CPCT_PORT_DESC;
                header->length = 0;

                WritablePacket *output = Packet::make(header, sizeof(OF::Structs::click_port_comm_header));

                for (size_t i = 1; i < (size_t)this->_pipeline->ninputs(); i++)
                {
                    this->_pipeline->output(i).push(output->clone());
                }

                if (output != nullptr)
                    output->kill();

                std::free(header);
                header = nullptr;
            }
            request = nullptr;

            break;
        }
        default:
        {
            std::cerr << "ERROR: Switch Case OFPortMng::push()" << std::endl;
            break;
        }
        }

        requestHeader = nullptr;
        p->kill();
    }

    return response;
}

Packet *OFPortMng::selected(int , int , uint32_t *)
{
    Packet *p = NULL;

    return p;
}

Packet *OFPortMng::selected(int outport, Packet *p)
{
    switch (p->packet_type_anno())
    {
    case PACKET_BROADCAST:
    case PACKET_MULTICAST:
    {
        for (int i = 1; i < this->_pipeline->noutputs(); i++)
        {
            if (outport != i)
            {
                struct OF::Structs::click_port_comm_header *header = (struct OF::Structs::click_port_comm_header *)std::malloc(sizeof(OF::Structs::click_port_comm_header) + p->length());
                header->identifier = OF::Structs::CPCI_FROM_PIPELINE;
                header->type = OF::Structs::CPCT_PACKET_OUT;
                header->length = p->length();

                std::memcpy(header->data, p->data(), p->length());

                WritablePacket *output = Packet::make(header, sizeof(OF::Structs::click_port_comm_header) + p->length());

                std::free(header);
                header = nullptr;

                if (this->_DBlevel > 10)
                    std::cout << "Sending Broadcast to " << this->_pipeline->output(i).element()->name().c_str() << std::endl;

                this->_pipeline->output(i).push(output);

                if (output != nullptr)
                    output->kill();
            }
        }

        if (p != nullptr)
            p->kill();

        break;
    }
    case PACKET_OTHERHOST:
    {
        click_ether *etherHeader = (click_ether *)p->data();
        uint8_t etherAddr212[] = {ETH212};
        uint8_t etherAddr213[] = {ETH213};

        // 211
        if (OF::Functions::compareArray(etherHeader->ether_dhost, etherAddr212, ETH_ALEN))
        {
            OF::Structs::click_port_comm_header *header = (OF::Structs::click_port_comm_header *)std::malloc(sizeof(OF::Structs::click_port_comm_header) + p->length());
            header->identifier = OF::Structs::CPCI_FROM_PIPELINE;
            header->type = OF::Structs::CPCT_PACKET_OUT;
            header->length = p->length();

            std::memcpy(header->data, p->data(), p->length());

            WritablePacket *output = Packet::make(header, sizeof(OF::Structs::click_port_comm_header) + p->length());

            p->kill();

            std::free(header);
            header = nullptr;

            this->_pipeline->output(ETHERPORT).push(output);

            break;
        }
        else if (OF::Functions::compareArray(etherHeader->ether_dhost, etherAddr213, ETH_ALEN))
        {
            OF::Structs::click_port_comm_header *header = (OF::Structs::click_port_comm_header *)std::malloc(sizeof(OF::Structs::click_port_comm_header) + p->length());
            header->identifier = OF::Structs::CPCI_FROM_PIPELINE;
            header->type = OF::Structs::CPCT_PACKET_OUT;
            header->length = p->length();

            std::memcpy(header->data, p->data(), p->length());

            WritablePacket *output = Packet::make(header, sizeof(OF::Structs::click_port_comm_header) + p->length());

            p->kill();

            std::free(header);
            header = nullptr;
            if (PRIOWLAN > PRIOVLC || !this->_pipeline->getVLCPortStatus())
                this->_pipeline->output(WLANPORT).push(output);
            else
                this->_pipeline->output(VLCPORT).push(output);

            break;
        }

        // 214
        if (OF::Functions::compareArray(etherHeader->ether_dhost, etherAddr213, ETH_ALEN))
        {
            OF::Structs::click_port_comm_header *header = (OF::Structs::click_port_comm_header *)std::malloc(sizeof(OF::Structs::click_port_comm_header) + p->length());
            header->identifier = OF::Structs::CPCI_FROM_PIPELINE;
            header->type = OF::Structs::CPCT_PACKET_OUT;
            header->length = p->length();

            std::memcpy(header->data, p->data(), p->length());

            WritablePacket *output = Packet::make(header, sizeof(OF::Structs::click_port_comm_header) + p->length());

            p->kill();

            std::free(header);
            header = nullptr;

            this->_pipeline->output(ETHERPORT).push(output);

            break;
        }
        else if (OF::Functions::compareArray(etherHeader->ether_dhost, etherAddr212, ETH_ALEN))
        {
            OF::Structs::click_port_comm_header *header = (OF::Structs::click_port_comm_header *)std::malloc(sizeof(OF::Structs::click_port_comm_header) + p->length());
            header->identifier = OF::Structs::CPCI_FROM_PIPELINE;
            header->type = OF::Structs::CPCT_PACKET_OUT;
            header->length = p->length();

            std::memcpy(header->data, p->data(), p->length());

            WritablePacket *output = Packet::make(header, sizeof(OF::Structs::click_port_comm_header) + p->length());

            p->kill();

            std::free(header);
            header = nullptr;
            if (PRIOWLAN > PRIOVLC || !this->_pipeline->getVLCPortStatus())
                this->_pipeline->output(WLANPORT).push(output);
            else
                this->_pipeline->output(VLCPORT).push(output);

            break;
        }

        etherHeader = nullptr;

        return p;
    }
    case PACKET_HOST:
    case PACKET_OUTGOING:
    {
        if (this->_DBlevel > 10)
            OF::Functions::printHexAndChar((uint8_t *)p->data(), p->length(), "WARNING: Host or Outgoing: ");

        if (p != nullptr)
            p->kill();

        break;
    }
    default:
    {
        if (p != nullptr)
            p->kill();

        std::cerr << "ERROR: Wrong Packet Type Anno" << std::endl;

        break;
    }
    }

    return NULL;
}

bool OFPortMng::broadAndMulticast(Packet *, OFPhysicalPort *)
{
    return true;
}

int OFPortMng::getNumberOfPorts() { return (this->_pipeline->ninputs() - 1); }

CLICK_ENDDECLS
ELEMENT_REQUIRES(OFPhysicalPort)
//export our element
ELEMENT_PROVIDES(OFPortMng)