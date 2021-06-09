#include "OFOXMField.hh"

CLICK_DECLS

OFOXMField::OFOXMField(uint8_t *_oxmData, uint8_t _oxmDataLength)
{
    struct OF::Structs::my_ofp_oxm_header *oxmHeader = (struct OF::Structs::my_ofp_oxm_header *)_oxmData;

    this->oxmData = (uint8_t *)std::malloc(oxmHeader->oxm_length);
    std::memcpy(this->oxmData, oxmHeader->oxm_value, oxmHeader->oxm_length);

    this->oxmDataLength = oxmHeader->oxm_length;
    this->oxmType = oxmHeader->oxm_field_WM;
    this->oxmType = this->oxmType >> 1;

#if CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
    if (this->oxmType == OFPXMT_OFB_IN_PORT)
    {
        uint32_t *PortNo = (uint32_t *)this->oxmData;
        *PortNo = __builtin_bswap32(*PortNo);
    }
#endif
}

OFOXMField::OFOXMField(OFOXMField &in)
{
    this->oxmData = (uint8_t *)std::malloc(in.getOXMDataLength());
    std::memcpy(this->oxmData, in.getOXMData(), in.getOXMDataLength());
    this->oxmDataLength = in.getOXMDataLength();
    this->oxmType = in.getOXMType();
}

OFOXMField::OFOXMField(OFOXMField *in)
{
    this->oxmData = (uint8_t *)std::malloc(in->getOXMDataLength());
    std::memcpy(this->oxmData, in->getOXMData(), in->getOXMDataLength());
    this->oxmDataLength = in->getOXMDataLength();
    this->oxmType = in->getOXMType();
}

OFOXMField::~OFOXMField()
{
    std::free(this->oxmData);
}

bool OFOXMField::checkPacketInOXMField(Packet *p, uint32_t portNo)
{
    switch (this->oxmType)
    {
    // Check Port Number
    case OFPXMT_OFB_IN_PORT:
    {
        if (portNo == *(uint32_t *)this->oxmData)
            return true;
        break;
    }
    // Check Ethernet Address
    case OFPXMT_OFB_ETH_DST:
    {
        const ether_header *etherHeader = (ether_header *)p->data();
        for (size_t i = 0; i < this->oxmDataLength; i++)
        {
            if (etherHeader->ether_dhost[i] != this->oxmData[i])
                return false;
        }
        return true;
        break;
    }
    case OFPXMT_OFB_ETH_SRC:
    {
        const ether_header *etherHeader = (ether_header *)p->data();
        for (size_t i = 0; i < this->oxmDataLength; i++)
        {
            if (etherHeader->ether_shost[i] != this->oxmData[i])
                return false;
        }
        return true;
        break;
    }
    // Check Ethernet Type (e.g. IPv4 == 0x0800)
    case OFPXMT_OFB_ETH_TYPE:
    {
        const ether_header *etherHeader = (ether_header *)p->data();
        if (etherHeader->ether_type == *(uint16_t *)this->oxmData)
            return true;
        break;
    }
    // Check IP Protocol (e.g. UDP == 17)
    case OFPXMT_OFB_IP_PROTO:
    {
        const ether_header *etherHeader = (ether_header *)p->data();
        if (etherHeader->ether_type == 0x0800)
        {
            const click_ip *ipHeader = (click_ip *)(p->data() + sizeof(click_ether));
            if (ipHeader->ip_p == *this->oxmData)
                return true;
        }
        break;
    }
    // Check IP Address
    case OFPXMT_OFB_IPV4_SRC:
    {
        const ether_header *etherHeader = (ether_header *)p->data();
        if (etherHeader->ether_type == 0x0800)
        {
            const click_ip *ipHeader = (click_ip *)(p->data() + sizeof(click_ether));
            if (ipHeader->ip_src.s_addr == *(in_addr_t *)this->oxmData)
                return true;
        }
        break;
    }
    case OFPXMT_OFB_IPV4_DST:
    {
        const ether_header *etherHeader = (ether_header *)p->data();
        if (etherHeader->ether_type == 0x0800)
        {
            const click_ip *ipHeader = (click_ip *)(p->data() + sizeof(click_ether));
            if (ipHeader->ip_dst.s_addr == *(in_addr_t *)this->oxmData)
                return true;
        }
        break;
    }
    // Check UDP Port
    case OFPXMT_OFB_UDP_SRC:
    {
        // // const click_ether *etherHeader = p->ether_header();
        // const ether_header *etherHeader = (ether_header *)p->data();
        // if (etherHeader->ether_type == 0x0800)
        // {
        //     const click_ip *ipHeader = (click_ip *)(p->data() + sizeof(click_ether));
        //     if (ipHeader->ip_p == IP_PROTO_UDP)
        //     {
        //         const click_udp *udpHeader = (click_udp *)(p->data() + sizeof(click_ether) + sizeof(click_ip));
        //         if (udpHeader->uh_sport == *(uint16_t *)this->oxmData)
        //             return true;
        //     }
        //     ipHeader = nullptr;
        // }
        // etherHeader = nullptr;
        // break;
    }
    case OFPXMT_OFB_UDP_DST:
    {
        // // const click_ether *etherHeader = p->ether_header();
        // const ether_header *etherHeader = (ether_header *)p->data();
        // if (etherHeader->ether_type == 0x0800)
        // {
        //     const click_udp *udpHeader = p->udp_header();
        //     if (udpHeader->uh_dport == *(uint16_t *)this->oxmData)
        //         return true;
        // }
        // break;
    }
    // Check TCP Port
    case OFPXMT_OFB_TCP_SRC:
    {
        // // const click_ether *etherHeader = p->ether_header();
        // const ether_header *etherHeader = (ether_header *)p->data();
        // if (etherHeader->ether_type == 0x0800)
        // {
        //     const click_tcp *tcpHeader = p->tcp_header();
        //     if (tcpHeader->th_sport == *(uint16_t *)this->oxmData)
        //         return true;
        // }
        // break;
    }
    case OFPXMT_OFB_TCP_DST:
    {
        // // const click_ether *etherHeader = p->ether_header();
        // const ether_header *etherHeader = (ether_header *)p->data();
        // if (etherHeader->ether_type == 0x0800)
        // {
        //     const click_tcp *tcpHeader = p->tcp_header();
        //     if (tcpHeader->th_dport == *(uint16_t *)this->oxmData)
        //         return true;
        // }
        // break;
    }

    case OFPXMT_OFB_IN_PHY_PORT:
    case OFPXMT_OFB_METADATA:
    case OFPXMT_OFB_VLAN_VID:
    case OFPXMT_OFB_VLAN_PCP:
    case OFPXMT_OFB_IP_DSCP:
    case OFPXMT_OFB_IP_ECN:
    case OFPXMT_OFB_SCTP_SRC:
    case OFPXMT_OFB_SCTP_DST:
    case OFPXMT_OFB_ICMPV4_TYPE:
    case OFPXMT_OFB_ICMPV4_CODE:
    case OFPXMT_OFB_ARP_OP:
    case OFPXMT_OFB_ARP_SPA:
    case OFPXMT_OFB_ARP_TPA:
    case OFPXMT_OFB_ARP_SHA:
    case OFPXMT_OFB_ARP_THA:
    case OFPXMT_OFB_IPV6_SRC:
    case OFPXMT_OFB_IPV6_DST:
    case OFPXMT_OFB_IPV6_FLABEL:
    case OFPXMT_OFB_ICMPV6_TYPE:
    case OFPXMT_OFB_ICMPV6_CODE:
    case OFPXMT_OFB_IPV6_ND_TARGET:
    case OFPXMT_OFB_IPV6_ND_SLL:
    case OFPXMT_OFB_IPV6_ND_TLL:
    case OFPXMT_OFB_MPLS_LABEL:
    case OFPXMT_OFB_MPLS_TC:
    case OFPXMT_OFP_MPLS_BOS:
    case OFPXMT_OFB_PBB_ISID:
    case OFPXMT_OFB_TUNNEL_ID:
    case OFPXMT_OFB_IPV6_EXTHDR:
    case OFPXMT_OFB_PBB_UCA:
    case OFPXMT_OFB_TCP_FLAGS:
    case OFPXMT_OFB_ACTSET_OUTPUT:
    case OFPXMT_OFB_PACKET_TYPE:
    default:
        std::cerr << "ERROR: Fall nicht implementiert, OFOXMField::checkPacketInOXMField(Packet *p, uint32_t portNo) " << std::dec << this->oxmType << std::endl;
        break;
    }

    return false;
}

// Getter and Setter
uint8_t *OFOXMField::getOXMData() { return this->oxmData; }
void OFOXMField::setOXMData(uint8_t *oxmData) { this->oxmData = oxmData; }
uint8_t OFOXMField::getOXMDataLength() { return this->oxmDataLength; }
void OFOXMField::setOXMDataLength(uint8_t oxmDataLength) { this->oxmDataLength = oxmDataLength; }
uint8_t OFOXMField::getOXMType() { return this->oxmType; }
void OFOXMField::setOXMType(uint8_t oxmType) { this->oxmType = oxmType; }

CLICK_ENDDECLS

ELEMENT_PROVIDES(OFOXMField)