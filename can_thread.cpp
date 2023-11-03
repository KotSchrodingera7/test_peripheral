
#include "can_thread.h"


void CanThread::ThreadReceive()
{
    if( device_ )
    {
        CanBusStatus status = device_->recv(msg_, 1);

        if( status == CanBusStatus::STATUS_OK )
        {
            qCInfo(c_can) << name_.c_str() << " THREAD read byte with size of " << msg_.data.size();
        }
    }
}


void CanThread::SendMessage(const CanFrame &msg)
{
    if( device_ )
    {
        device_->send(msg);
    }
}