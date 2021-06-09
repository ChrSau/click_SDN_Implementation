//click includes

#include "OFResponder_Test.hh"

OFResponder::OFResponder(OFPipeline *inPipeline, TCPClient *tcp, std::queue<myPacket *> *socketQueue, std::mutex *queueLock)
{
    if (inPipeline == nullptr || inPipeline == NULL || tcp == nullptr || tcp == NULL)
        exit(EXIT_FAILURE);

    printf("OFResponer star config\n");

    this->pipeline = inPipeline;
    this->switchStatusFlags = 0x0000;
    this->tcp = tcp;

    this->switchDesc = new OFSwitchDesc("SEW-EURODRIVE GmbH & Co KG",
                                        "Click Router",
                                        "0.0.1",
                                        "None",
                                        "None");

    this->socketQueue = socketQueue;
    this->queueLock = queueLock;

    printf("OFResponer configured\n");
}

OFResponder::~OFResponder()
{
    this->multipartMsg.clear();

    if (this->switchDesc != nullptr)
        delete this->switchDesc;
}

myPacket *OFResponder::simple_action(myPacket *p)
{
    if (p != nullptr || p != NULL)
    {

        // OF::Functions::printHexAndChar(p->data(), p->length(), "OFResponder in: ");

        struct ofp_header *header = (struct ofp_header *)p->data();

        class myPacket *response = new myPacket();

        if (header->version == 0x06 || header->version == 0x05)
        {
            this->OFVersion = header->version;
        }
        else
        {
        }

        uint16_t packetLength = header->length;
#if __BYTE_ORDER == __LITTLE_ENDIAN
        packetLength = __builtin_bswap16(packetLength);
#endif
        if (p->length() > packetLength)
        {
            class myPacket *backToQueue = new myPacket();
            backToQueue->make(&p->data()[packetLength], p->length() - packetLength);

            queueLock->lock();
            socketQueue->push(backToQueue);
            queueLock->unlock();

            class myPacket *forward = new myPacket();
            forward->make(p->data(), packetLength);

            myPacket *newP = p;
            p = forward;
            delete newP;

            // OF::Functions::printHexAndChar(backToQueue->data(), backToQueue->length(), "backToQueue: ");
            // OF::Functions::printHexAndChar(p->data(), p->length(), "To Pipeline: ");

            header = (struct ofp_header *)p->data();
        }

        switch (header->type)
        {
        case OFPT_MULTIPART_REPLY:
        {

            if (this->_DBlevel > 10)
                OF::Functions::printHex((uint8_t *)p->data(), p->length(), "Input MULTIPART_REPLY");

            response->make(p->data(), p->length());

            // output(0).push(response);
            p->kill();

            break;
        }
        case OFPT_HELLO:
        {
            printf("OFPT_HELLO\n");

            struct ofp_hello *responseData = (ofp_hello *)std::calloc(1, sizeof(struct ofp_hello) + sizeof(struct ofp_hello_elem_versionbitmap) + sizeof(uint32_t));
            responseData->header.type = header->type;
            responseData->header.version = this->OFVersion;
            responseData->header.length = (sizeof(struct ofp_header) + sizeof(struct ofp_hello_elem_versionbitmap) + sizeof(uint32_t));
            responseData->header.xid = header->xid;

            struct ofp_hello_elem_versionbitmap *helloElem = (ofp_hello_elem_versionbitmap *)std::calloc(1, sizeof(struct ofp_hello_elem_versionbitmap) + sizeof(uint32_t));

            helloElem->length = sizeof(struct ofp_hello_elem_versionbitmap) + sizeof(uint32_t);
            helloElem->type = OFPHET_VERSIONBITMAP;
            uint32_t bitmap = 0x60; // Wir unterstützen die Versionen 1.4 und 1.5

#if __BYTE_ORDER == __LITTLE_ENDIAN
            responseData->header.length = __builtin_bswap16(responseData->header.length);
            helloElem->length = __builtin_bswap16(helloElem->length);
            helloElem->type = __builtin_bswap16(helloElem->type);
            bitmap = __builtin_bswap32(bitmap);
#endif

            std::memcpy(helloElem->bitmaps, &bitmap, sizeof(uint32_t));
            std::memcpy(responseData->elements, helloElem, sizeof(struct ofp_hello_elem_versionbitmap) + sizeof(uint32_t));

            response->make((uint8_t *)responseData, sizeof(struct ofp_hello) + sizeof(struct ofp_hello_elem_versionbitmap) + sizeof(uint32_t));

            // output(0).push(response);

            std::free(responseData);
            std::free(helloElem);
            responseData = nullptr;
            helloElem = nullptr;

            break;
        }
        case OFPT_FEATURES_REQUEST:
        {
            printf("OFPT_FEATURES_REQUEST\n");

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

#if __BYTE_ORDER == __BIG_ENDIAN
            responseMsg.header.length = sizeof(struct ofp_switch_features);
#elif __BYTE_ORDER == __LITTLE_ENDIAN
            responseMsg.header.length = __builtin_bswap16(sizeof(struct ofp_switch_features));
            responseMsg.datapath_id = __builtin_bswap64(responseMsg.datapath_id);
            responseMsg.n_buffers = __builtin_bswap32(responseMsg.n_buffers);
            responseMsg.capabilities = __builtin_bswap32(responseMsg.capabilities);
#endif

            response->make((uint8_t *)&responseMsg, sizeof(struct ofp_switch_features));

            break;
        }
        case OFPT_BARRIER_REQUEST:
        {
            printf("OFPT_BARRIER_REQUEST\n");

            struct ofp_header *responseMsg = (ofp_header *)std::calloc(1, sizeof(struct ofp_header));
            responseMsg->length = sizeof(struct ofp_header);
            responseMsg->type = OFPT_BARRIER_REPLY;
            responseMsg->version = header->version;
            responseMsg->xid = header->xid;
#if __BYTE_ORDER == __LITTLE_ENDIAN
            responseMsg->length = __builtin_bswap16(responseMsg->length);
#endif

            response->make((uint8_t *)responseMsg, sizeof(struct ofp_header));

            std::free(responseMsg);
            responseMsg = nullptr;

            break;
        }
        case OFPT_SET_CONFIG:
        {
            printf("OFPT_SET_CONFIG\n");

            struct ofp_switch_config *request = (ofp_switch_config *)p->data();

            this->switchStatusFlags = request->flags;
            uint16_t send_len = request->miss_send_len;

            if (request->header.length != p->length())
            {
                request++;

                if (request->header.type == OFPT_BARRIER_REQUEST)
                {
                    struct ofp_header *request2 = (struct ofp_header *)request;
                    struct ofp_header *responseMsg = (struct ofp_header *)std::calloc(1, sizeof(struct ofp_header));
                    responseMsg->length = sizeof(struct ofp_header);
                    responseMsg->type = OFPT_BARRIER_REPLY;
                    responseMsg->version = request->header.version;
                    responseMsg->xid = request->header.xid;
#if __BYTE_ORDER == __LITTLE_ENDIAN
                    responseMsg->length = __builtin_bswap16(responseMsg->length);
#endif

                    // myPacket *responseBarrier = new myPacket();
                    response->make((uint8_t *)responseMsg, sizeof(struct ofp_header));

                    OF::Functions::printHex((uint8_t *)responseMsg, sizeof(struct ofp_header), "OFPT_BARRIER_REPLY: ");

                    this->tcp->Send((uint8_t *)responseMsg, sizeof(struct ofp_header));
                    // output(0).push(response);

                    std::free(responseMsg);
                    responseMsg = nullptr;

                    request2++;

                    if (request2->type == OFPT_GET_CONFIG_REQUEST)
                    {

                        struct ofp_switch_config *responseMsg = (ofp_switch_config *)std::calloc(1, sizeof(struct ofp_switch_config));

                        responseMsg->header.length = sizeof(struct ofp_switch_config);
                        responseMsg->header.type = OFPT_GET_CONFIG_REPLY;
                        responseMsg->header.version = request->header.version;
                        responseMsg->header.xid = request2->xid;

                        responseMsg->flags = this->switchStatusFlags;
                        responseMsg->miss_send_len = send_len;

#if __BYTE_ORDER == __LITTLE_ENDIAN
                        responseMsg->header.length = __builtin_bswap16(responseMsg->header.length);
                        responseMsg->miss_send_len = __builtin_bswap16(responseMsg->miss_send_len);
#endif
                        response->make((uint8_t *)responseMsg, sizeof(struct ofp_switch_config));

                        std::free(responseMsg);
                        responseMsg = nullptr;
                    }
                    request2 = nullptr;
                }
            }

            request = nullptr;

            break;
        }
        case OFPT_GET_CONFIG_REQUEST:
        {

            printf("OFPT_GET_CONFIG_REQUEST\n");

            struct ofp_switch_config *responseMsg = (ofp_switch_config *)std::calloc(1, sizeof(struct ofp_switch_config));

            responseMsg->header.length = sizeof(struct ofp_switch_config);
            responseMsg->header.type = OFPT_GET_CONFIG_REPLY;
            responseMsg->header.version = header->version;
            responseMsg->header.xid = header->xid;
            responseMsg->flags = OFPC_FRAG_NORMAL | OFPC_FRAG_DROP;
            responseMsg->miss_send_len = 0x00c8;
#if __BYTE_ORDER == __LITTLE_ENDIAN
            responseMsg->header.length = __builtin_bswap16(responseMsg->header.length);

            responseMsg->flags = __builtin_bswap16(responseMsg->flags);
            responseMsg->miss_send_len = __builtin_bswap16(responseMsg->miss_send_len);
#endif

            response->make((uint8_t *)responseMsg, sizeof(struct ofp_switch_config));

            std::free(responseMsg);
            responseMsg = nullptr;

            break;
        }
        case OFPT_FLOW_MOD:
        {
            printf("OFPT_FLOW_MOD\n");

            // output(2).push(p);

            break;
        }
        case OFPT_GROUP_MOD:
        {
            printf("OFPT_GROUP_MOD\n");

            break;
        }
        case OFPT_PACKET_OUT:
        {

            printf("OFPT_PACKET_OUT\n");

            // output(2).push(p);

            break;
        }
        case OFPT_ROLE_REQUEST:
        {

            printf("OFPT_ROLE_REQUEST\n");

            struct ofp_role_request *request = (struct ofp_role_request *)p->data();
#if __BYTE_ORDER == __LITTLE_ENDIAN
            request->generation_id = __builtin_bswap64(request->generation_id);
            request->role = __builtin_bswap32(request->role);
#endif

            bool check = true;

            for (auto &&controller : this->controllers)
            {
                if (controller.rolle == OFPCR_ROLE_MASTER && controller.generationId == request->generation_id)
                {
                    struct ofp_error_msg responseMsg;
                    responseMsg.code = OFPRRFC_STALE;
                    responseMsg.type = OFPET_ROLE_REQUEST_FAILED;
                    responseMsg.header.type = OFPT_ERROR;
                    responseMsg.header.version = this->OFVersion;
                    responseMsg.header.length = sizeof(struct ofp_error_msg);
#if __BYTE_ORDER == __LITTLE_ENDIAN
                    responseMsg.code = __builtin_bswap16(responseMsg.code);
                    responseMsg.type = __builtin_bswap16(responseMsg.type);
                    responseMsg.header.length = __builtin_bswap16(responseMsg.header.length);
#endif
                    response->make((uint8_t *)&responseMsg, sizeof(struct ofp_error_msg));

                    check = false;

                    break;
                }
            }

            if (check)
            {

                for (auto &&contr : this->controllers)
                {
                    if (contr.rolle == OFPCR_ROLE_MASTER)
                        contr.rolle = OFPCR_ROLE_SLAVE;
                }

                struct OF::Structs::controllerData controller;

                controller.generationId = request->generation_id;
                controller.OFVersion = request->header.version;
                controller.rolle = request->role;

                this->controllers.push_back(controller);

                // ofp_role_reply == ofp_role_request
                struct ofp_role_request responseMsg;

                responseMsg.header.length = 24; // sizeof(struct ofp_role_request);
                responseMsg.header.xid = request->header.xid;
                responseMsg.header.version = this->OFVersion;
                responseMsg.header.type = OFPT_ROLE_REPLY;

                responseMsg.generation_id = request->generation_id;
                responseMsg.role = request->role;

#if __BYTE_ORDER == __LITTLE_ENDIAN
                responseMsg.header.length = __builtin_bswap16(responseMsg.header.length);

                responseMsg.generation_id = __builtin_bswap64(responseMsg.generation_id);
                responseMsg.role = __builtin_bswap32(responseMsg.role);
#endif

                response->make((uint8_t *)&responseMsg, sizeof(struct ofp_role_request));
            }

            request = nullptr;

            break;
        }
        case OFPT_MULTIPART_REQUEST:
        {

            printf("Multipart Request\n");
            struct ofp_multipart_request *request = (ofp_multipart_request *)p->data();

#if __BYTE_ORDER == __LITTLE_ENDIAN
            request->type = __builtin_bswap16(request->type);
            request->flags = __builtin_bswap16(request->flags);
#endif

            if (request->flags > 0)
            {
                if (!this->multipartMsg.empty())
                {
                    if ((this->multipartMsg.back().header.xid == request->header.xid) && this->multipartMsg.back().type == request->type)
                    {
                        this->multipartMsg.push_back(*request);
                    }
                    else
                    {
                        struct ofp_error_msg responseMsg;
                        responseMsg.header.type = OFPT_ERROR;
                        responseMsg.header.version = request->header.version;
                        responseMsg.header.xid = request->header.xid;
                        responseMsg.code = OFPBRC_BAD_MULTIPART;
                        responseMsg.type = OFPET_BAD_REQUEST;

#if __BYTE_ORDER == __BIG_ENDIAN
                        responseMsg.length = sizeof(struct ofp_error_msg);
#elif __BYTE_ORDER == __LITTLE_ENDIAN
                        responseMsg.header.length = __builtin_bswap16(sizeof(struct ofp_error_msg));
                        responseMsg.code = __builtin_bswap16(responseMsg.code);
                        responseMsg.type = __builtin_bswap16(responseMsg.type);
#endif

                        response->make((uint8_t *)&responseMsg, sizeof(struct ofp_error_msg));
                        // output(0).push(response);

                        this->multipartMsg.clear();
                    }
                }
                else
                {
                    this->multipartMsg.push_back(*request);
                }
            }
            else
            {
                this->multipartMsg.push_back(*request);

                switch (this->multipartMsg[0].type)
                {
                case OFPMP_TABLE_FEATURES:
                {
                    printf("OFPMP_TABLE_FEATURES\n");

                    // output(2).push(p);

                    response = this->pipeline->simple_action(0, p);

                    this->multipartMsg.clear();
                    break;
                }
                case OFPMP_DESC:
                {
                    printf("OFPMP_DESC\n");

                    struct ofp_multipart_reply *responseMsg = (ofp_multipart_reply *)std::calloc(1, sizeof(struct ofp_multipart_reply) + sizeof(struct ofp_desc));

                    responseMsg->header.type = OFPT_MULTIPART_REPLY;
                    responseMsg->header.version = this->multipartMsg[0].header.version;
                    responseMsg->header.xid = this->multipartMsg[0].header.xid;
                    responseMsg->header.length = sizeof(struct ofp_multipart_reply) + sizeof(struct ofp_desc);

                    responseMsg->type = OFPMP_DESC;
                    responseMsg->flags = 0x0000;
                    // this->switchDesc = new OFSwitchDesc();

                    struct ofp_desc *responseSwitchDesc = this->switchDesc->getSwitchDesc();

                    if (responseSwitchDesc != nullptr || responseSwitchDesc != NULL)
                    {
                        std::memcpy(responseMsg->body, responseSwitchDesc, sizeof(struct ofp_desc));
                    }
                    else
                    {
                        std::cerr << "SwitchDesc is nullptr or NULL" << std::endl;
                    }

#if __BYTE_ORDER == __LITTLE_ENDIAN
                    responseMsg->flags = __builtin_bswap16(responseMsg->flags);
                    responseMsg->type = __builtin_bswap16(responseMsg->type);

                    responseMsg->header.length = __builtin_bswap16(responseMsg->header.length);
#endif

                    response->make((uint8_t *)responseMsg, sizeof(struct ofp_multipart_reply) + sizeof(struct ofp_desc));

                    // this->tcp->Send((uint8_t *)responseMsg, sizeof(struct ofp_multipart_reply) + sizeof(struct ofp_desc));

                    // output(0).push(response);

                    std::free(responseMsg);
                    responseMsg = nullptr;

                    this->multipartMsg.clear();

                    break;
                }
                case OFPMP_PORT_DESC:
                {

                    printf("Port Description\n");

                    response = this->pipeline->simple_action(0, p);
                    // output(2).push(p);

                    this->multipartMsg.clear();

                    break;
                }
                case OFPMP_AGGREGATE_STATS:
                {

                    printf("OFPMP_AGGREGATE_STATS\n");

                    // output(2).push(p);

                    this->multipartMsg.clear();

                    break;
                }
                default:
                {

                    printf("Keine Ahnung Multipart\n");

                    this->multipartMsg.clear();

                    break;
                }
                }
            }

            request = nullptr;
            break;
        }
        case OFPT_ECHO_REQUEST:
        {
            printf("OFPT_ECHO_REPLY");
            struct ofp_header responseData;
            responseData.type = OFPT_ECHO_REPLY;
            responseData.version = OFP_VERSION - 1;
            responseData.length = 0x0800; //sizeof(struct ofp_header);
            responseData.xid = header->xid;

            response->make((uint8_t *)&responseData, sizeof(struct ofp_header));
            // output(0).push(response);

            break;
        }
        default:
        {
            printf("Keine Ahnung welcher Header\n");

            OF::Functions::printHex(p->data(), p->length(), "Unbekannter Header: ");

            response = nullptr;

            // output(0).push(p);
            break;
        }
        }

        lastMessage = p;

        // output(1).push(p);

        header = nullptr;
        p->kill();

        return response;
    }
    else
    {
        return NULL;
    }
}
