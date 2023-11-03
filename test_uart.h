
#ifndef __TEST_UART_H__
#define __TEST_UART_H__

#include <string>

class Uart 
{
    int fd_;
    std::string dev_;

public:

    Uart() {}
    ~Uart() {
        close(fd_);
    }
    int Open(int port);

    int Write(const std::string &data);
    int Read(std::string &data);
    int SetRTS(int level);
    int GetCTS();

    std::string GetDev() const 
    {
        return dev_;
    };

    void run(void);
};

#endif //__TEST_UART_H__