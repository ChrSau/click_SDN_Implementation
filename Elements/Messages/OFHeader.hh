#pragma once

#include "BaseMessage.hh"
#include <memory>

class OFHeader : public BaseMessage
{
private:
    struct impl;
    std::unique_ptr<impl> m;

public:
    OFHeader(/* args */);
    ~OFHeader();

    OFHeader(const OFHeader &_in) noexcept;
    OFHeader(OFHeader &&_in) noexcept;

    OFHeader &operator=(const OFHeader &_in) noexcept;
    OFHeader &operator=(OFHeader &&_in) noexcept;

    Packet *createPacket() const noexcept;
    void createFromPacket(const Packet &_in) noexcept;

    uint8_t getOFVersion() const noexcept;
    OFHeader &setOFVersion(const uint8_t &_in) noexcept;

    uint8_t getMessageType() const noexcept;
    OFHeader &setMessageType(const uint8_t &_in) noexcept;

    uint16_t getMessageLength() const noexcept;
    OFHeader &setMessageLength(const uint16_t &_in) noexcept;

    uint32_t getMessageXid() const noexcept;
    OFHeader &setMessageXid(const uint32_t &_in) noexcept;
};
