#ifndef __SPI_TEST_H__
#define __SPI_TEST_H__

#include <string>
#include <cstdint>

class SpiTest
{
    std::string dev_;

    int fd_;
    uint32_t mode;
    uint32_t speed{500000};
    uint8_t bits{8};
public:
    SpiTest(const std::string &name_dev) : dev_(name_dev)  {};
    ~SpiTest();

    int TestTransfer();
};

#endif //__SPI_TEST_H__