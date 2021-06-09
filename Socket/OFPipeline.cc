#include "OFPipeline.hh"

OFPipeline::OFPipeline(TCPClient *tcp, int DBLevel)
{
    printf("OFPipeline start config\n");

    this->tcp = tcp;
    this->_DBlevel = DBLevel;

    int numberOfTables = 2;

    // Init PortMng
    this->portsMng = new OFPortMng(this->_DBlevel);

    // Add Live Ports to Element Task.
    for (auto Port : this->portsMng->getPorts())
    {
        uint32_t portNo = Port->getPortNo();

        if (Port->getPortState() == OFPPS_LIVE && portNo != OFPP_LOCAL)
        {
            if (Port->fd() >= 0)
            {
                // add_select(Port.fd(), SELECT_READ);
                std::thread thread(&OFPipeline::selected, this, Port->fd(), 0);
                this->threads.push_back(std::move(thread));
            }
        }
    }

    // Init TableMng
    this->tablesMng = new OFTableMng(this->_DBlevel, numberOfTables);

    printf("OFPipeline configured\n");
}

OFPipeline::~OFPipeline()
{
    // this->portsMng->cleanup();
    // this->tablesMng->cleanup();

    delete this->portsMng;
    delete this->tablesMng;

    this->portsMng = nullptr;
    this->tablesMng = nullptr;

    for (auto &&thread : this->threads)
    {
        thread.join();
        thread.~thread();
    }

    this->threads.clear();
}

myPacket *OFPipeline::simple_action(int port, myPacket *p)
{
    myPacket *responseTables = this->tablesMng->push(0, p->clone(), 0);
    myPacket *responsePorts = this->portsMng->push(0, p->clone());

    // p->kill();

    if (responsePorts != nullptr && responsePorts != NULL && responsePorts->length() != 0)
    {
        if (_DBlevel > 10)
            OF::Functions::printHexAndChar(responsePorts->data(), responsePorts->length(), "Out Ports: ");
        // output(0).push(responsePorts);

        if (responseTables != NULL && responseTables != nullptr)
            responseTables->kill();

        tcp->Send(responsePorts->data(), responsePorts->length());

        // return responsePorts;
        delete(responsePorts);
    }
    else if (responseTables != nullptr && responseTables != NULL && responseTables->length() != 0)
    {
        if (_DBlevel > 10)
            OF::Functions::printHexAndChar(responseTables->data(), responseTables->length(), "Out Tables: ");
        // output(0).push(responseTables);

        if (responsePorts != NULL && responsePorts != nullptr)
            responsePorts->kill();

        tcp->Send(responseTables->data(), responseTables->length());
        // return responseTables;
        delete(responseTables);
    }

    return NULL;
}

// PortMng functions
void OFPipeline::selected(int fd, int mask)
{
    uint32_t portNo = OFPP_LOCAL;
    myPacket *p = this->portsMng->selected(fd, mask, &portNo);
    if (p != NULL)
    {
        myPacket *responseTables = NULL; // = this->tablesMng->push(1, p->clone(), portNo);

        if (responseTables != NULL && portNo != OFPP_LOCAL)
        {
            if (_DBlevel > 1)
                std::cout << "responseTables != NULL" << std::endl;

            struct ofp_packet_in *checkStruct = (struct ofp_packet_in *)responseTables->data();
            if (checkStruct->header.type == OFPT_PACKET_IN)
            {

                if (_DBlevel > 10)
                    OF::Functions::printHex((uint8_t *)responseTables->data(), responseTables->length(), "myPacket In: ");

                // output(0).push(responseTables->clone());
                checkStruct = nullptr;
            }
            responseTables->kill();
            responseTables = nullptr;
        }
        p->kill();
    }
    p = nullptr;
}
