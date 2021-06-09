#ifndef OF_PACKET_HH
#define OF_PACKET_HH 1

#include <cstdlib>
#include <cstring>

namespace OF
{

    /**
 * This Packet Class could be used to store Data. At the moment, it is used just in the OFQueue Class for testing purposes.
 */
    class Packet
    {
    private:
        uint8_t *_data{};
        uint32_t _length{};

    public:
        constexpr Packet(const uint8_t *const data, const uint32_t &length) noexcept
        {
            if (data != nullptr)
            {
                this->_data = static_cast<uint8_t *>(std::malloc(length));
                std::memcpy(this->_data, data, length);
                this->_length = length;
            }
        }

        constexpr Packet(const Packet &in) noexcept
        {
            if (in.data() != nullptr)
            {
                this->_length = in.length();
                this->_data = static_cast<uint8_t *>(std::malloc(this->_length));
                std::memcpy(this->_data, in.data(), this->_length);
            }
        }

        constexpr Packet(const Packet *const in) noexcept
        {
            if (in->data() != nullptr)
            {
                this->_length = in->length();
                this->_data = static_cast<uint8_t *>(std::malloc(this->_length));
                std::memcpy(this->_data, in->data(), this->_length);
            }
        }

        ~Packet()
        {
            std::free(this->_data);
            this->_data = nullptr;
        }

        static Packet *make(const uint8_t *const data, const uint32_t &length) noexcept
        {
            return new Packet(data, length);
        }

        constexpr const uint8_t *data() const noexcept { return this->_data; }
        constexpr const uint32_t &length() const noexcept { return this->_length; }
    };
} // namespace OF

#endif