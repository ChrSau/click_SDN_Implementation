#ifndef CLICK_OFPORTDESC__HH
#define CLICK_OFPORTDESC__HH 1

// std include
#include <string>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

// own include
#include "../Utilities/openflow.h"

// click include
#include <click/config.h>

CLICK_DECLS

class OFPortDesc
{
private:
    struct ofp_port *portData;

public:
    OFPortDesc();
    OFPortDesc(uint32_t port_no, uint8_t *hardAddr, std::string name, uint32_t config, uint32_t state, uint16_t type, uint32_t curr, uint32_t advertised, uint32_t supported, uint32_t peer, uint32_t curr_speed, uint32_t max_speed);
    ~OFPortDesc();

    struct ofp_port *getPortData();

    bool set_port_no(uint32_t);
    bool set_hardAddr(uint8_t *);
    bool set_name(std::string);
    bool set_name(char*);
    bool set_config(uint32_t);
    bool set_state(uint32_t);
    bool set_type(uint16_t);
    bool set_curr(uint32_t);
    bool set_advertised(uint32_t);
    bool set_supported(uint32_t);
    bool set_peer(uint32_t);
    bool set_curr_speed(uint32_t);
    bool set_max_speed(uint32_t);
};

CLICK_ENDDECLS
#endif