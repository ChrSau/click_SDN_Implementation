#ifndef CLICK_OFPHYSICALPORT__HH
#define CLICK_OFPHYSICALPORT__HH
// Standard Libs
#include <string>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <cstring>
// OpenFlow Header
#include "../openflow.h"
// Helper Classes
// #include "OFTableFlowEntry.hh"
#include "../OFHelper.hh"
#include "Packet.hh"
// OS Libs
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <features.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <assert.h>

// #ifdef __linux__
// #define FROMDEVICE_ALLOW_LINUX 1
// #endif

// #define FAKE_DLT_EN10MB 1 /* Ethernet (10Mb) */

class OFPhysicalPort
{
private:
    struct OF::Structs::my_ofp_port portStruct;
    struct OF::Structs::my_ofp_port_counter portCounter;

    enum
    {
        default_snaplen = 2046
    };

    int _fd;

    bool _force_ip;
    int _burst;
    int _datalink;

    std::string _ifname;
    bool _sniffer : 1;
    bool _promisc : 1;
    bool _outbound : 1;
    bool _timestamp : 1;
    int _was_promisc : 2;
    int _snaplen;
    uint16_t _protocol;
    unsigned _headroom;
    enum
    {
        method_default,
        method_netmap,
        method_pcap,
        method_linux
    };
    int _method;

    // static String read_handler(Element *, void *) CLICK_COLD;
    // static int write_handler(const String &, Element *, void *, ErrorHandler *) CLICK_COLD;

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
                   uint32_t max_speed);
    ~OFPhysicalPort();

    struct OF::Structs::my_ofp_port *getPortStruct();
    uint32_t getPortStructSize();
    uint32_t getPortState();
    uint32_t getPortNo();

    void setStats(struct OF::Structs::my_ofp_port_counter *);

    void cleanup();

    const int fd();

    myPacket *selected();

    int linux_fd() const
    {
        return _fd;
    }

    static int open_packet_socket(std::string);
    static int set_promiscuous(int, std::string, bool);

    int send_packet(myPacket *p);
    // void kernel_drops(bool &known, int &max_drops) const;

    // int getMaxEntries();
};

#endif