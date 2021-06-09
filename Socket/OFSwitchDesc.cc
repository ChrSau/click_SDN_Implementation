#include "OFSwitchDesc.hh"

OFSwitchDesc::OFSwitchDesc()
{
    this->switchDesc = nullptr;
    OFSwitchDesc("SEW-EURODRIVE GmbH & Co KG",
                 "Click Router",
                 "0.0.1",
                 "None",
                 "None");
}

OFSwitchDesc::OFSwitchDesc(std::string mfr_desc, std::string hw_desc, std::string sw_desc, std::string serial_num, std::string dp_desc)
{
    if (this->switchDesc == nullptr || this->switchDesc == NULL)
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

    this->switchDesc->mfr_desc[DESC_STR_LEN - 1] = '\0';
    this->switchDesc->hw_desc[DESC_STR_LEN - 1] = '\0';
    this->switchDesc->sw_desc[DESC_STR_LEN - 1] = '\0';
    this->switchDesc->serial_num[SERIAL_NUM_LEN - 1] = '\0';
    this->switchDesc->dp_desc[DESC_STR_LEN - 1] = '\0';

    // OF::Functions::printHexAndChar((uint8_t*)this->switchDesc, sizeof(struct ofp_desc), "Test: ");
    // OF::Functions::printHex((uint8_t *)this->switchDesc, sizeof(struct ofp_desc), "Switch Desc: ");
}

OFSwitchDesc::~OFSwitchDesc()
{
    if (this->switchDesc != nullptr)
        std::free(this->switchDesc);
    this->switchDesc = nullptr;
}

struct ofp_desc *OFSwitchDesc::getSwitchDesc()
{
    // this->switchDesc = (struct ofp_desc *)std::calloc(1, sizeof(struct  ofp_desc));
    // OF::Functions::printHex((uint8_t *)this->switchDesc, sizeof(struct ofp_desc), "Switch Desc: ");
    return this->switchDesc;
}