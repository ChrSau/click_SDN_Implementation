#include "OFPortDesc.hh"

CLICK_DECLS

OFPortDesc::OFPortDesc()
{
    OFPortDesc(0x00000000,
               new uint8_t[OFP_ETH_ALEN]{0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
               "Default", 0x00000000,
               OFPPS_LIVE,
               OFPPDPT_ETHERNET,
               OFPPF_100MB_FD | OFPPF_COPPER,
               0x00000000,
               0x00000000,
               0x00000000,
               0x19000,
               0x00000000);
}

OFPortDesc::OFPortDesc(uint32_t port_no = 0x00000000,
                       uint8_t *hardAddr = new uint8_t[OFP_ETH_ALEN]{0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
                       std::string name = "Default",
                       uint32_t config = 0x00000000,
                       uint32_t state = OFPPS_LIVE,
                       uint16_t type = OFPPDPT_ETHERNET,
                       uint32_t curr = OFPPF_100MB_FD | OFPPF_COPPER,
                       uint32_t advertised = 0x00000000,
                       uint32_t supported = 0x00000000,
                       uint32_t peer = 0x00000000,
                       uint32_t curr_speed = 0x19000,
                       uint32_t max_speed = 0x00000000)
{

    this->portData = (ofp_port *)calloc(1, sizeof(struct ofp_port) + sizeof(struct ofp_port_desc_prop_ethernet));

    portData->port_no = port_no;
    portData->length = sizeof(struct ofp_port) + sizeof(struct ofp_port_desc_prop_ethernet);

    portData->config = config;

    memcpy(portData->hw_addr, hardAddr, OFP_ETH_ALEN);

    name.resize(OFP_MAX_PORT_NAME_LEN);
    memcpy(portData->name, name.c_str(), OFP_MAX_PORT_NAME_LEN);

    portData->state = state;

    struct ofp_port_desc_prop_ethernet *port_desc_header = (ofp_port_desc_prop_ethernet *)calloc(1, sizeof(struct ofp_port_desc_prop_ethernet));
    port_desc_header->length = sizeof(struct ofp_port_desc_prop_ethernet);
    port_desc_header->type = type;
    port_desc_header->curr = curr;
    port_desc_header->curr_speed = curr_speed;
    port_desc_header->advertised = advertised;
    port_desc_header->supported = supported;
    port_desc_header->peer = peer;
    port_desc_header->max_speed = max_speed;

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    port_desc_header->length = __builtin_bswap16(port_desc_header->length);
    port_desc_header->type = __builtin_bswap16(port_desc_header->type);
    port_desc_header->curr = __builtin_bswap32(port_desc_header->curr);
    port_desc_header->advertised = __builtin_bswap32(port_desc_header->advertised);
    port_desc_header->supported = __builtin_bswap32(port_desc_header->supported);
    port_desc_header->peer = __builtin_bswap32(port_desc_header->peer);
    port_desc_header->max_speed = __builtin_bswap32(port_desc_header->max_speed);
    port_desc_header->curr_speed = __builtin_bswap32(port_desc_header->curr_speed);

    portData->port_no = __builtin_bswap32(portData->port_no);
    portData->length = __builtin_bswap16(portData->length);
    portData->config = __builtin_bswap32(portData->config);
    portData->state = __builtin_bswap32(portData->state);
#endif

    memcpy(portData->properties, port_desc_header, sizeof(struct ofp_port_desc_prop_ethernet));

}

OFPortDesc::~OFPortDesc()
{
    delete this->portData;
}

// Getter
struct ofp_port *OFPortDesc::getPortData()
{
    return this->portData;
}

// Setter
bool OFPortDesc::set_port_no(uint32_t in)
{
    this->portData->port_no = in;
    return true;
}
bool OFPortDesc::set_hardAddr(uint8_t *in)
{
    if (in != nullptr)
    {
        memcpy(this->portData->hw_addr, in, OFP_ETH_ALEN);

        for (size_t i = 0; i < OFP_ETH_ALEN; i++)
        {
            if (this->portData->hw_addr[i] != in[i])
                return false;
        }

        return true;
    }
    else
    {
        return false;
    }
}
bool OFPortDesc::set_name(std::string in)
{
    if (in.size() != 0)
    {
        in.resize(OFP_MAX_PORT_NAME_LEN);
        in[OFP_MAX_PORT_NAME_LEN - 1] = '\0';
        memcpy(this->portData->name, in.c_str(), OFP_MAX_PORT_NAME_LEN);
        return true;
    }
    else
    {
        return false;
    }
}
bool OFPortDesc::set_name(char *in)
{
    if (in != nullptr)
    {
        size_t i;
        for (i = 0; (i < OFP_MAX_PORT_NAME_LEN) && (in[i] != '\0'); i++)
        {
        }

        if (i == OFP_MAX_PORT_NAME_LEN - 1)
        {
            in[OFP_MAX_PORT_NAME_LEN - 1] = '\0';
        }
        memcpy(this->portData->name, in, OFP_MAX_PORT_NAME_LEN);
        return true;
    }
    else
    {
        return false;
    }
}
bool OFPortDesc::set_config(uint32_t in)
{
    this->portData->config = in;
    return true;
}
bool OFPortDesc::set_state(uint32_t in)
{
    this->portData->state = in;
    return true;
}
bool OFPortDesc::set_type(uint16_t in)
{
    this->portData->properties->type = in;
    return true;
}
bool OFPortDesc::set_curr(uint32_t in)
{
    ofp_port_desc_prop_ethernet *prop = (ofp_port_desc_prop_ethernet *)this->portData->properties;
    prop->curr = in;
    return true;
}
bool OFPortDesc::set_advertised(uint32_t in)
{
    ofp_port_desc_prop_ethernet *prop = (ofp_port_desc_prop_ethernet *)this->portData->properties;
    prop->advertised = in;
    return true;
}
bool OFPortDesc::set_supported(uint32_t in)
{
    ofp_port_desc_prop_ethernet *prop = (ofp_port_desc_prop_ethernet *)this->portData->properties;
    prop->supported = in;
    return true;
}
bool OFPortDesc::set_peer(uint32_t in)
{
    ofp_port_desc_prop_ethernet *prop = (ofp_port_desc_prop_ethernet *)this->portData->properties;
    prop->peer = in;
    return true;
}
bool OFPortDesc::set_curr_speed(uint32_t in)
{
    ofp_port_desc_prop_ethernet *prop = (ofp_port_desc_prop_ethernet *)this->portData->properties;
    prop->curr_speed = in;
    return true;
}
bool OFPortDesc::set_max_speed(uint32_t in)
{
    ofp_port_desc_prop_ethernet *prop = (ofp_port_desc_prop_ethernet *)this->portData->properties;
    prop->max_speed = in;
    return true;
}

CLICK_ENDDECLS
ELEMENT_PROVIDES(OFPortDesc)
