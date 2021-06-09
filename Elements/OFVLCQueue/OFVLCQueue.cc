#include "OFVLCQueue.hh"

CLICK_DECLS

OFVLCQueue::OFVLCQueue() : _vlcReady(true), _DBlevel(0), _queueMaxSize(256) {}
OFVLCQueue::~OFVLCQueue() {}

void OFVLCQueue::cleanup(CleanupStage)
{
    while (!this->_queue.empty())
    {
        delete (this->_queue.front());
        this->_queue.pop();
    }
}

int OFVLCQueue::initialize(ErrorHandler *errh)
{
    return EXIT_SUCCESS;
}

int OFVLCQueue::configure(Vector<String> &conf, ErrorHandler *errh)
{
    if (Args(conf, this, errh)
            .read_mp("SIZE", this->_queueMaxSize)
            .read("DB", this->_DBlevel)
            .complete() < 0)
    {
        return -EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void OFVLCQueue::push(int inPort, Packet *p)
{
    switch (inPort)
    {
    case ToQueue: // To Queue
    {
        if (this->_queue.size() < this->_queueMaxSize)
        {
            this->_queue.push(OF::Packet::make((uint8_t *)p->data(), p->length()));
            if (this->_vlcReady && this->_queue.empty())
            {
                this->_vlcReady = false;
                this->output(0).push(Packet::make(this->_queue.front()->data(), this->_queue.front()->length()));
                delete (this->_queue.front());
                this->_queue.pop();
            }
        }
        else
        {
            click_chatter("WARNING: OFVLCQueue %s is full. Dropping Packet!", this->class_name());
        }
        p->kill();
        break;
    }
    case ACKFromVLC: // ACK from VLC Module
    {

        if (this->_DBlevel > 10)
            std::cout << "Queue Size: " << this->_queue.size() << std::endl;

        if (p->data()[8] == 0x40 && p->data()[9] == 0xf0)
        {
            this->_vlcReady = true;
            if (this->output(0).element()->input_is_push(0) && !this->_queue.empty())
            {
                this->output(0).push(Packet::make(this->_queue.front()->data(), this->_queue.front()->length()));
                delete (this->_queue.front());
                this->_queue.pop();
            }
            p->kill();
        }
        else
        {
            click_chatter("WARNING: Wrong Packet on input 1 of %s", this->class_name());
            p->kill();
        }
        break;
    }
    default:
        click_chatter("WARNING: Wrong input Port on %s", this->class_name());
        p->kill();
        break;
    }
}

Packet *OFVLCQueue::pull()
{
    WritablePacket *response = NULL;

    if (this->_vlcReady)
    {
        response = Packet::make(this->_queue.front()->data(), this->_queue.front()->length());
        delete (this->_queue.front());
        this->_queue.pop();

        this->_vlcReady = false;
    }

    return response;
}

CLICK_ENDDECLS

EXPORT_ELEMENT(OFVLCQueue)