//our includes
#include "OFPortMng.hh"

OFPortMng::OFPortMng(int DBlevel)
{

    printf("OFPortMng start config\n");
    this->_DBlevel = DBlevel;

    int i = 0;

    DIR *dp;
    struct dirent *dirp;
    if ((dp = opendir(PATHTODEVICES)) == NULL)
    {
        std::cout << "Error(" << errno << ") opening " << PATHTODEVICES << std::endl;
        exit(EXIT_FAILURE);
    }

    while ((dirp = readdir(dp)) != NULL)
    {
        std::string deviceName = dirp->d_name;
        if (deviceName.compare("lo") != 0 && deviceName.compare(".") != 0 && deviceName.compare("..") != 0) // We don't need the Loop Device
        {
            if (true)
            {
                if (true)
                {
                    std::string deviceState;
                    std::string deviceOperstate = PATHTODEVICES + deviceName + "/operstate";
                    std::string deviceSpeed = PATHTODEVICES + deviceName + "/speed";
                    std::string deviceAddress = PATHTODEVICES + deviceName + "/address";
                    std::string deviceDuplex = PATHTODEVICES + deviceName + "/duplex";

                    std::ifstream operstateFile(deviceOperstate, std::ios_base::in);
                    std::ifstream addressFile(deviceAddress, std::ios_base::in);
                    std::ifstream speedFile(deviceSpeed, std::ios_base::in);
                    std::ifstream duplexFile(deviceDuplex, std::ios_base::in);

                    uint8_t hwAddrArray[OFP_ETH_ALEN];
                    std::string speed;
                    std::string duplex;

                    if (_DBlevel > 10)
                        std::cout << deviceName << ": \n    State: ";

                    if (operstateFile.is_open())
                    {
                        getline(operstateFile, deviceState);

                        if (_DBlevel > 10)
                            std::cout << deviceState;

                        this->numberOfPorts++;

                        if (_DBlevel > 10)
                            std::cout << " HW Address: ";

                        if (addressFile.is_open())
                        {
                            std::string hwAddr;
                            getline(addressFile, hwAddr);

                            if (_DBlevel > 10)
                                std::cout << hwAddr;

                            for (size_t j = 0; j < OFP_ETH_ALEN; j++)
                            {
                                hwAddrArray[j] = std::stoi(hwAddr.substr(j * 3, 2), 0, 16);
                            }
                        }
                        else
                        {
                            if (_DBlevel > 10)
                                std::cout << "Unable to open Address";
                        }

                        if (deviceState != "down")
                        {
                            if (_DBlevel > 10)
                                std::cout << " Speed: ";

                            if (speedFile.is_open())
                            {
                                getline(speedFile, speed);

                                if (_DBlevel > 10)
                                    std::cout << speed << "MB/s";
                            }
                            else
                            {
                                if (_DBlevel > 10)
                                    std::cout << "Unable to open Address";
                            }

                            if (_DBlevel > 10)
                                std::cout << " Duplex: ";

                            if (duplexFile.is_open())
                            {
                                getline(duplexFile, duplex);

                                if (_DBlevel > 10)
                                    std::cout << duplex;
                            }
                            else
                            {
                                if (_DBlevel > 10)
                                    std::cout << "Unable to open Address";
                            }
                        }

                        if (_DBlevel > 10)
                            std::cout << std::endl;

                        OFPhysicalPort *physicalport = new OFPhysicalPort(i,
                                                                          hwAddrArray,
                                                                          deviceName,
                                                                          0,
                                                                          (deviceState != "down") ? OFPPS_LIVE : OFPPS_LINK_DOWN,
                                                                          OFPPDPT_ETHERNET,
                                                                          ((speed == "10") ? ((duplex == "half") ? OFPPF_10MB_HD : OFPPF_10MB_FD) : (speed == "100") ? ((duplex == "half") ? OFPPF_100MB_HD : OFPPF_100MB_FD) : (speed == "1000") ? ((duplex == "half") ? OFPPF_1GB_HD : OFPPF_1GB_FD) : OFPPF_OTHER) | OFPPF_COPPER,
                                                                          0,
                                                                          0,
                                                                          0,
                                                                          (speed.empty()) ? 0 : (std::stoi(speed, 0, 10) * 1000),
                                                                          (speed.empty()) ? 0 : (std::stoi(speed, 0, 10) * 1000));

                        this->_OFPhysicalPorts.push_back(physicalport);

                        addressFile.close();
                        speedFile.close();
                        operstateFile.close();
                        duplexFile.close();
                    }
                    else
                    {
                        std::cout << "Unable to open operstate" << std::endl;
                    }
                    i++;
                }
            }
        }
    }

    // LOCALPORT
    // 55:55:55:55:55:55
    uint8_t hwAddrArray2[] = {0x55, 0x55, 0x55, 0x55, 0x55, 0x55};
    OFPhysicalPort *physicalport = new OFPhysicalPort(OFPP_LOCAL,
                                                      hwAddrArray2,
                                                      "LocalPort",
                                                      0,
                                                      OFPPS_LIVE,
                                                      OFPPDPT_ETHERNET,
                                                      OFPPF_100MB_FD | OFPPF_COPPER,
                                                      0,
                                                      0,
                                                      0,
                                                      0x19000,
                                                      0x19000);

    if (_DBlevel > 10)
        OF::Functions::printHex((uint8_t *)physicalport->getPortStruct(), physicalport->getPortStructSize(), "physicalport.getPortStruct(): ");

    this->_OFPhysicalPorts.push_back(physicalport);
    if (_DBlevel > 10)
        OF::Functions::printHex((uint8_t *)this->_OFPhysicalPorts[0]->getPortStruct(), this->_OFPhysicalPorts[0]->getPortStructSize(), "this->_OFPhysicalPorts[0].getPortStruct(): ");

    std::free(dp);
    std::free(dirp);
    dp = nullptr;
    dirp = nullptr;

    // // Add Live Ports to Element Task.
    // for (auto Port : this->_OFPhysicalPorts)
    // {
    //     uint32_t portNo = Port.getPortNo();

    //     if (Port.getPortState() == OFPPS_LIVE && portNo != OFPP_LOCAL)
    //     {
    //         if (Port.fd() >= 0)
    //         {
    //             add_select(Port.fd(), SELECT_READ);
    //         }
    //     }
    // }

    std::vector<uint8_t> eno2 = {143, 93, 153, 203};
    std::vector<uint8_t> enx24f5a2f13a13 = {192, 168, 2, 52};

    this->ownIpAddr.push_back(eno2);
    this->ownIpAddr.push_back(enx24f5a2f13a13);

    printf("OFPortMng configured\n");
}

OFPortMng::~OFPortMng()
{
    for (auto physicalPort : this->_OFPhysicalPorts)
    {
        delete physicalPort;
    }

    this->_OFPhysicalPorts.clear();
}

void OFPortMng::cleanup()
{
}

myPacket *OFPortMng::push(int port, myPacket *p)
{

    struct ofp_header *requestHeader = (struct ofp_header *)p->data();

    OF::Functions::printHexAndChar(p->data(), p->length(), "OFPortMng push: ");

    myPacket *response = new myPacket();

    switch (requestHeader->type)
    {
    case OFPT_PACKET_OUT:
    {
        struct OF::Structs::ofp_packet_out_v4 *request = (struct OF::Structs::ofp_packet_out_v4 *)p->data();

#if __BYTE_ORDER == __LITTLE_ENDIAN
        request->in_port = __builtin_bswap32(request->in_port);
#endif

        /* 
        *   TODO:
        *       Überprüfung wo welcher Port ist. Im Moment habe ich nur einen Output.
        */

        if (request->actions_len > 0 && request->in_port == OFPP_ANY)
        {
            struct ofp_action_output *actionRequest = (struct ofp_action_output *)request->actions;

            uint16_t actionRequestLength = actionRequest->len;
#if __BYTE_ORDER == __LITTLE_ENDIAN
            actionRequestLength = __builtin_bswap16(actionRequestLength);
            actionRequest->port = __builtin_bswap32(actionRequest->port);
#endif

            if (_DBlevel > 10)
                OF::Functions::printHex((uint8_t *)actionRequest, actionRequestLength, "actionRequest ");

            if (this->_OFPhysicalPorts[actionRequest->port]->getPortState() == OFPPS_LIVE)
            {
                uint8_t *requestData = (uint8_t *)p->data();
                requestData += sizeof(struct OF::Structs::ofp_packet_out_v4) + actionRequestLength;
                uint16_t requestDataLength = p->length() - (sizeof(struct OF::Structs::ofp_packet_out_v4) + actionRequestLength);

                if (_DBlevel > 10)
                    OF::Functions::printHex((uint8_t *)requestData, requestDataLength, "requestData ");

                myPacket *newResponse = new myPacket();
                newResponse->make(requestData, requestDataLength);
                this->_OFPhysicalPorts[actionRequest->port]->send_packet(newResponse);

                newResponse = nullptr;

                requestData = nullptr;
            }
            else
            {
                std::cerr << "der Port ist nicht Live" << std::endl;
            }
            actionRequest = nullptr;
        }
        else if (request->in_port != OFPP_ANY && request->actions->type == OFPAT_OUTPUT)
        {
            struct ofp_action_output *outPutActionRequest = (struct ofp_action_output *)request->actions;
#if __BYTE_ORDER == __LITTLE_ENDIAN
            outPutActionRequest->port = __builtin_bswap32(outPutActionRequest->port);
#endif

            if (outPutActionRequest->port == OFPP_FLOOD)
            {
                uint32_t outPacketLength = p->length() - sizeof(struct OF::Structs::ofp_packet_out_v4) - sizeof(struct ofp_action_output);

                for (unsigned int i = 0; i < this->_OFPhysicalPorts.size(); i++)
                {
                    if (this->_OFPhysicalPorts[i]->getPortNo() != request->in_port)
                    {
                        myPacket *newResponse = new myPacket();
                        newResponse->make((uint8_t *)outPutActionRequest + 1, outPacketLength);
                        this->_OFPhysicalPorts[i]->send_packet(newResponse);
                    }
                }
            }
            else
            {
                uint32_t outPacketLength = p->length() - sizeof(struct OF::Structs::ofp_packet_out_v4) - sizeof(struct ofp_action_output);
                for (unsigned int i = 0; i < this->_OFPhysicalPorts.size(); i++)
                {
                    if (this->_OFPhysicalPorts[i]->getPortNo() == outPutActionRequest->port)
                    {
                        myPacket *newResponse = new myPacket();
                        newResponse->make((uint8_t *)outPutActionRequest + 1, outPacketLength);
                        this->_OFPhysicalPorts[i]->send_packet(newResponse);
                        break;
                    }
                }
            }
            outPutActionRequest = nullptr;
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

            uint32_t sizeOfStruct = sizeof(struct ofp_multipart_reply) + (this->_OFPhysicalPorts.size() * sizeof(struct OF::Structs::my_ofp_port));

            struct ofp_multipart_reply *responseMsg = (ofp_multipart_reply *)std::calloc(1, sizeOfStruct);

            for (size_t i = 0; i < this->_OFPhysicalPorts.size(); i++)
            {
                std::memcpy(responseMsg->body + (i * sizeof(struct OF::Structs::my_ofp_port)), this->_OFPhysicalPorts[i]->getPortStruct(), sizeof(struct OF::Structs::my_ofp_port));
                if (_DBlevel > 10)
                    OF::Functions::printHex((uint8_t *)this->_OFPhysicalPorts[i]->getPortStruct(), this->_OFPhysicalPorts[i]->getPortStructSize(), "this->_OFPhysicalPorts[i].getPortStruct(): ");
            }

            responseMsg->header.type = OFPT_MULTIPART_REPLY;
            responseMsg->header.version = request->header.version;
            responseMsg->header.xid = request->header.xid;
            responseMsg->header.length = sizeOfStruct;

            responseMsg->type = OFPMP_PORT_DESC;
            responseMsg->flags = 0;
#if __BYTE_ORDER == __LITTLE_ENDIAN
            responseMsg->header.length = __builtin_bswap16(responseMsg->header.length);
            responseMsg->type = __builtin_bswap16(responseMsg->type);
            responseMsg->flags = __builtin_bswap16(responseMsg->flags);
#endif

            response->make((uint8_t *)responseMsg, sizeOfStruct);

            if (_DBlevel > 10)
                OF::Functions::printHex((uint8_t *)response->data(), response->length(), "Output PortMng: ");

            std::free(responseMsg);
            responseMsg = nullptr;
        }
        request = nullptr;

        break;
    }
    default:
    {
        break;
    }
    }

    // output(0).push(p);

    requestHeader = nullptr;
    p->kill();

    return response;
}

myPacket *OFPortMng::selected(int fd, int mask, uint32_t *outport)
{
    myPacket *p = NULL;

    for (auto port : this->_OFPhysicalPorts)
    {
        if (port->fd() == fd)
        {
            p = port->selected(/*fd, mask*/);

            if (p != NULL)
            {

                if (_DBlevel > 10)
                    OF::Functions::printHex((uint8_t *)p->data(), p->length(), port->getPortStruct()->name);

                const struct OF::Structs::ethernet_header *etherHeader = p->ether_header();
                // printHex((uint8_t *)&etherHeader->ether_type, 2, "Ether Type: ");
                // std::cout << std::dec << etherHeader->ether_type << std::endl;

                uint16_t etherType = etherHeader->type;
#if __BYTE_ORDER == __LITTLE_ENDIAN
                etherType = __builtin_bswap16(etherType);
#endif

                bool checkIp;
                bool broadcastCheck;
                bool multicastCheck;
                bool ip6MultiCastCheck;

                if (etherType == ETHERTYPE_IP)
                {
                    // const struct click_ip *ipHeader = p->ip_header();

                    if (_DBlevel > 10)
                        std::cout << "IPv4" << std::endl;
                    // uint8_t *etherHeaderUint = (uint8_t *)etherHeader;
                    struct OF::Structs::my_ip_header *ipHeader = (struct OF::Structs::my_ip_header *)(((uint8_t *)etherHeader) + 14);

                    // ipHeaderUint += 15;
                    if (_DBlevel > 10)
                        OF::Functions::printHex(ipHeader->dest, 4, "IP Destination: ");

                    for (auto ipAddr : this->ownIpAddr)
                    {
                        checkIp = true;
                        for (size_t i = 0; i < 4; i++)
                        {
                            if (ipAddr[i] != ipHeader->dest[i])
                            {
                                checkIp = true;
                                break;
                            }
                            else
                            {
                                checkIp = false;
                            }
                        }
                        if (!checkIp)
                        {
                            if (_DBlevel > 10)
                                std::cout << "Drop" << std::endl;
                            break;
                        }
                    }
                }

                broadcastCheck = true;
                for (size_t i = 0; i < 6; i++)
                {
                    if (etherHeader->dst.addr[i] != this->broadcastPort[i])
                    {
                        broadcastCheck = false;
                        break;
                    }
                }

                multicastCheck = true;
                for (size_t i = 0; i < 3; i++)
                {
                    if (etherHeader->dst.addr[i] != this->multicastPort[i])
                    {
                        multicastCheck = false;
                        break;
                    }
                }

                ip6MultiCastCheck = true;
                for (size_t i = 0; i < 2; i++)
                {
                    if (etherHeader->dst.addr[i] != this->ip6MultiCastPort[i])
                    {
                        ip6MultiCastCheck = false;
                        break;
                    }
                }

                if (broadcastCheck || multicastCheck || ip6MultiCastCheck)
                {
                    for (unsigned int i = 0; i < this->_OFPhysicalPorts.size(); i++)
                    {
                        if (this->_OFPhysicalPorts[i]->getPortNo() != port->getPortNo())
                        {
                            this->_OFPhysicalPorts[i]->send_packet(p->clone());
                        }
                    }
                    if (_DBlevel > 10)
                        std::cout << "Broadcast!!!" << std::endl;
                }

                if (broadcastCheck || multicastCheck || ip6MultiCastCheck || !checkIp)
                {

                    p->kill();
                    p = NULL;
                    // output(0).push(p->clone());
                }
                else
                {
                    *outport = port->getPortNo();
                    if (*outport == 3)
                        std::cout << "Port 3 myPacket" << std::endl;
                }
            }
            break;
        }
    }

    return p;
}
