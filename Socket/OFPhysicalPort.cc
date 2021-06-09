#include "OFPhysicalPort.hh"


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
                               uint32_t max_speed)
{

    std::cout << "OFPhysicalPort Constructor" << std::endl;

    //     this->_datalink = -1;
    //     this->_promisc = 0;
    //     this->_snaplen = default_snaplen;

    //     this->_promisc = false;
    //     this->_outbound = false;
    //     this->_sniffer = true;
    //     this->_timestamp = true;
    //     this->_protocol = 0;
    //     this->_headroom = myPacket::default_headroom;
    //     this->_headroom += (4 - (this->_headroom + 2) % 4) % 4;
    //     this->_force_ip = false;
    //     this->_burst = 1;

    //     this->_method = method_default;

    // #if FROMDEVICE_ALLOW_LINUX || FROMDEVICE_ALLOW_PCAP || FROMDEVICE_ALLOW_NETMAP
    //     this->_fd = -1;
    // #endif

    this->_fd = 1;

    this->_ifname = name;
    // this->portStruct = (OF::Structs::my_ofp_port *)std::calloc(1, sizeof(struct OF::Structs::my_ofp_port));
    // this->portCounter = (OF::Structs::my_ofp_port_counter *)std::calloc(1, sizeof(struct OF::Structs::my_ofp_port_counter));

    this->portStruct.port_no = portNo;
    std::memcpy(this->portStruct.hw_addr, hardAddr, sizeof(OFP_ETH_ALEN));
    name.resize(OFP_MAX_PORT_NAME_LEN - 1);
    std::memcpy(this->portStruct.name, name.c_str(), OFP_MAX_PORT_NAME_LEN);
    this->portStruct.config = config;
    this->portStruct.state = state;
    this->portStruct.properties[0].type = type;
    this->portStruct.properties[0].curr = curr;
    this->portStruct.properties[0].advertised = advertised;
    this->portStruct.properties[0].supported = supported;
    this->portStruct.properties[0].peer = peer;
    this->portStruct.properties[0].curr_speed = curr_speed;
    this->portStruct.properties[0].max_speed = max_speed;
    this->portStruct.length = sizeof(struct OF::Structs::my_ofp_port);
    this->portStruct.properties[0].length = sizeof(struct ofp_port_desc_prop_ethernet);

#if __BYTE_ORDER == __LITTLE_ENDIAN
    this->portStruct.port_no = __builtin_bswap32(this->portStruct.port_no);
    this->portStruct.config = __builtin_bswap32(this->portStruct.config);
    this->portStruct.state = __builtin_bswap32(this->portStruct.state);
    this->portStruct.properties[0].type = __builtin_bswap16(this->portStruct.properties[0].type);
    this->portStruct.properties[0].curr = __builtin_bswap32(this->portStruct.properties[0].curr);
    this->portStruct.properties[0].advertised = __builtin_bswap32(this->portStruct.properties[0].advertised);
    this->portStruct.properties[0].supported = __builtin_bswap32(this->portStruct.properties[0].supported);
    this->portStruct.properties[0].peer = __builtin_bswap32(this->portStruct.properties[0].peer);
    this->portStruct.properties[0].curr_speed = __builtin_bswap32(this->portStruct.properties[0].curr_speed);
    this->portStruct.properties[0].max_speed = __builtin_bswap32(this->portStruct.properties[0].max_speed);
    this->portStruct.length = __builtin_bswap16(this->portStruct.length);
    this->portStruct.properties[0].length = __builtin_bswap16(this->portStruct.properties[0].length);
#endif

    // if ((this->_method == this->method_default || this->_method == this->method_linux) && this->getPortState() == OFPPS_LIVE && this->getPortNo() != OFPP_LOCAL)
    // {
    // this->_fd = this->open_packet_socket(this->_ifname);
    // if (this->_fd < 0)
    // {
    //     std::cout << this->_fd << " < 0: _ifname: " << this->_ifname << std::endl;
    //     exit(-1);
    // }

    // int promisc_ok = this->set_promiscuous(this->_fd, this->_ifname, this->_promisc);
    // if (promisc_ok < 0)
    // {
    //     if (this->_promisc)
    //         this->_was_promisc = -1;
    // }
    // else
    //     this->_was_promisc = promisc_ok;

    // this->_datalink = FAKE_DLT_EN10MB;
    // this->_method = this->method_linux;
    // }
}

OFPhysicalPort::~OFPhysicalPort()
{
    // if (this->portStruct != nullptr && this->portStruct != NULL)
    //     std::free(this->portStruct);
    // if (this->portCounter != nullptr && this->portCounter != NULL)
    //     std::free(this->portCounter);
    // this->portStruct = nullptr;
    // this->portCounter = nullptr;

    // if (this->_fd >= 0 && this->_method == this->method_linux)
    // {
    //     if (this->_was_promisc >= 0)
    //         this->set_promiscuous(this->_fd, this->_ifname, this->_was_promisc);
    //     close(this->_fd);
    // }

    // this->_fd = -1;
}

void OFPhysicalPort::cleanup()
{
    this->~OFPhysicalPort();
}

uint32_t OFPhysicalPort::getPortState()
{
#if __BYTE_ORDER == __BIG_ENDIAN
    return this->portStruct.state;
#elif __BYTE_ORDER == __LITTLE_ENDIAN
    return __builtin_bswap32(this->portStruct.state);
#endif
}

struct OF::Structs::my_ofp_port *OFPhysicalPort::getPortStruct()
{
    return &this->portStruct;
}

uint32_t OFPhysicalPort::getPortStructSize()
{
    return sizeof(struct OF::Structs::my_ofp_port);
}

void OFPhysicalPort::setStats(struct OF::Structs::my_ofp_port_counter *in)
{
    this->portCounter.Collisions += in->Collisions;
    this->portCounter.Duration_ns += in->Duration_ns;
    this->portCounter.Duration_s += in->Duration_s;
    this->portCounter.ReceiveCRCErrors += in->ReceiveCRCErrors;
    this->portCounter.ReceivedBytes += in->ReceivedBytes;
    this->portCounter.ReceivedPackets += in->ReceivedPackets;
    this->portCounter.ReceiveDrops += in->ReceiveDrops;
    this->portCounter.ReceiveErrors += in->ReceiveErrors;
    this->portCounter.ReceiveFrameAlignmentErrors += in->ReceiveFrameAlignmentErrors;
    this->portCounter.ReceiveOverrunErrors += in->ReceiveOverrunErrors;
    this->portCounter.TransmitDrops += in->TransmitDrops;
    this->portCounter.TransmitErrors += in->TransmitErrors;
    this->portCounter.TransmittedBytes += in->TransmittedBytes;
    this->portCounter.TransmittedPackets += in->TransmittedPackets;

    std::free(in);
    in = nullptr;
}

int OFPhysicalPort::open_packet_socket(std::string ifname)
{
    // int fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    // // get interface index
    // struct ifreq ifr;
    // memset(&ifr, 0, sizeof(ifr));
    // strncpy(ifr.ifr_name, ifname.c_str(), sizeof(ifr.ifr_name));
    // int res = ioctl(fd, SIOCGIFINDEX, &ifr);
    // if (res != 0)
    // {
    //     if (fd > 0)
    //         close(fd);
    //     return EXIT_FAILURE;
    // }
    // int ifindex = ifr.ifr_ifindex;

    // // bind to the specified interface.  from packet man page, only
    // // sll_protocol and sll_ifindex fields are used; also have to set
    // // sll_family
    // sockaddr_ll sa;
    // memset(&sa, 0, sizeof(sa));
    // sa.sll_family = AF_PACKET;
    // sa.sll_protocol = htons(ETH_P_ALL);
    // sa.sll_ifindex = ifindex;
    // res = bind(fd, (struct sockaddr *)&sa, sizeof(sa));
    // if (res != 0)
    // {
    //     close(fd);
    //     return EXIT_FAILURE;
    // }

    // // nonblocking I/O on the packet socket so we can poll
    // fcntl(fd, F_SETFL, O_NONBLOCK);

    // return fd;

    return NULL;
}

int OFPhysicalPort::set_promiscuous(int fd, std::string ifname, bool promisc)
{
    // // get interface flags
    // struct ifreq ifr;
    // memset(&ifr, 0, sizeof(ifr));
    // strncpy(ifr.ifr_name, ifname.c_str(), sizeof(ifr.ifr_name));
    // if (ioctl(fd, SIOCGIFFLAGS, &ifr) != 0)
    //     return -2;
    // int was_promisc = (ifr.ifr_flags & IFF_PROMISC ? 1 : 0);

    // // set or reset promiscuous flag
    // if (ioctl(fd, SIOCGIFINDEX, &ifr) != 0)
    //     return -2;
    // struct packet_mreq mr;
    // memset(&mr, 0, sizeof(mr));
    // mr.mr_ifindex = ifr.ifr_ifindex;
    // mr.mr_type = (promisc ? PACKET_MR_PROMISC : PACKET_MR_ALLMULTI);
    // if (setsockopt(fd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) < 0)
    //     return -3;

    // return was_promisc;

    return NULL;
}

myPacket *OFPhysicalPort::selected()
{
    // while (1)
    // {
    //     myPacket *p = new myPacket();
    //     int nlinux = 0;
    //     // while (_method == method_linux && nlinux < _burst)
    //     // {
    //     struct sockaddr_ll sa;
    //     socklen_t fromlen = sizeof(sa);
    //     p->make(_headroom, 0, _snaplen, 0);
    //     int len = recvfrom(this->_fd, p->data(), p->length(), MSG_TRUNC, (sockaddr *)&sa, &fromlen);

    //     std::cout << "legth of Physical: " << std::dec << len << std::endl;

    //     if (len > 0 && (sa.sll_pkttype != PACKET_OUTGOING || _outbound) && (_protocol == 0 || _protocol == sa.sll_protocol))
    //     {
    //         if (len > _snaplen)
    //         {
    //             assert(p->length() == _snaplen);
    //             // SET_EXTRA_LENGTH_ANNO(p, len - _snaplen);
    //         }
    //         else
    //             //     p->take(_snaplen - len);
    //             // p->set_packet_type_anno((Packet::PacketType)sa.sll_pkttype);
    //             // p->timestamp_anno().set_timeval_ioctl(_fd, SIOCGSTAMP);
    //             // p->set_mac_header(p->data());
    //             ++nlinux;
    //         this->portCounter->ReceivedPackets++;
    //         this->portCounter->ReceivedBytes += p->length();

    //         OF::Functions::printHex(p->data(), p->length(), "Data From Port: ");
    //         // output(0).push(p);

    //         return p;
    //     }
    //     else
    //     {
    //         p->kill();

    //         if (len <= 0 && errno != EAGAIN)
    //         {
    //             // printf("FromDevice(%s): recvfrom: %s\n", _ifname.c_str(), strerror(errno));
    //         }

    //         // return NULL;
    //     }
    //     p->kill();
    //     delete (p);
    // }
    // }
    return NULL;
}

int OFPhysicalPort::send_packet(myPacket *p)
{
    // int r = 0;
    // errno = 0;
    // if (p != NULL)
    // {
    //     if (this->getPortState() == OFPPS_LIVE && this->getPortNo() != OFPP_LOCAL)
    //     {

    //         // if (_method == method_linux)
    //         r = send(_fd, p->data(), p->length(), 0);
    //         // OF::Functions::printHex((uint8_t*) p->data(), p->length(), "In Port to Send: ");

    //         if (r >= 0)
    //             return 0;
    //         else
    //             return errno ? -errno : -EINVAL;
    //     }
    //     p->kill();
    // }
    return 0;
}

uint32_t OFPhysicalPort::getPortNo()
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    return __builtin_bswap32(this->portStruct.port_no);
#elif __BYTE_ORDER == __BIG_ENDIAN
    return this->portStruct.port_no;
#endif
}

const int OFPhysicalPort::fd()
{
    if (this->getPortState() == OFPPS_LIVE)
    {
        return _fd;
    }
    else
    {
        return 0;
    }
}