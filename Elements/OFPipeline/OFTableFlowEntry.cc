#include "OFTableFlowEntry.hh"

CLICK_DECLS

OFTableFlowEntry::OFTableFlowEntry(uint8_t *matchStruct,
                                   uint16_t,
                                   uint16_t Priority,
                                   struct ofp_instruction_header *,
                                   uint16_t Timeouts,
                                   uint64_t Cookie,
                                   uint64_t Flags,
                                   std::shared_ptr<class OFPipeline *> pipeline,
                                   std::shared_ptr<class OFTableMng> tableMng,
                                   std::shared_ptr<class OFPortMng> portMng)
{
    this->pipeline = pipeline;
    this->tableMng = tableMng;
    this->portMng = portMng;

    if (Timeouts != 0)
    {
        this->Timeouts = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count() + Timeouts;
    }
    else
    {
        this->Timeouts = -1;
    }

    this->Priority = Priority;
    this->Cookie = Cookie;
    this->Flags = Flags;

    // Match Header and OXM Fields
    struct ofp_match *matchHeader = (struct ofp_match *)matchStruct;

    if (matchHeader->length > 4)
    {
        const uint16_t lenghtOfMatchTypeAndLength = sizeof(((ofp_match *)0)->type) + sizeof(((ofp_match *)0)->length);
        int lengthLeft = matchHeader->length - lenghtOfMatchTypeAndLength;
        const uint8_t oxmHeaderLength = sizeof(struct OF::Structs::my_ofp_oxm_header);

        while (lengthLeft > 0)
        {
            struct OF::Structs::my_ofp_oxm_header *oxmHeader = (struct OF::Structs::my_ofp_oxm_header *)(matchStruct + matchHeader->length - lengthLeft);

            uint8_t *oxmData = (uint8_t *)std::malloc(oxmHeader->oxm_length + oxmHeaderLength);
            std::memcpy(oxmData, oxmHeader, oxmHeader->oxm_length + oxmHeaderLength);

            class OFOXMField *newOXMField = new OFOXMField(oxmData, oxmHeader->oxm_length + oxmHeaderLength);

            this->oxmFields.push_back(std::make_shared<class OFOXMField>(newOXMField));

            if (newOXMField != nullptr)
                delete newOXMField;
            newOXMField = nullptr;

            lengthLeft = lengthLeft - oxmHeader->oxm_length - oxmHeaderLength;
            std::free(oxmData);
            oxmHeader = nullptr;
        };
    }

    // Instructions
    int startOfInstruction = (matchHeader->length % 8) == 0 ? matchHeader->length : matchHeader->length + 8 - (matchHeader->length % 8); // ofp_match is padded as needed, to make its overall size a multiple of 8, to preserve alignment in structures using it

    struct ofp_instruction_header *instructionHeader = (struct ofp_instruction_header *)(matchStruct + startOfInstruction);

#if CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
    instructionHeader->type = __builtin_bswap16(instructionHeader->type);
    instructionHeader->len = __builtin_bswap16(instructionHeader->len);
#endif

    switch (instructionHeader->type)
    {
    case OFPIT_APPLY_ACTIONS:
    {
        if (instructionHeader->len > 8)
        {
            struct ofp_instruction_actions *instructionAction = (struct ofp_instruction_actions *)instructionHeader;

            struct ofp_action_header *actionHeader = (struct ofp_action_header *)instructionAction->actions;
#if CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
            actionHeader->type = __builtin_bswap16(actionHeader->type);
            actionHeader->len = __builtin_bswap16(actionHeader->len);
#endif
            if (actionHeader->len > sizeof(struct ofp_instruction_actions))
            {
                uint8_t *actionData = (uint8_t *)std::malloc(actionHeader->len);
                std::memcpy(actionData, actionHeader, actionHeader->len);
                class OFAction *newOFAction = new OFAction(actionData, actionHeader->len, this->tableMng, this->portMng);
                this->action = newOFAction;

                newOFAction = nullptr;

                std::free(actionData);
                actionData = nullptr;
            }
        }
        else
        {
            // Packets without actions, should be droped

            struct ofp_action_experimenter_header *actionHeader = (struct ofp_action_experimenter_header *)std::calloc(1, sizeof(struct ofp_action_experimenter_header));
            actionHeader->type = OFPAT_EXPERIMENTER;
            actionHeader->len = sizeof(struct ofp_action_experimenter_header);
            actionHeader->experimenter = 0; // If 0, Packet will be droped

            this->action = new OFAction((uint8_t *)actionHeader, actionHeader->len, this->tableMng, this->portMng);
        }
        break;
    }
    case OFPIT_GOTO_TABLE:
    case OFPIT_WRITE_METADATA:
    case OFPIT_WRITE_ACTIONS:
    case OFPIT_CLEAR_ACTIONS:
    case OFPIT_DEPRECATED: // Former OFPIT_METER [Apply meter (rate limiter)]
    case OFPIT_STAT_TRIGGER:
    case OFPIT_EXPERIMENTER:
    default:
        std::cerr << "ERROR IN FLOWENTRY CONSTRUCTOR!!!" << std::endl;
        break;
    }

    std::free(matchStruct);
}

OFTableFlowEntry::OFTableFlowEntry(OFTableFlowEntry *&in)
{
    std::cout << "OFTableFlowEntry Copy *" << std::endl;

    this->pipeline = in->getPipeline();
    this->tableMng = in->getTableMng();
    this->portMng = in->getPortMng();
    this->action = new OFAction(in->getAction());
    this->oxmFields = in->getOxmFields();

    this->ReceivedPacketsCounter = in->getReceivedPacketsCounter();
    this->ReceivedBytesCounter = in->getReceivedBytesCounter();
    this->Duration_s_Counter = in->getDuration_s_Counter();
    this->Duration_ns_Counter = in->getDuration_ns_Counter();
    this->Timeouts = in->getTimeouts();
    this->Cookie = in->getCookie();
    this->Flags = in->getFlags();
    this->Priority = in->getPriority();
}

OFTableFlowEntry::~OFTableFlowEntry()
{
    delete this->action;
    this->action = nullptr;
}

bool OFTableFlowEntry::checkPacketOnFlow(Packet *p, uint32_t portNo)
{
    this->ReceivedPacketsCounter += 1;
    this->ReceivedBytesCounter += p->length();

    for (auto &&oxmField : this->oxmFields)
    {
        if (!oxmField->checkPacketInOXMField(p, portNo))
        {
            return false;
        }
    }
    if (this->action != NULL)
    {
        if (!this->action->applyActionOnPacket(p, portNo))
        {
            std::cerr << "WARNING: Action konnte nicht ausgeführt werden." << std::endl;
            if (p != nullptr)
            {
                p->kill();
                p = nullptr;
            }
            return false;
        }
        else
        {
            return true;
        }
    }
    if (p != nullptr)
    {
        p->kill();
        p = nullptr;
    }

    return true;
}

void OFTableFlowEntry::setTimeout(int64_t timeout) { this->Timeouts = timeout; }

std::shared_ptr<class OFPipeline *> OFTableFlowEntry::getPipeline() { return this->pipeline; }
std::shared_ptr<class OFTableMng> OFTableFlowEntry::getTableMng() { return this->tableMng; }
std::shared_ptr<class OFPortMng> OFTableFlowEntry::getPortMng() { return this->portMng; }
class OFAction *OFTableFlowEntry::getAction() { return this->action; }
std::vector<std::shared_ptr<class OFOXMField>> OFTableFlowEntry::getOxmFields() { return this->oxmFields; }

CLICK_ENDDECLS
ELEMENT_PROVIDES(OFTableFlowEntry)