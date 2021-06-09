#ifndef CLICK_OFSWITCHDESC__HH
#define CLICK_OFSWITCHDESC__HH
#include <string>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <cstring>
#include "../openflow.h"
#include "../OFHelper.hh"

class OFSwitchDesc
{
private:
    struct ofp_desc *switchDesc = nullptr;

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

#endif