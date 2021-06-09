#include "OFPhysicalPort.hh"

CLICK_DECLS

OFPhysicalPort::OFPhysicalPort(OFPhysicalPort *in)
{
    this->portStruct = (struct OF::Structs::my_ofp_port *)std::calloc(1, in->getPortStructSize());
    std::memcpy(this->portStruct, in->getPortStruct(), in->getPortStructSize());

    this->portCounter = (struct OF::Structs::my_ofp_port_counter *)std::calloc(1, in->getPortCounterSize());
    std::memcpy(this->portCounter, in->getPortCounter(), in->getPortCounterSize());

    this->_datalink = -1;
    this->_snaplen = default_snaplen;

    this->_promisc = true;
    this->_protocol = 0;
    this->_headroom = Packet::default_headroom;
    this->_headroom += (4 - (this->_headroom + 2) % 4) % 4;
    this->_force_ip = false;
    this->_burst = 1;
    this->_fd = -1;

    this->_ifname = in->getPortStruct()->name;

    if (this->getPortState() == OFPPS_LIVE && this->getPortNo() != OFPP_LOCAL)
    {
        this->_fd = this->open_packet_socket(this->_ifname, NULL);
        if (this->_fd < 0)
        {
            std::cerr << "ERROR: " << this->_fd << " < 0: _ifname: " << this->_ifname << std::endl;
            exit(-1);
        }

        int promisc_ok = this->set_promiscuous(this->_fd, this->_ifname, this->_promisc);
        if (promisc_ok < 0)
        {
            if (this->_promisc)
                std::cerr << "WARNING: cannot set promiscuous mode" << std::endl;
            this->_was_promisc = -1;
        }
        else
            this->_was_promisc = promisc_ok;

        this->_datalink = FAKE_DLT_EN10MB;
    }
}

OFPhysicalPort::OFPhysicalPort(uint32_t portNo,
                               uint8_t *hardAddr,
                               std::string name,
                               uint32_t config,
                               uint32_t state,
                               uint16_t type,
                               uint32_t curr,
                               uint32_t advertised,
                               uint32_t supported,
                               uint32_t peer,
                               uint32_t curr_speed,
                               uint32_t max_speed,
                               ErrorHandler *errh,
                               OFPortMng *_PortMng)
{

    this->PortMng = _PortMng;

    this->_datalink = -1;
    this->_snaplen = default_snaplen;

    this->_promisc = true;
    this->_protocol = 0;
    this->_headroom = Packet::default_headroom;
    this->_headroom += (4 - (this->_headroom + 2) % 4) % 4;
    this->_force_ip = false;
    this->_burst = 1;
    this->_fd = -1;

    this->_ifname = name;

    this->portStruct = (OF::Structs::my_ofp_port *)std::calloc(1, sizeof(struct OF::Structs::my_ofp_port));
    this->portCounter = (OF::Structs::my_ofp_port_counter *)std::calloc(1, sizeof(struct OF::Structs::my_ofp_port_counter));

    this->portStruct->port_no = portNo;
    std::memcpy(this->portStruct->hw_addr, hardAddr, sizeof(OFP_ETH_ALEN));
    name.resize(OFP_MAX_PORT_NAME_LEN - 1);
    std::memcpy(this->portStruct->name, name.c_str(), OFP_MAX_PORT_NAME_LEN);
    this->portStruct->config = config;
    this->portStruct->state = state;
    this->portStruct->properties[0].type = type;
    this->portStruct->properties[0].curr = curr;
    this->portStruct->properties[0].advertised = advertised;
    this->portStruct->properties[0].supported = supported;
    this->portStruct->properties[0].peer = peer;
    this->portStruct->properties[0].curr_speed = curr_speed;
    this->portStruct->properties[0].max_speed = max_speed;
    this->portStruct->length = sizeof(struct OF::Structs::my_ofp_port);
    this->portStruct->properties[0].length = sizeof(struct ofp_port_desc_prop_ethernet);

#if CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
    this->portStruct->port_no = __builtin_bswap32(this->portStruct->port_no);
    this->portStruct->config = __builtin_bswap32(this->portStruct->config);
    this->portStruct->state = __builtin_bswap32(this->portStruct->state);
    this->portStruct->properties[0].type = __builtin_bswap16(this->portStruct->properties[0].type);
    this->portStruct->properties[0].curr = __builtin_bswap32(this->portStruct->properties[0].curr);
    this->portStruct->properties[0].advertised = __builtin_bswap32(this->portStruct->properties[0].advertised);
    this->portStruct->properties[0].supported = __builtin_bswap32(this->portStruct->properties[0].supported);
    this->portStruct->properties[0].peer = __builtin_bswap32(this->portStruct->properties[0].peer);
    this->portStruct->properties[0].curr_speed = __builtin_bswap32(this->portStruct->properties[0].curr_speed);
    this->portStruct->properties[0].max_speed = __builtin_bswap32(this->portStruct->properties[0].max_speed);
    this->portStruct->length = __builtin_bswap16(this->portStruct->length);
    this->portStruct->properties[0].length = __builtin_bswap16(this->portStruct->properties[0].length);
#endif

    if (this->getPortState() == OFPPS_LIVE && this->getPortNo() != OFPP_LOCAL)
    {
        this->_fd = this->open_packet_socket(this->_ifname, errh);
        if (this->_fd < 0)
        {
            std::cout << this->_fd << " < 0: _ifname: " << this->_ifname << std::endl
                      << errh->e_error << std::endl;
            exit(-1);
        }

        int promisc_ok = this->set_promiscuous(this->_fd, this->_ifname, this->_promisc);
        if (promisc_ok < 0)
        {
            if (this->_promisc)
                errh->warning("cannot set promiscuous mode");
            this->_was_promisc = -1;
        }
        else
            this->_was_promisc = promisc_ok;

        this->_datalink = FAKE_DLT_EN10MB;
    }
}

OFPhysicalPort::~OFPhysicalPort()
{
    this->cleanup();
}

void OFPhysicalPort::cleanup()
{
    std::free(this->portStruct);
    std::free(this->portCounter);
    this->portStruct = nullptr;
    this->portCounter = nullptr;

    if (this->_fd >= 0)
    {
        if (this->_was_promisc >= 0)
            this->set_promiscuous(this->_fd, this->_ifname, this->_was_promisc);
        close(this->_fd);
    }

    this->_fd = -1;

    this->PortMng = nullptr;
}

uint32_t OFPhysicalPort::getPortState()
{
#if CLICK_BYTE_ORDER == CLICK_BIG_ENDIAN
    return this->portStruct->state;
#elif CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
    return __builtin_bswap32(this->portStruct->state);
#endif
}

struct OF::Structs::my_ofp_port *OFPhysicalPort::getPortStruct() { return this->portStruct; }
uint32_t OFPhysicalPort::getPortStructSize() { return sizeof(struct OF::Structs::my_ofp_port); }

struct OF::Structs::my_ofp_port_counter *OFPhysicalPort::getPortCounter() { return this->portCounter; }
uint32_t OFPhysicalPort::getPortCounterSize() { return sizeof(struct OF::Structs::my_ofp_port_counter); }

void OFPhysicalPort::setStats(struct OF::Structs::my_ofp_port_counter *in)
{
    this->portCounter->Collisions += in->Collisions;
    this->portCounter->Duration_ns += in->Duration_ns;
    this->portCounter->Duration_s += in->Duration_s;
    this->portCounter->ReceiveCRCErrors += in->ReceiveCRCErrors;
    this->portCounter->ReceivedBytes += in->ReceivedBytes;
    this->portCounter->ReceivedPackets += in->ReceivedPackets;
    this->portCounter->ReceiveDrops += in->ReceiveDrops;
    this->portCounter->ReceiveErrors += in->ReceiveErrors;
    this->portCounter->ReceiveFrameAlignmentErrors += in->ReceiveFrameAlignmentErrors;
    this->portCounter->ReceiveOverrunErrors += in->ReceiveOverrunErrors;
    this->portCounter->TransmitDrops += in->TransmitDrops;
    this->portCounter->TransmitErrors += in->TransmitErrors;
    this->portCounter->TransmittedBytes += in->TransmittedBytes;
    this->portCounter->TransmittedPackets += in->TransmittedPackets;

    std::free(in);
    in = nullptr;
}

int OFPhysicalPort::open_packet_socket(std::string ifname, ErrorHandler *)
{
    int fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (fd == -1)
    {
        std::cerr << "ERROR: " << ifname << ": socket: " << strerror(errno) << std::endl;
        return -1;
    }
    // get interface index
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifname.c_str(), sizeof(ifr.ifr_name));
    int res = ioctl(fd, SIOCGIFINDEX, &ifr);
    if (res != 0)
    {
        close(fd);
        std::cerr << "ERROR: " << ifname << ": SIOCGIFINDEX: " << strerror(errno) << std::endl;
        return -1;
    }
    int ifindex = ifr.ifr_ifindex;

    // bind to the specified interface.  from packet man page, only
    // sll_protocol and sll_ifindex fields are used; also have to set
    // sll_family
    sockaddr_ll sa;
    memset(&sa, 0, sizeof(sa));
    sa.sll_family = AF_PACKET;
    sa.sll_protocol = htons(ETH_P_ALL);
    sa.sll_ifindex = ifindex;
    res = bind(fd, (struct sockaddr *)&sa, sizeof(sa));
    if (res != 0)
    {
        close(fd);
        std::cerr << "ERROR: " << ifname << ": bind: " << strerror(errno) << std::endl;
        return -1;
    }

    // nonblocking I/O on the packet socket so we can poll
    fcntl(fd, F_SETFL, O_NONBLOCK);

    return fd;
}

int OFPhysicalPort::set_promiscuous(int fd, std::string ifname, bool promisc)
{
    // get interface flags
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifname.c_str(), sizeof(ifr.ifr_name));
    if (ioctl(fd, SIOCGIFFLAGS, &ifr) != 0)
        return -2;
    int was_promisc = (ifr.ifr_flags & IFF_PROMISC ? 1 : 0);

    // set or reset promiscuous flag
    if (ioctl(fd, SIOCGIFINDEX, &ifr) != 0)
        return -2;
    struct packet_mreq mr;
    memset(&mr, 0, sizeof(mr));
    mr.mr_ifindex = ifr.ifr_ifindex;
    mr.mr_type = (promisc ? PACKET_MR_PROMISC : PACKET_MR_ALLMULTI);
    if (setsockopt(fd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) < 0)
        return -3;

    return was_promisc;
}

Packet *OFPhysicalPort::selected(int, int)
{
    struct sockaddr_ll sa;
    socklen_t fromlen = sizeof(sa);
    WritablePacket *p = Packet::make(_headroom, 0, _snaplen, 0);
    int len = recvfrom(_fd, p->data(), p->length(), MSG_TRUNC, (sockaddr *)&sa, &fromlen);

    if (len > 0)
    {
        if (len > _snaplen)
        {
            assert(p->length() == (uint32_t)_snaplen);
            SET_EXTRA_LENGTH_ANNO(p, len - _snaplen);
        }
        else
            p->take(_snaplen - len);
        p->set_packet_type_anno((Packet::PacketType)sa.sll_pkttype);
        p->timestamp_anno().set_timeval_ioctl(_fd, SIOCGSTAMP);
        p->set_mac_header(p->data());

        switch (sa.sll_pkttype)
        {
        case PACKET_BROADCAST:
        case PACKET_MULTICAST:
        {
            this->portCounter->ReceivedPackets++;
            this->portCounter->ReceivedBytes += p->length();
            this->PortMng->broadAndMulticast(p, this);
            if (p != NULL || p != nullptr)
                p->kill();
            p = nullptr;
            break;
        }
        case PACKET_OTHERHOST:
        {
            if (_protocol == 0 || _protocol == sa.sll_protocol)
            {

                this->portCounter->ReceivedPackets++;
                this->portCounter->ReceivedBytes += p->length();
            }
            break;
        }
        case PACKET_HOST:
        case PACKET_OUTGOING:
        {
            p->kill();
            p = nullptr;
            break;
        }
        default:
        {
            p->kill();
            p = nullptr;
            this->portCounter->ReceiveErrors++;
            break;
        }
        }
    }
    else
    {
        p->kill();
        p = nullptr;
        if (len <= 0 && errno != EAGAIN)
            click_chatter("FromDevice(%s): recvfrom: %s", _ifname.c_str(), strerror(errno));

        this->portCounter->ReceiveErrors++;
    }
    return p;
}

int OFPhysicalPort::send_packet(Packet *p)
{
    ssize_t r = 0;
    errno = 0;
    if (p != NULL && p != nullptr)
    {
        if (this->getPortState() == OFPPS_LIVE && this->getPortNo() != OFPP_LOCAL)
        {
            r = send(_fd, p->data(), p->length(), 0);

            if (r >= 0)
            {
                this->portCounter->TransmittedPackets++;
                this->portCounter->TransmittedBytes += p->length();
                if (r != (ssize_t)p->length())
                    std::cerr << "ERROR: in send_packet out of Port\n\t" << r << "Bytes gesendet von " << p->length() << "Bytes" << std::endl;
                p->kill();
                p = nullptr;
                return 0;
            }
            else
            {
                this->portCounter->TransmitErrors++;
                std::cerr << "ERROR: Packet konnte nicht gesendet werden\tTransmitErrors of " << std::dec << this->_ifname << ": " << this->portCounter->TransmitErrors << std::endl;
                p->kill();
                p = nullptr;
                return errno ? -errno : -EINVAL;
            }
        }
        else
        {
            p->kill();
            p = nullptr;
        }
    }
    return 0;
}
CLICK_ENDDECLS
ELEMENT_PROVIDES(OFPhysicalPort)