#ifndef __CAN_THREAD_H__
#define __CAN_THREAD_H__

#include "canbus.h"

#include <string>
#include <memory>

class CanThread
{
    std::shared_ptr<CanBus> device_;

    CanFrame msg_;
public:
    CanThread(const std::string &name) 
    {
        device_ = std::make_shared<CanBus>();

        if( device_ )
        {
            std::cout << "CAN: device is open with name of " << name << std::endl;
            device_->open(name);
        }
    };
    ~CanThread() = default;

    void ThreadReceive();

    void SendMessage(const CanFrame &msg);

    CanFrame GetMessage(void)
    {
        return msg_;
    }
};

#endif //__CAN_THREAD_H__