#pragma once

#include <memory>

class Data
{
private:
    struct impl;
    std::unique_ptr<impl> m;

    Data() noexcept;
    Data(const Data &) = delete;
    Data(Data &&) = delete;
    Data &operator=(const Data &) = delete;
    Data &operator=(Data &&) = delete;

public:
    ~Data();

    static Data &getIntance() noexcept;

    uint8_t &getOFVersion() const noexcept;
    Data &setVersion(const uint8_t &_in) noexcept;
};
