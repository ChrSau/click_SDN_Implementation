#ifndef PACKET_HH
#define PACKET_HH

#include <iostream>
#include <vector>
#include <cstring>
#include "../OFHelper.hh"

class myPacket
{
private:

    uint8_t *myData;
    int packetLength;
    std::vector<uint8_t> _data;

public:
    enum
    {
        default_headroom = 28, ///< Default packet headroom() for
                               ///  Packet::make().  4-byte aligned.
        min_buffer_length = 64 ///< Minimum buffer_length() for
                               ///  Packet::make()
    };

    myPacket();
    myPacket(uint8_t *, int);
    ~myPacket();

    myPacket *make(uint8_t *data, size_t lenght);
    myPacket *make(uint32_t headroom, const void *data, uint32_t length, uint32_t tailroom);
    uint8_t *data();
    int length();
    bool kill();

    struct OF::Structs::ethernet_header *ether_header();

    myPacket *clone();
};

#endif