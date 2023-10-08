
#include "spi_test.h"

#include <iostream>
#include <vector>

#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include <numeric>

SpiTest::~SpiTest()
{
    close(fd_);
}

int SpiTest::TestTransfer()
{
    int ret;
    uint32_t scratch32;
    struct spi_ioc_transfer trx;
    std::vector<uint8_t> tx_buffer(32), rx_buffer(32, 0xff);
    trx.tx_buf = (unsigned long)tx_buffer.data();
    trx.rx_buf = (unsigned long)rx_buffer.data();
    trx.bits_per_word = 0;
    trx.speed_hz = 1000000;
    trx.delay_usecs = 0;
    trx.len = 32;

    std::iota(tx_buffer.begin(), tx_buffer.end(), 1);

    fd_ = open(dev_.c_str(), O_RDWR);
    if(fd_ < 0) {
        std::cout << "Could not open the SPI device...\r\n" << std::endl;
        return -1;
    }

    ret = ioctl(fd_, SPI_IOC_RD_MODE32, &scratch32);
    if(ret != 0) {
        std::cout << "Could not read SPI mode...\r\n" << std::endl;
        close(fd_);
        return -1;
    }

    scratch32 |= SPI_MODE_0;

    ret = ioctl(fd_, SPI_IOC_WR_MODE32, &scratch32);
    if(ret != 0) {
        std::cout << "Could not write SPI mode...\r\n" << std::endl;
        close(fd_);
        return -1;
    }

    ret = ioctl(fd_, SPI_IOC_RD_MAX_SPEED_HZ, &scratch32);
    if(ret != 0) {
        std::cout << "Could not read the SPI max speed...\r\n" << std::endl;
        close(fd_);
        return -1;
    }

    scratch32 = 5000000;

    ret = ioctl(fd_, SPI_IOC_WR_MAX_SPEED_HZ, &scratch32);
    if(ret != 0) {
        std::cout << "Could not write the SPI max speed...\r\n" << std::endl;
        close(fd_);
        return -1;
    }

    ret = ioctl(fd_, SPI_IOC_MESSAGE(1), &trx);
    if(ret != 0) {
        std::cout << "SPI transfer returned ...\r\n" << ret << std::endl;
    }

    if( rx_buffer == tx_buffer ) 
    {
        std::cout << "Spi test: tx == rx" << std::endl;
        return 0;
    }

    return -1;
}