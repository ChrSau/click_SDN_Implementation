#include "OFInterfaceWrapper.hh"

CLICK_DECLS

OFInterfaceWrapper::OFInterfaceWrapper()
    : _timer(this), _interval(0), _VLC(false)
{
    this->_portStruct = (struct OF::Structs::my_ofp_port *)std::calloc(1, sizeof(struct OF::Structs::my_ofp_port));
    this->_portCounter = (struct OF::Structs::my_ofp_port_counter *)std::calloc(1, sizeof(struct OF::Structs::my_ofp_port_counter));
}
OFInterfaceWrapper::OFInterfaceWrapper(OFInterfaceWrapper *&) {}
OFInterfaceWrapper::~OFInterfaceWrapper()
{
    std::free(this->_portStruct);
    this->_portStruct = nullptr;

    std::free(this->_portCounter);
    this->_portCounter = nullptr;
}

void OFInterfaceWrapper::cleanup(CleanupStage)
{
    this->~OFInterfaceWrapper();
}

int OFInterfaceWrapper::initialize(ErrorHandler *errh)
{
    if (this->_VLC)
    {
        this->_timer.initialize(this);
        if (_interval != 0)
            this->_timer.schedule_after_msec(_interval);
    }

    return EXIT_SUCCESS;
}

int OFInterfaceWrapper::configure(Vector<String> &conf, ErrorHandler *errh)
{

    String ifname;
    String speed;
    String duplex;

    if (Args(conf, this, errh)
            .read_mp("IFNAME", ifname)
            .read("SPEED", speed)
            .read("DUPLEX", duplex)
            .read("DB", this->_DBlevel)
            .read("INTERVAL", this->_interval)
            .read("VLC", this->_VLC)
            .complete() < 0)
    {
        return -EXIT_FAILURE;
    }

    std::cout << (this->_VLC == true ? "VLC check" : "NO VLC") << std::endl;

    EtherAddress *etherAddr = new EtherAddress();
    if (!cp_ethernet_address(ifname, etherAddr, 0))
    {
        std::cerr << "ERROR: Could't parse Ethernet Address" << std::endl;
        return -EXIT_FAILURE;
    }

    std::string stdHW_Addr(etherAddr->unparse().c_str());

    for (size_t i = 0; i < OFP_ETH_ALEN; i++)
    {
        this->_portStruct->hw_addr[i] = std::stoi(stdHW_Addr.substr(i * 3, 2), 0, 16);
    }

    if (_DBlevel > 10)
        OF::Functions::printHexAndChar(this->_portStruct->hw_addr, OFP_ETH_ALEN, "EtherAddr: ");

    std::memcpy(this->_portStruct->name, (this->_VLC ? "VLC\0" : ifname.c_str()), (this->_VLC ? 4 : (ifname.length() <= OFP_MAX_PORT_NAME_LEN ? ifname.length() : OFP_MAX_PORT_NAME_LEN)));

#if CLICK_BYTE_ORDER == CLICK_BIG_ENDIAN

    this->_portStruct->state = OFPPS_LIVE;
    this->_portStruct->properties[0].type = OFPPDPT_ETHERNET;
    this->_portStruct->properties[0].curr = ((speed.compare("10", 2)) ? ((duplex.compare("half", 4)) ? OFPPF_10MB_HD : OFPPF_10MB_FD) : (speed.compare("100", 3)) ? ((duplex.compare("half", 4)) ? OFPPF_100MB_HD : OFPPF_100MB_FD) : (speed.compare("1000", 4)) ? ((duplex.compare("half", 4)) ? OFPPF_1GB_HD : OFPPF_1GB_FD) : OFPPF_OTHER) | OFPPF_COPPER;
    this->_portStruct->properties[0].advertised = 0;
    this->_portStruct->properties[0].supported = 0;
    this->_portStruct->properties[0].peer = 0;
    this->_portStruct->properties[0].curr_speed = speed.empty() ? 0 : std::stoi(speed.c_str(), 0, 10) * 1000;
    this->_portStruct->properties[0].max_speed = speed.empty() ? 0 : std::stoi(speed.c_str(), 0, 10) * 1000;
    this->_portStruct->length = sizeof(struct OF::Structs::my_ofp_port);
    this->_portStruct->properties[0].length = sizeof(struct ofp_port_desc_prop_ethernet);
    this->_portStruct->config = 0;

#elif CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN

    this->_portStruct->state = __builtin_bswap32(OFPPS_LIVE);
    this->_portStruct->properties[0].type = __builtin_bswap16(OFPPDPT_ETHERNET);
    this->_portStruct->properties[0].curr = __builtin_bswap32(((speed.compare("10", 2)) ? ((duplex.compare("half", 4)) ? OFPPF_10MB_HD : OFPPF_10MB_FD) : (speed.compare("100", 3)) ? ((duplex.compare("half", 4)) ? OFPPF_100MB_HD : OFPPF_100MB_FD) : (speed.compare("1000", 4)) ? ((duplex.compare("half", 4)) ? OFPPF_1GB_HD : OFPPF_1GB_FD) : OFPPF_OTHER) | OFPPF_COPPER);
    this->_portStruct->properties[0].advertised = __builtin_bswap32(0);
    this->_portStruct->properties[0].supported = __builtin_bswap32(0);
    this->_portStruct->properties[0].peer = __builtin_bswap32(0);
    this->_portStruct->properties[0].curr_speed = __builtin_bswap32(speed.empty() ? 0 : std::stoi(speed.c_str(), 0, 10) * 1000);
    this->_portStruct->properties[0].max_speed = __builtin_bswap32(speed.empty() ? 0 : std::stoi(speed.c_str(), 0, 10) * 1000);
    this->_portStruct->length = __builtin_bswap16(sizeof(struct OF::Structs::my_ofp_port));
    this->_portStruct->properties[0].length = __builtin_bswap16(sizeof(struct ofp_port_desc_prop_ethernet));
    this->_portStruct->config = __builtin_bswap32(0);

#else
#error Problem with detecting Endian Format of your System.
#endif

    delete etherAddr;
    etherAddr = nullptr;

    return EXIT_SUCCESS;
}

void OFInterfaceWrapper::push(int inPort, Packet *p)
{
    if (this->_DBlevel > 10)
    {
        std::string check = "OFInterfaceWrapper::push(int inPort, Packet *p) on Port : ";
        check.push_back(inPort + '0');
        OF::Functions::printHexAndChar((uint8_t *)p->data(), p->length(), check);
    }

    switch (inPort)
    {
    case 0: // Packet from Pipeline
    {
        struct OF::Structs::click_port_comm_header *header = (struct OF::Structs::click_port_comm_header *)p->data();
        if (header->identifier == OF::Structs::CPCI_FROM_PIPELINE)
        {
            switch (header->type)
            {
            case OF::Structs::CPCT_PACKET_OUT:
            {
                if (this->_DBlevel > 10)
                    OF::Functions::printHexAndChar((uint8_t *)p->data(), p->length(), "CPCT_PACKET_OUT: ");

                WritablePacket *response = Packet::make(header->data, header->length);

                if (this->_DBlevel > 10)
                    OF::Functions::printHexAndChar((uint8_t *)response->data(), response->length(), "CPCT_PACKET_OUT 2: ");

                this->sendPacketToInterface(response);

                break;
            }
            case OF::Structs::CPCT_PORT_DESC:
            {
                struct OF::Structs::click_port_comm_header *responseHeader = (struct OF::Structs::click_port_comm_header *)std::calloc(1, sizeof(struct OF::Structs::click_port_comm_header) + sizeof(struct OF::Structs::my_ofp_port));

                std::memcpy(responseHeader->data, this->_portStruct, sizeof(struct OF::Structs::my_ofp_port));
                responseHeader->identifier = OF::Structs::CPCI_TO_PIPELINE;
                responseHeader->type = OF::Structs::CPCT_PORT_DESC;
                responseHeader->length = sizeof(struct OF::Structs::my_ofp_port);

                WritablePacket *response = Packet::make(responseHeader, sizeof(struct OF::Structs::click_port_comm_header) + sizeof(struct OF::Structs::my_ofp_port));

                output(0).push(response);

                std::free(responseHeader);
                responseHeader = nullptr;

                if (response != nullptr)
                    response->kill();

                break;
            }
            default:
                break;
            }
        }

        header = nullptr;
        break;
    }
    case 1: // Packet from Interface Device
    {
        OF::Structs::click_port_comm_header *header = (OF::Structs::click_port_comm_header *)p->data();

        if (header->identifier == OF::Structs::CPCI_PING)
        {
            OF::Structs::click_ping_data *pingData = (OF::Structs::click_ping_data *)header->data;
            switch (header->type)
            {
            case OF::Structs::CPCT_PING_REQUEST:
            {

                this->sendPing(OF::Structs::CPCT_PING_RESPONSE, pingData->seqNummer);
                break;
            }
            case OF::Structs::CPCT_PING_RESPONSE:
            {
                this->_responseSeqNummer = pingData->seqNummer;
                this->_pingTrys = 0;

#if CLICK_BYTE_ORDER == CLICK_BIG_ENDIAN

                if (this->_portStruct->state != OFPPS_LIVE)
                {

                    this->_portStruct->state = OFPPS_LIVE;

#elif CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN

                if (this->_portStruct->state != __builtin_bswap32(OFPPS_LIVE))
                {
                    this->_portStruct->state = __builtin_bswap32(OFPPS_LIVE);

#else
#error Problem with detecting Endian Format of your System.
#endif

                    this->sendPortStatusMessage();
                }

                break;
            }
            default:
            {
                this->sendPacketToPipeline(p);

                break;
            }
            }
        }
        else
        {
            this->sendPacketToPipeline(p);
        }

        break;
    }
    default:
    {
        std::cerr << "ERROR: Wrong inPort on OFInterfaceWrapper::push():" << std::endl;
        p->kill();
        break;
    }
    }
}

bool OFInterfaceWrapper::sendPacketToInterface(Packet *p)
{
    this->_portCounter->TransmittedPackets++;
    this->_portCounter->TransmittedBytes += p->length();

    output(1).push(p);
    p->kill();

    return true;
}

bool OFInterfaceWrapper::sendPacketToPipeline(Packet *p)
{
    this->_portCounter->ReceivedPackets++;
    this->_portCounter->ReceivedBytes += p->length();

    switch (p->packet_type_anno())
    {
    case PACKET_BROADCAST:
    case PACKET_MULTICAST:
    case PACKET_OTHERHOST:
    {
        if (this->_DBlevel > 10)
            OF::Functions::printHexAndChar((uint8_t *)p->data(), p->length(), "WARNING: Broad-, Multicast or Otherhost: ");

        struct OF::Structs::click_port_comm_header *header = (OF::Structs::click_port_comm_header *)std::calloc(1, sizeof(struct OF::Structs::click_port_comm_header) + p->length());
        header->identifier = OF::Structs::CPCI_TO_PIPELINE;
        header->type = OF::Structs::CPCT_PACKET_IN;
        header->length = p->length();
        std::memcpy(header->data, p->data(), p->length());

        WritablePacket *response = Packet::make(header, sizeof(OF::Structs::click_port_comm_header) + p->length());
        response->set_packet_type_anno(p->packet_type_anno());

        p->kill();

        output(0).push(response);

        std::free(header);
        header = nullptr;

        if (response != nullptr)
            response->kill();

        break;
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

    return false;
}

void OFInterfaceWrapper::run_timer(Timer *)
{
    if (this->_responseSeqNummer == this->_seqNummer)
    {
        this->_seqNummer++;
        if (this->_interval != 0)
            this->_timer.schedule_after_msec(this->_interval);
    }

#if CLICK_BYTE_ORDER == CLICK_BIG_ENDIAN

    else if (this->_pingTrys >= 3 && (this->_portStruct->state != OFPPS_LINK_DOWN))
    {
        this->_portStruct->state = OFPPS_LINK_DOWN;

#elif CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN

    else if (this->_pingTrys >= 3 && (this->_portStruct->state != __builtin_bswap32(OFPPS_LINK_DOWN)))
    {
        this->_portStruct->state = __builtin_bswap32(OFPPS_LINK_DOWN);

#else
#error Problem with detecting Endian Format of your System.
#endif

        this->sendPortStatusMessage();
        if (this->_interval != 0)
            this->_timer.schedule_after_msec(this->_interval / 2);
    }
    else
    {
        this->_pingTrys++;
        if (this->_interval != 0)
            this->_timer.schedule_after_msec(this->_interval / 2);
    }

    this->sendPing(OF::Structs::CPCT_PING_REQUEST, this->_seqNummer);
}

void OFInterfaceWrapper::sendPing(uint8_t type, uint64_t seqNummer)
{
    uint32_t payloadLength = sizeof(OF::Structs::click_ping_data);
    uint32_t messageLength = sizeof(OF::Structs::click_port_comm_header) + payloadLength;

    OF::Structs::click_port_comm_header *pingRequest = (OF::Structs::click_port_comm_header *)std::calloc(1, messageLength);
    OF::Structs::click_ping_data *pingPayload = (OF::Structs::click_ping_data *)std::malloc(payloadLength);

    pingRequest->identifier = OF::Structs::CPCI_PING;
    pingRequest->length = payloadLength;
    pingRequest->type = type;

    pingPayload->seqNummer = seqNummer;

    std::memcpy(pingRequest->data, pingPayload, payloadLength);

    std::free(pingPayload);
    pingPayload = nullptr;

    WritablePacket *ping = Packet::make(pingRequest, messageLength);

    std::free(pingRequest);
    pingRequest = nullptr;

    output(1).push(ping);
}

void OFInterfaceWrapper::sendPortStatusMessage()
{
    uint16_t payloadLength = sizeof(OF::Structs::ofp_port_status);
    uint16_t messageLength = sizeof(OF::Structs::click_port_comm_header) + payloadLength;

    OF::Structs::ofp_port_status *payload = (OF::Structs::ofp_port_status *)std::calloc(1, payloadLength);

#if CLICK_BYTE_ORDER == CLICK_BIG_ENDIAN

    payload->header.length = payloadLength;

#elif CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN

    payload->header.length = __builtin_bswap16(payloadLength);

#else
#error Problem with detecting Endian Format of your System.
#endif

    payload->header.type = OFPT_PORT_STATUS;
    payload->reason = OFPPR_MODIFY;

    std::memcpy(&payload->desc, this->_portStruct, sizeof(OF::Structs::my_ofp_port));

    OF::Structs::click_port_comm_header *message = (OF::Structs::click_port_comm_header *)std::calloc(1, messageLength);

    message->identifier = OF::Structs::CPCI_TO_PIPELINE;
    message->length = payloadLength;
    message->type = OF::Structs::CPCT_PORT_STATUS;

    std::memcpy(message->data, payload, payloadLength);

    std::free(payload);
    payload = nullptr;

    WritablePacket *portDown = Packet::make(message, messageLength);

    std::free(message);
    message = nullptr;

    output(0).push(portDown);
}

CLICK_ENDDECLS

EXPORT_ELEMENT(OFInterfaceWrapper)