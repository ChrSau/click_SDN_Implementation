#include "Packet.hh"

myPacket::myPacket()
{
    this->packetLength = 0;
    this->myData = nullptr;
}

myPacket::myPacket(uint8_t *data, int length)
{
    // for (int i = 0; i < length; i++)
    // {
    //     this->_data.push_back(data[i]);
    // }

    this->myData = (uint8_t *)std::malloc(length);

    std::memcpy(this->myData, data, length);

    this->packetLength = length;

    // std::cout << "zweite Stelle: " << std::hex << std::setfill('0') << std::setw(2) << (int)data[1];

    // OF::Functions::printHex(&this->_data[0], this->length(), "At cloning: ");
}

myPacket::~myPacket()
{
    this->kill();
}

myPacket *myPacket::make(uint8_t *data, size_t length)
{
    // if (this->_data.size() > 0)
    // {
    //     std::cout << "Delete Data in Packet!!!!" << std::endl;
    //     this->_data.erase(this->_data.begin(), this->_data.end());
    // }

    // uint8_t *inData = (uint8_t *)data;

    if (this->packetLength != 0)
    {
        std::free(this->myData);
        this->myData = nullptr;
        this->packetLength = 0;
    }

    this->myData = (uint8_t *)std::malloc(length);

    this->packetLength = length;

    std::memcpy(this->myData, data, length);

    // OF::Functions::printHex(data, length,"Befor copy: ");
    // for (size_t i = 0; i < length; i++)
    // {
    //     this->_data.push_back(data[i]);
    // }
    // OF::Functions::printHex(&this->_data[0], length,"After copy: ");

    return this;
}

myPacket *myPacket::make(uint32_t headroom, const void *data, uint32_t length, uint32_t tailroom)
{
    if (this->_data.size() > 0)
        this->_data.erase(this->_data.begin(), this->_data.end());

    // this->_data.resize(headroom + length + tailroom);

    // std::cout << "this->_data.size(): " << this->_data.size() << std::endl;

    if (length > 0)
    {
        // for (size_t i = headroom; i < length - tailroom; i++)
        // {
        //     this->_data.push_back(((uint8_t *)data)[i]);
        // }
    }
    // std::cout << "Hallo" << std::endl;

    // uint8_t *inData = (uint8_t *)data;
    // for (size_t i = 0; i < length; i++)
    // {
    //     this->_data.push_back(inData[i]);
    // }

    return this;
}

uint8_t *myPacket::data()
{
    return this->myData;
    // return (uint8_t *)&this->_data[0];
}

int myPacket::length()
{
    return this->packetLength;
    // return this->_data.size();
}

bool myPacket::kill()
{
    if (this->myData != nullptr)
        std::free(this->myData);

    this->myData = nullptr;
    return true;
}

myPacket *myPacket::clone()
{

    // OF::Functions::printHex(this->data(), this->length(), "befor cloning: ");

    myPacket *response = new myPacket(this->data(), this->length());

    // OF::Functions::printHex(response->data(), response->length(), "After cloning: ");

    // response->make((uint8_t *)&this->_data[0], this->_data.size());

    // this->kill();

    return response;
}

struct OF::Structs::ethernet_header *myPacket::ether_header()
{
    struct OF::Structs::ethernet_header *response = (struct OF::Structs::ethernet_header *)this->myData;

    return response;
}
