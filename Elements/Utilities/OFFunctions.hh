#ifndef OF_FUNCTIONS_HH
#define OF_FUNCTIONS_HH 1

#include <iostream>
#include <iomanip>
#include <string>
#include <chrono>

namespace OF
{

/**
 *  The functions in this Class are just for support and are may needed everywhere. 
 */
class Functions
{
public:
    Functions() {}
    ~Functions() {}

    static bool compareArray(uint8_t *in1, uint8_t *in2, uint64_t length)
    {
        for (size_t i = 0; i < length; i++)
        {
            if (in1[i] != in2[i])
                return false;
        }
        return true;
    }

    static std::string getTimeAndDateString()
    {
        auto now = std::chrono::system_clock::now();
        auto timeNow = std::chrono::system_clock::to_time_t(now);

        return std::ctime(&timeNow);
    }

    static uint8_t isPrintable(uint8_t in)
    {
        if (in <= 0x7e && in >= 0x20)
        {
            return in;
        }
        else
        {
            return '.';
        }
    }

    static void printHex(uint8_t *packet, int length, std::string message)
    {
        if (packet == nullptr || packet == NULL)
        {
            std::cerr << "ERROR: Pointer is NULL or nullptr" << std::endl;
        }
        else
        {
            std::cout << std::endl
                      << message << "(" << std::dec << length << " Bytes)" << std::endl;

            for (int j = 0; j < length; j++)
            {
                if (j % 8 == 0 && j > 0)
                    std::cout << "   ";
                if (j % 16 == 0 && j > 0)
                    std::cout << std::endl;
                std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)packet[j] << " ";
            }
            std::cout << std::endl
                      << std::endl;
        }
    }

    static void printHex(uint8_t *packet, int length)
    {
        printHex(packet, length, "");
    }

    static void printChar(uint8_t *data, int length, std::string message)
    {
        if (data == nullptr || data == NULL)
        {
            std::cerr << "Pointer is NULL or nullptr" << std::endl;
        }
        else
        {
            std::cout << std::endl
                      << message << "(" << std::dec << length << " Bytes)" << std::endl;

            for (int i = 0; i < length; i++)
            {
                std::cout << data[i];

                if (i % 30 == 0 && i != 0)
                    std::cout << "\n";
            }
            std::cout << std::endl;
        }
    }

    static void printChar(uint8_t *data, int length)
    {
        printChar(data, length, "");
    }

    static void printHexAndChar(uint8_t *packet, int length, std::string message)
    {
        if (packet == nullptr || packet == NULL)
        {
            std::cerr << "ERROR: Pointer is NULL or nullptr" << std::endl;
        }
        else
        {
            unsigned int linecount = 0;

            std::cout << std::endl
                      << message << "(" << std::dec << length << " Bytes)" << std::endl;

            for (int j = 0; j < length; j++)
            {

                if (j % 8 == 0 && j > 0)
                    std::cout << "   ";
                if (j % 16 == 0 && j > 0)
                {
                    std::cout << "      ";

                    std::cout << "|";
                    for (int i = j - 16; i < j; i++)
                    {
                        if (i % 8 == 0 && i > (j - 16) && i != j)
                            std::cout << " ";
                        std::cout << isPrintable(packet[i]);
                    }
                    std::cout << "|";

                    std::cout << std::endl;
                }

                if (j % 16 == 0)
                    std::cout << std::hex << std::setfill('0') << std::setw(7) << linecount++ << "0   ";

                std::cout << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(packet[j]) << " ";

                if (j == length - 1)
                {
                    if (j % 16 < 8)
                        std::cout << "   ";

                    std::cout << "      ";
                    for (int i = 0; i < 16 - (j % 16); i++)
                    {
                        std::cout << "   ";
                    }

                    std::cout << "|";
                    for (int i = j - (j % 16); i < j + 1; i++)
                    {
                        if (i % 8 == 0 && i > (j - (j % 16)) && i != j)
                            std::cout << " ";
                        std::cout << isPrintable(packet[i]);
                    }
                    std::cout << "|";
                }
            }
            std::cout << std::endl
                      << std::endl;
        }
    }

    static void printHexAndChar(uint8_t *packet, int length)
    {
        printHexAndChar(packet, length, "");
    }

    static void printHexAndChar(uint8_t *packet, int length, const char *string)
    {
        printHexAndChar(packet, length, std::string(string));
    }

    static void printHexAndCharToError(uint8_t *packet, int length, std::string message)
    {
        if (packet == nullptr || packet == NULL)
        {
            std::cerr << "Pointer is NULL or nullptr" << std::endl;
        }
        else
        {
            unsigned int linecount = 0;

            std::cerr << std::endl
                      << message << "(" << std::dec << length << " Bytes)" << std::endl;

            for (int j = 0; j < length; j++)
            {
                if (j % 8 == 0 && j > 0)
                    std::cerr << "   ";
                if (j % 16 == 0 && j > 0)
                {
                    std::cerr << "      ";
                    std::cerr << "|";
                    for (int i = j - 16; i < j; i++)
                    {
                        std::cerr << isPrintable(packet[i]);
                    }
                    std::cerr << "|";

                    std::cerr << std::endl;
                }
                if (j % 16 == 0)
                    std::cerr << std::hex << std::setfill('0') << std::setw(7) << linecount++ << "0   ";

                std::cerr << std::hex << std::setfill('0') << std::setw(2) << (int)packet[j] << " ";

                if (j == length - 1)
                {
                    if (j % 16 < 8)
                        std::cerr << "   ";

                    std::cerr << "      ";
                    for (int i = 0; i < 16 - (j % 16); i++)
                    {
                        std::cerr << "   ";
                    }

                    std::cerr << "|";
                    for (int i = j - (j % 16); i < j + 1; i++)
                    {
                        std::cerr << isPrintable(packet[i]);
                    }
                    std::cerr << "|";
                }
            }
            std::cerr << std::endl
                      << std::endl;
        }
    }

    static void printHexAndCharToError(uint8_t *packet, int length)
    {
        printHexAndCharToError(packet, length, "");
    }

    static void printIp4Addr(uint8_t *packet, std::string message)
    {
        std::cout << message;
        for (int i = 0; i < 4; i++)
        {
            std::cout << std::dec << static_cast<int>(packet[i]);
            if (i < 3)
                std::cout << ".";
        }
        std::cout << std::endl
                  << std::endl;
    }

    static void printIp4Addr(uint8_t *packet)
    {
        printIp4Addr(packet, "");
    }

    static bool compareString(std::string in1, std::string in2)
    {
        std::cout << std::dec << in1 << "\n"
                  << in2 << std::endl;
        for (size_t i = 0; i < (in1.length() <= in2.length() ? in1.length() : in2.length()); i++)
        {
            if (in1.at(i) != in2.at(i))
                return false;
        }
        return true;
    }
};

} // namespace OF

#endif