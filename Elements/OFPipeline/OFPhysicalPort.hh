#ifndef CLICK_OFPHYSICALPORT__HH
#define CLICK_OFPHYSICALPORT__HH 1

// Standard Libs
#include <string>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <cstring>

// OpenFlow Header
#include "../Utilities/openflow.h"

// Click Include
#include <click/config.h>
#include <click/element.hh>
#include <click/etheraddress.hh>
#include <click/error.hh>
#include <click/straccum.hh>
#include <click/args.hh>
#include <click/glue.hh>
#include <click/packet_anno.hh>
#include <click/standard/scheduleinfo.hh>
#include <click/userutils.hh>

// Helper Classes
#include "OFTableFlowEntry.hh"
#include "OFPortMng.hh"
#include "../Utilities/OFFunctions.hh"
#include "../Utilities/OFStructs.hh"
#include "../Utilities/OFConstants.hh"

// OS Libs
#include <sys/socket.h>
#include <net/if.h>
#include <features.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>

#ifdef __linux__
#define FROMDEVICE_ALLOW_LINUX 1
#endif

#define FAKE_DLT_EN10MB 1 /* Ethernet (10Mb) */

CLICK_DECLS

class OFPhysicalPort
{
private:
    struct OF::Structs::my_ofp_port *portStruct;
    struct OF::Structs::my_ofp_port_counter *portCounter;

    class OFPortMng *PortMng;

    enum
    {
        default_snaplen = 2046
    };

    int _fd;
    bool _force_ip;
    int _burst;
    int _datalink;

    std::string _ifname;
    bool _promisc : 1;
    int _was_promisc : 2;
    int _snaplen;
    uint16_t _protocol;
    unsigned _headroom;

public:
    OFPhysicalPort(uint32_t portNo,
                   uint8_t *hardAddr,
                   std::string name,
                   uint32_t config,
                   uint32_t state,
                   uint16_t type,
                   uint32_t curr,
                   uint32_t advertised,
                   uint32_t supported,
                   uint32_t peer,
                   uint32_t curr_speed,
                   uint32_t max_speed,
                   ErrorHandler *errh,
                   OFPortMng *_PortMng);
    OFPhysicalPort(OFPhysicalPort *in);
    ~OFPhysicalPort();

    struct OF::Structs::my_ofp_port *getPortStruct();
    struct OF::Structs::my_ofp_port_counter *getPortCounter();
    uint32_t getPortStructSize();
    uint32_t getPortCounterSize();
    uint32_t getPortState();
    uint32_t getPortNo()
    {
#if CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
        return __builtin_bswap32(this->portStruct->port_no);
#elif CLICK_BYTE_ORDER == CLICK_BIG_ENDIAN
        return this->portStruct->port_no;
#endif
    }


    void setStats(struct OF::Structs::my_ofp_port_counter *);

    void cleanup();

    inline int fd() const
    {
        return _fd;
    }

    Packet *selected(int fd, int mask);

    int linux_fd() const
    {
        return _fd;
    }

    static int open_packet_socket(std::string, ErrorHandler *);
    static int set_promiscuous(int, std::string, bool);

    int send_packet(Packet *p);
};

CLICK_ENDDECLS
#endif