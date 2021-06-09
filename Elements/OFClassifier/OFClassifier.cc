#include "OFClassifier.hh"

CLICK_DECLS

OFClassifier::OFClassifier() {}

OFClassifier::OFClassifier(OFClassifier *&in) {}

OFClassifier::~OFClassifier() {}

void OFClassifier::cleanup(CleanupStage) {}

int OFClassifier::initialize(ErrorHandler *errh)
{
    if (this->_DBlevel < 0)
        this->_DBlevel = 0;

    return EXIT_SUCCESS;
}

int OFClassifier::configure(Vector<String> &conf, ErrorHandler *errh)
{

    if (Args(conf, this, errh)
            .read("DB", this->_DBlevel)
            .complete() < 0)
    {
        return -EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void OFClassifier::push(int, Packet *p)
{
    struct ofp_header *header = (ofp_header *)p->data();

    if (header->version == OFP_VERSION || header->version == 0x05)
    {
        uint16_t packetLength = header->length;
#if __BYTE_ORDER == __LITTLE_ENDIAN
        packetLength = __builtin_bswap16(packetLength);
#endif

        if (p->length() > packetLength && header->type != OFPT_PACKET_OUT)
        {
            class Packet *backToQueue = Packet::make(&p->data()[packetLength], p->length() - packetLength);

            this->push(0, backToQueue);
            backToQueue->kill();

            class Packet *forward = Packet::make(p->data(), packetLength);

            p->kill();
            p = nullptr;

            p = forward->clone();

            forward->kill();

            header = (struct ofp_header *)p->data();
        }

        switch (header->type)
        {
        case OFPT_HELLO:
        case OFPT_ERROR:
        case OFPT_ECHO_REQUEST:
        case OFPT_EXPERIMENTER:
        case OFPT_FEATURES_REQUEST:
        case OFPT_GET_CONFIG_REQUEST:
        case OFPT_BARRIER_REQUEST:
        case OFPT_SET_CONFIG:
        case OFPT_ROLE_REQUEST:

            output(0).push(p);
            break;

        case OFPT_FLOW_MOD:
        // case OFPT_GROUP_MOD:
        case OFPT_PACKET_OUT:

            output(1).push(p);
            break;

        case OFPT_MULTIPART_REQUEST:
        {
            struct ofp_multipart_request *multipartHeader = (ofp_multipart_request *)p->data();

#if CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
            multipartHeader->type = __builtin_bswap16(multipartHeader->type);
#endif

            switch (multipartHeader->type)
            {
            case OFPMP_DESC:

                output(0).push(p);
                break;

            case OFPMP_TABLE_FEATURES:
            case OFPMP_PORT_DESC:
            case OFPMP_AGGREGATE_STATS:
            case OFPMP_TABLE_STATS:
            case OFPMP_GROUP_FEATURES:

                output(1).push(p);
                break;

            default:
                std::string errorMsg = "ERROR OFClassifier: Wrong Messagetype in Multipart Message! " + std::to_string(multipartHeader->type);
                OF::Functions::printHexAndCharToError((uint8_t *)p->data(), p->length());
                click_chatter(errorMsg.c_str());
                break;
            }

            multipartHeader = nullptr;
            break;
        }

        default:
            std::string errorMsg = "ERROR OFClassifier: Wrong Messagetype! " + std::to_string(header->type);
            OF::Functions::printHexAndCharToError((uint8_t *)p->data(), p->length());
            click_chatter(errorMsg.c_str());
            break;
        }
    }
    else
    {
        std::string errorMsg = "ERROR OFClassifier: Wrong OpenFlowversion! " + std::to_string(header->version);
        click_chatter(errorMsg.c_str());
    }

    header = nullptr;
    p->kill();
}

CLICK_ENDDECLS

EXPORT_ELEMENT(OFClassifier)