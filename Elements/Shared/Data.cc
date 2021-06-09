#include "Data.hh"

struct Data::impl
{
    uint8_t ofVersion = 0;
};

Data::Data() noexcept
    : m{std::make_unique<impl>()}
{
}

Data::~Data() {}

Data &Data::getIntance() noexcept
{
    static Data data;
    return data;
}

uint8_t &Data::getOFVersion() const noexcept
{
    return m->ofVersion;
}

Data &Data::setVersion(const uint8_t &_in) noexcept
{
    m->ofVersion = _in;
    return *this;
}
