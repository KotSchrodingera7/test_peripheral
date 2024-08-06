#ifndef __CAN_THREAD_H__
#define __CAN_THREAD_H__

#include "canbus.h"
#include "tester_debug.h"

#include <string>
#include <memory>

class CanThread
{
public:
    CanThread(const std::string &name) 
    {
        device_ = std::make_shared<CanBus>();

        if( device_ )
        {
            qInfo(c_can) << name.c_str() << " CAN: device is open";
            device_->open(name);
            name_ = std::move(name);
        }
    };
    ~CanThread() = default;

    void ThreadReceive();

    void SendMessage(const CanFrame &msg);

    CanFrame GetMessage(void)
    {
        return msg_;
    }

private:
    std::shared_ptr<CanBus> device_;

    CanFrame msg_;

    std::string name_;
};

#endif //__CAN_THREAD_H__