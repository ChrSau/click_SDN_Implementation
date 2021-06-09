#include "OFSocketClient.hh"

CLICK_DECLS

OFSocketClient::OFSocketClient()
{
    this->sock = -1;
    this->port = 0;
    this->address = "";
    this->responder = nullptr;
    this->interfaceName = "";
}

OFSocketClient::OFSocketClient(OFResponder *responder)
{
    this->sock = -1;
    this->port = 0;
    this->address = "";
    this->responder = responder;
    this->interfaceName = "";
}

OFSocketClient::OFSocketClient(std::string address = "", int port = 0, std::string interfaceName = "", OFResponder *responder = NULL)
{
    this->sock = -1;
    this->port = port;
    this->address = address;
    if (responder != NULL)
        this->responder = responder;
    this->interfaceName = interfaceName;
}

OFSocketClient::OFSocketClient(std::string address = "", int port = 0, int sock = -1, std::string interfaceName = "", OFResponder *responder = NULL)
{
    this->sock = sock;
    this->port = port;
    this->address = address;
    if (responder != NULL)
        this->responder = responder;
    this->interfaceName = interfaceName;
}

OFSocketClient::OFSocketClient(OFSocketClient &in)
{
    this->sock = in.getSock();
    this->port = in.getPort();
    this->address = in.getAddress();
    this->server = in.getServer();
    this->responder = in.getResponder();
    this->interfaceName = in.getInterfaceName();
}

OFSocketClient::~OFSocketClient() {}

bool OFSocketClient::connectToRemote()
{
    if (this->address.compare("") && this->port != 0 && this->interfaceName.compare(""))
    {
        return this->connectToRemote(this->address, this->port, this->interfaceName);
    }
    else
    {
        return false;
    }
}

bool OFSocketClient::connectToRemote(std::string address, int port, std::string interfaceName)
{
    if (this->address.compare(address) && this->port != port && this->interfaceName.compare(interfaceName))
    {
        this->port = port;
        this->address = address;
        this->interfaceName = interfaceName;
    }

    struct ifreq ifr;

    if (this->sock == -1)
    {
        this->sock = socket(AF_INET, SOCK_STREAM, 0);
        if (this->sock == -1)
        {
            std::cerr << "Could not create socket" << std::endl;
        }
    }
    if (inet_addr(address.c_str()))
    {
        struct hostent *he;

        if ((he = gethostbyname(address.c_str())) == NULL)
        {
            herror("gethostbyname");
            std::cerr << "Failed to resolve hostname\n";
            return false;
        }
        std::memcpy((char *)&server.sin_addr.s_addr, (char *)he->h_addr, he->h_length);
    }
    else
    {
        server.sin_addr.s_addr = inet_addr(address.c_str());
    }
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    std::memset(&ifr, 0, sizeof(ifr));
    std::memcpy(ifr.ifr_name, this->interfaceName.c_str(), IF_NAMESIZE <= (this->interfaceName.size() + 1) ? IF_NAMESIZE : (this->interfaceName.size() + 1));

    if (setsockopt(this->sock, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr, sizeof(ifr)) < 0)
    {
        std::cerr << "Fehler, Socket konnte nicht gebunden werden" << std::endl;
    }
    else
    {
        std::cout << "Socket wurde gebunden" << std::endl;
    }

    if (connect(this->sock, (struct sockaddr *)&this->server, sizeof(this->server)) < 0)
    {
        std::cerr << "connect failed. Error";
        return false;
    }

    return true;
}

bool OFSocketClient::Send(Packet *p)
{
    if (p)
    {
        return this->Send((uint8_t *)p->data(), p->length());
    }
    else
    {
        return false;
    }
}

bool OFSocketClient::Send(uint8_t *data, size_t length)
{
    if (data != nullptr && data != NULL && length > 0)
    {
        if (sock != -1)
        {
            if (send(sock, data, length, 0) < 0)
            {
                std::cerr << "Send failed : " << std::endl;
                return false;
            }
        }
        else
            return false;
        return true;
    }
    else
    {
        return false;
    }
}

void OFSocketClient::receive()
{
    while (this->responder->getRun())
    {
        // int n;
        WritablePacket *p;

        p = Packet::make(this->default_headroom, 0, this->default_snaplen, 0);

        ssize_t _len = read(this->sock, p->data(), p->length());
        if (_len < 0)
        {
            std::cerr << "receive failed!" << std::endl;
            std::cerr << strerror(errno) << "\n";

            p->kill();
        }
        else if (_len == 0)
        {

            std::cerr << strerror(errno) << "\n";
            
            std::cerr << "Connection Closed\n";

            this->exit();

            p->kill();
            break;
        }
        else
        {
            if (_len > this->default_snaplen)
            {
                std::cerr << "Packet was bigger then the default_snaplen\n";
                p->kill();
            }
            else
            {
                // trim packet to actual length
                p->take(this->default_snaplen - _len);
            }

            p->timestamp_anno().assign_now();

            this->responder->simple_action(p);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void OFSocketClient::exit()
{
    if (this->sock != -1)
    {
        close(this->sock);
        this->sock = -1;
    }
}

std::string OFSocketClient::getAddress() { return this->address; }
int OFSocketClient::getPort() { return this->port; }
sockaddr_in OFSocketClient::getServer() { return this->server; }
int OFSocketClient::getSock() { return this->sock; }
OFResponder *OFSocketClient::getResponder() { return this->responder; }
std::string OFSocketClient::getInterfaceName() { return this->interfaceName; }

CLICK_ENDDECLS

ELEMENT_PROVIDES(OFSocketClient)
ELEMENT_LIBS(-L /usr/local/lib -lpthread)