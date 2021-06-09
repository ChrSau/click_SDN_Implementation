#ifndef CLICK_OFSWITCHDESC__HH
#define CLICK_OFSWITCHDESC__HH 1

#include <iostream>
#include <string>
#include <cstring>

#include "../Utilities/openflowWrapper.h"

#include <click/config.h>

CLICK_DECLS

class OFSwitchDesc
{
private:
    struct ofp_desc *switchDesc;

public:
    OFSwitchDesc();
    OFSwitchDesc(std::string mfr_desc,   /* Manufacturer description. */
                 std::string hw_desc,    /* Hardware description. */
                 std::string sw_desc,    /* Software description. */
                 std::string serial_num, /* Serial number. */
                 std::string dp_desc     /* Human readable description of datapath. */);
    ~OFSwitchDesc();

    struct ofp_desc *getSwitchDesc();
};

CLICK_ENDDECLS
#endif