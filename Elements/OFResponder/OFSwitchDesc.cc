#include "OFSwitchDesc.hh"

CLICK_DECLS

OFSwitchDesc::OFSwitchDesc()
{
    OFSwitchDesc("SEW-EURODRIVE GmbH & Co KG",
                 "Click Router",
                 "0.0.1",
                 "0000000001",
                 "0000000001");
}

OFSwitchDesc::OFSwitchDesc(std::string mfr_desc, std::string hw_desc, std::string sw_desc, std::string serial_num, std::string dp_desc)
{
    this->switchDesc = (ofp_desc *)std::calloc(1, sizeof(struct ofp_desc));

    mfr_desc.resize(DESC_STR_LEN - 1);
    hw_desc.resize(DESC_STR_LEN - 1);
    sw_desc.resize(DESC_STR_LEN - 1);
    serial_num.resize(SERIAL_NUM_LEN - 1);
    dp_desc.resize(DESC_STR_LEN - 1);

    std::memcpy(this->switchDesc->mfr_desc, mfr_desc.c_str(), DESC_STR_LEN);
    std::memcpy(this->switchDesc->hw_desc, hw_desc.c_str(), DESC_STR_LEN);
    std::memcpy(this->switchDesc->sw_desc, sw_desc.c_str(), DESC_STR_LEN);
    std::memcpy(this->switchDesc->serial_num, serial_num.c_str(), SERIAL_NUM_LEN);
    std::memcpy(this->switchDesc->dp_desc, dp_desc.c_str(), DESC_STR_LEN);
}

OFSwitchDesc::~OFSwitchDesc()
{
    std::free(this->switchDesc);
    this->switchDesc = nullptr;
}

struct ofp_desc *OFSwitchDesc::getSwitchDesc()
{
    return this->switchDesc;
}

CLICK_ENDDECLS
ELEMENT_PROVIDES(OFSwitchDesc)