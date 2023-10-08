
#include "can_thread.h"


void CanThread::ThreadReceive()
{
    if( device_ )
    {
        CanBusStatus status = device_->recv(msg_, 1);

        if( status == CanBusStatus::STATUS_OK )
        {
            // std::cout << "CAN THREAD read byte with size of " << msg_.data.size() << " data: " << std::endl;

            for( const uint8_t &byte : msg_.data )
            {
                std::cout << std::hex << (int)(byte) << " "; 
            }

            std::cout << std::endl;
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