#include "OFQueue.hh"

CLICK_DECLS

OFQueue::OFQueue() {}
OFQueue::~OFQueue() {}

int OFQueue::initialize(ErrorHandler *)
{
    click_chatter("OFQueue: Init successfull\n");
    return 0;
}

int OFQueue::configure(Vector<String> &conf, ErrorHandler *errh)
{

    if (Args(conf, this, errh)
            .read_mp("DB", _DBlevel)
            .read_mp("QUEUESIZE", this->maxQueueSize)
            .complete() < 0)
    {
        click_chatter("Error OFQueue configuration\n");
        return -1;
    }

    this->counter = (struct OF::Structs::ofp_queue_counter *)std::calloc(1, sizeof(struct OF::Structs::ofp_queue_counter));

    click_chatter("OFQueue configured\n");
    return 0;
}

void OFQueue::cleanup(CleanupStage)
{
    std::free(this->counter);
    this->counter = nullptr;

    while (this->outputQueue.size() > 0)
    {
        delete (this->outputQueue.front());
        this->outputQueue.pop();
    }
}

Packet *OFQueue::pull(int port)
{
    if (port == 0)
    {
        if (this->outputQueue.size() > 0)
        {
            if (_DBlevel > 10)
                std::cout << "Es ist etwas in der Queue!" << std::endl;

            Packet *returnValue = Packet::make(this->outputQueue.front()->data(), this->outputQueue.front()->length());

            if (_DBlevel > 10)
                OF::Functions::printHexAndChar((uint8_t *)returnValue->data(), returnValue->length(), "Queue Output");

            delete (this->outputQueue.front());
            this->outputQueue.pop();

            return returnValue;
        }
        else
        {
            return NULL;
        }
    }

    return NULL;
}

void OFQueue::push(int port, Packet *p)
{
    switch (port)
    {
    case 0:
        this->normalMessage(p);
        break;

    case 1:
        this->openFlowMessage(p);
        break;
    }
}

void OFQueue::openFlowMessage(Packet *p)
{
    p->kill();
}

void OFQueue::normalMessage(Packet *p)
{
    if (this->_DBlevel > 10)
    {
        OF::Functions::printHexAndChar((uint8_t *)p->data(), p->length(), "OFQueue input normal Message: ");
    }

    if (this->outputQueue.size() < this->maxQueueSize)
    {

        OF::Packet *toSavePacket = OF::Packet::make((uint8_t *)p->data(), p->length());

        p->kill();
        p = nullptr;

        if (this->_DBlevel > 10)
            OF::Functions::printHexAndChar((uint8_t *)toSavePacket->data(), toSavePacket->length(), "Input Message after Copy: ");

        this->outputQueue.push(toSavePacket);
        this->counter->Transmit_Bytes += toSavePacket->length();
        this->counter->Transmit_Packets++;
    }
    else
    {
        this->counter->Transmit_Overrun_Errors++;
        output(1).push(p);
    }

    if (_DBlevel > 10)
        std::cout << "Queue Stats: " << std::endl
                  << "  Transmit Bytes: " << std::dec << this->counter->Transmit_Bytes << std::endl
                  << "  Transmit Packets: " << std::dec << this->counter->Transmit_Packets << std::endl
                  << "  Transmit Overrun Errors: " << std::dec << this->counter->Transmit_Overrun_Errors << std::endl
                  << std::endl;
}

CLICK_ENDDECLS

EXPORT_ELEMENT(OFQueue)