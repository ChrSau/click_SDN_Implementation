#pragma once

// Class forwarding
class Packet;

class BaseMessage
{
private:

public:
    BaseMessage();
    virtual ~BaseMessage() = 0;

    virtual Packet createPacket() = 0;
    virtual void createFromPacket(const Packet &) = 0;
};

