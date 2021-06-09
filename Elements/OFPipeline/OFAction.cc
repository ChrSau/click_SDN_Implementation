#include "OFAction.hh"

CLICK_DECLS

OFAction::OFAction(uint8_t *data, uint16_t dataLength, std::shared_ptr<OFTableMng> tableMng, std::shared_ptr<OFPortMng> portMng)
{
    this->tableMng = tableMng;
    this->portMng = portMng;

    this->actionDataLength = dataLength;

    this->actionData = std::make_unique<uint8_t[]>(dataLength);
    for (size_t i = 0; i < dataLength; i++)
    {
        this->actionData[i] = data[i];
    }

    struct ofp_action_header *actionHeader = (struct ofp_action_header *)&this->actionData[0];

    switch (actionHeader->type)
    {
    case OFPAT_OUTPUT:
    {
        struct ofp_action_output *actionOutput = (struct ofp_action_output *)&this->actionData[0];
#if CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
        actionOutput->port = __builtin_bswap32(actionOutput->port);
        actionOutput->max_len = __builtin_bswap16(actionOutput->max_len);
#endif
        break;
    }
    case OFPAT_EXPERIMENTER:
    {

        break;
    }
    case OFPAT_COPY_TTL_OUT:
    case OFPAT_COPY_TTL_IN:
    case OFPAT_SET_MPLS_TTL:
    case OFPAT_DEC_MPLS_TTL:
    case OFPAT_PUSH_VLAN:
    case OFPAT_POP_VLAN:
    case OFPAT_PUSH_MPLS:
    case OFPAT_POP_MPLS:
    case OFPAT_SET_QUEUE:
    case OFPAT_GROUP:
    case OFPAT_SET_NW_TTL:
    case OFPAT_DEC_NW_TTL:
    case OFPAT_SET_FIELD:
    case OFPAT_PUSH_PBB:
    case OFPAT_POP_PBB:
    default:
        std::cerr << "ERROR not yet implemented" << std::endl;
        break;
    }
}

OFAction::OFAction(OFAction *in)
{
    this->actionData = std::make_unique<uint8_t[]>(in->getActionDataLength());
    uint8_t *data = in->getActionData();
    for (size_t i = 0; i < in->getActionDataLength(); i++)
    {
        this->actionData[i] = data[i];
    }
    this->actionDataLength = in->getActionDataLength();
    this->tableMng = in->getTableMng();
    this->portMng = in->getPortMng();
    std::free(data);
}

OFAction::OFAction(OFAction &in)
{
    this->actionData = std::make_unique<uint8_t[]>(in.getActionDataLength());
    uint8_t *data = in.getActionData();
    for (size_t i = 0; i < in.getActionDataLength(); i++)
    {
        this->actionData[i] = data[i];
    }
    this->actionDataLength = in.getActionDataLength();
    this->tableMng = in.getTableMng();
    this->portMng = in.getPortMng();
    std::free(data);
}

OFAction::~OFAction(){}

bool OFAction::applyActionOnPacket(Packet *p, uint32_t portNo)
{
    struct ofp_action_header *actionHeader = (struct ofp_action_header *)&this->actionData[0];

    switch (actionHeader->type)
    {
    case OFPAT_OUTPUT:
    {
        struct ofp_action_output *actionOutput = (struct ofp_action_output *)&this->actionData[0];

        if (actionOutput->port == OFPP_CONTROLLER)
        {
            this->tableMng->sendPacketToSocket(p, portNo);

        }
        else
        {
            struct OF::Structs::click_port_comm_header *header = (struct OF::Structs::click_port_comm_header *)std::malloc(sizeof(OF::Structs::click_port_comm_header) + p->length());
            header->identifier = OF::Structs::CPCI_FROM_PIPELINE;
            header->type = OF::Structs::CPCT_PACKET_OUT;
            header->length = p->length();

            std::memcpy(header->data, p->data(), p->length());
            

            WritablePacket *output = Packet::make(header, sizeof(OF::Structs::click_port_comm_header) + p->length());

            std::free(header);
            header = nullptr;
            
            if (p != nullptr)
                p->kill();
            p = nullptr;

            this->portMng->getPipeline()->output(actionOutput->port).push(output);
        }
        return true;
        break;
    }
    case OFPAT_EXPERIMENTER: // Own system to Discard Packets
    {
        struct ofp_action_experimenter_header *actionOutput = (struct ofp_action_experimenter_header *)&this->actionData[0];

        if (actionOutput->experimenter == 0)
        {
            p->kill();
            return true;
        }

        break;
    }
    case OFPAT_COPY_TTL_OUT:
    case OFPAT_COPY_TTL_IN:
    case OFPAT_SET_MPLS_TTL:
    case OFPAT_DEC_MPLS_TTL:
    case OFPAT_PUSH_VLAN:
    case OFPAT_POP_VLAN:
    case OFPAT_PUSH_MPLS:
    case OFPAT_POP_MPLS:
    case OFPAT_SET_QUEUE:
    case OFPAT_GROUP:
    case OFPAT_SET_NW_TTL:
    case OFPAT_DEC_NW_TTL:
    case OFPAT_SET_FIELD:
    case OFPAT_PUSH_PBB:
    case OFPAT_POP_PBB:

    default:
        break;
    }
    return false;
}

uint8_t *OFAction::getActionData()
{
    uint8_t *data = (uint8_t *)std::malloc(this->actionDataLength);
    std::memcpy(data, &this->actionData[0], this->actionDataLength);
    return data;
}
uint16_t OFAction::getActionDataLength() { return this->actionDataLength; }
std::shared_ptr<OFTableMng> OFAction::getTableMng() { return this->tableMng; }
std::shared_ptr<OFPortMng> OFAction::getPortMng() { return this->portMng; }

CLICK_ENDDECLS

ELEMENT_PROVIDES(OFAction)