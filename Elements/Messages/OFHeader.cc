#include "OFHeader.hh"

#include "../Shared/Data.hh"
#include "../Utilities/openflowWrapper.hh"
#include "../Utilities/OFStructs.hh"
#include "../Utilities/OFPaket.hh"

using namespace OF;

struct OFHeader::impl
{
    uint8_t version = 0; /* OFP_VERSION. */
    uint8_t type = 0;    /* One of the OFPT_ constants. */
    uint16_t length = 0; /* Length including this ofp_header. */
    uint32_t xid = 0;    /* Transaction id associated with this packet.
                           Replies use the same id as was in the request
                           to facilitate pairing. */
};

OFHeader::OFHeader()
    : m{std::make_unique<impl>()}
{
}

OFHeader::~OFHeader() {}

OFHeader::OFHeader(const OFHeader &_in) noexcept
    : m{std::make_unique<impl>()}
{
}
OFHeader::OFHeader(OFHeader &&_in) noexcept
    : m{std::make_unique<impl>()}
{
}

OFHeader &OFHeader::operator=(const OFHeader &_in) noexcept
{
}
OFHeader &OFHeader::operator=(OFHeader &&_in) noexcept
{
}

Packet *OFHeader::createPacket() const noexcept
{
    Packet *result = nullptr;

    switch (static_cast<OF::Structs::OFVersions>(Data::getIntance().getOFVersion()))
    {
    case OF::Structs::OFVersions::Version1:
    case OF::Structs::OFVersions::Version2:
    case OF::Structs::OFVersions::Version3:
    case OF::Structs::OFVersions::Version4:
    case OF::Structs::OFVersions::Version5:
    {
        OF_1_5::ofp_header header{};
        header.version = m->version;
        header.type = m->type;
        header.length = m->length;
        header.xid = m->xid;
#if CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
        header.length = __builtin_bswap16(header.length);
        header.xid = __builtin_bswap32(header.xid);
#endif
        result = Packet::make(reinterpret_cast<uint8_t *>(&header), sizeof(header));
        break;
    }
    default:
        break;
    }

    return result;
}

void OFHeader::createFromPacket(const Packet &_in) noexcept 
{
    switch (static_cast<OF::Structs::OFVersions>(Data::getIntance().getOFVersion()))
    {
    case OF::Structs::OFVersions::Version1:
    case OF::Structs::OFVersions::Version2:
    case OF::Structs::OFVersions::Version3:
    case OF::Structs::OFVersions::Version4:
    case OF::Structs::OFVersions::Version5:
    {
        OF_1_5::ofp_header *header = reinterpret_cast<OF_1_5::ofp_header *>(_in.data());
        m->version = header->version;
        m->type = header->type;
        m->length = header->length;
        m->xid = header->xid;
#if CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
        m->length = __builtin_bswap16(m->length);
        m->xid = __builtin_bswap32(m->xid);
#endif
        break;
    }
    default:
        break;
    }
}

uint8_t OFHeader::getOFVersion() const noexcept
{
    return m->version;
}
OFHeader &OFHeader::setOFVersion(const uint8_t &_in) noexcept
{
    m->version = _in;
    return *this;
}

uint8_t OFHeader::getMessageType() const noexcept
{
    return m->type;
}
OFHeader &OFHeader::setMessageType(const uint8_t &_in) noexcept
{
    m->type = _in;
    return *this;
}

uint16_t OFHeader::getMessageLength() const noexcept
{
    return m->length;
}
OFHeader &OFHeader::setMessageLength(const uint16_t &_in) noexcept
{
    m->length = _in;
    return *this;
}

uint32_t OFHeader::getMessageXid() const noexcept
{
    return m->xid;
}
OFHeader &OFHeader::setMessageXid(const uint32_t &_in) noexcept
{
    m->xid = _in;
    return *this;
}