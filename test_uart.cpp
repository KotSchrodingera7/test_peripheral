
#include <stdio.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

#include <time.h>

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <chrono>
#include <ctime>


#include "test_uart.h"


void Uart::run(void)
{
    std::string data;

    if( Read(data) == 0 ) 
    {
        std::cout << "Read from " << dev_ << " " << data << std::endl;
    }
}

int Uart::Open(int port)
{
    dev_ = std::string("/dev/ttyS").append(std::to_string(port));
    fd_ = open(dev_.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);

    if( fd_ == -1 )
    {
        return - 1;
    }

    struct termios setting;

    cfmakeraw(&setting);
	tcgetattr(fd_, &setting);
	cfsetispeed(&setting, B9600);
	cfsetospeed(&setting, B9600);
    cfmakeraw(&setting);
	tcflush(fd_, TCIFLUSH);
	tcsetattr(fd_, TCSANOW, &setting);

    return 0;
}

int Uart::Write(const std::string &data)
{
    if (fd_ > 0)
    {
        int res = write(fd_, data.data(), data.size());
        if (res != data.size())
        {
            return -1;
        }
        return 0;
    }

    return -1;
    
}

int Uart::Read(std::string &data)
{
    if (fd_ > 0)
    {
        fd_set rfds;
        struct timeval tv;
        int retval;

        FD_ZERO(&rfds);
        FD_SET(fd_, &rfds);

        tv.tv_sec = 0;
        tv.tv_usec = 50000;
        retval = select(fd_ + 1, &rfds, NULL, NULL, &tv);

        if (retval)
        {
            data.resize(64);
            auto t_start = std::chrono::high_resolution_clock::now();

            bool wait_start_byte_ = true;
            while (std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - t_start).count() < 40)
            {
                int count = read(fd_, &data[0], data.size());
                data.resize(count);
                std::cout << "Read " << dev_ << " " << "with size of " << data.size() << std::endl;
                return 0;

            }
        }

        return -1;
    }
    return -1;
}

int Uart::SetRTS(int level)
{
	int status;

	if (fd_ < 0) {
		std::cout << "Invalid File descriptor" << std::endl;
		return -1;
	}

	if (ioctl(fd_, TIOCMGET, &status) == -1) {
		std::cout << "set_RTS(): TIOCMGET" << std::endl;
		return -1;
	}

	if (level) 
    {
		status |= TIOCM_RTS;
    }
	else 
    {
		status &= ~TIOCM_RTS;
    }

	if (ioctl(fd_, TIOCMSET, &status) == -1) {
		std::cout << "set_RTS(): TIOCMSET" << std::endl;
		return -1;
	}
	return 0;
}

int Uart::GetCTS()
{
    int status;

    if (ioctl(fd_, TIOCMGET, &status) == -1) 
    {
		std::cout << "set_RTS(): TIOCMGET" << std::endl;
		return -1;
	}

    if (status & TIOCM_CTS)
    {
		return 1;
    }
	else
	{
        return 0;
    }

    return -1;
}
