#include <string.h> // strcpy
#include <sys/time.h>
#include <sys/timerfd.h>

#include "canbus.h"
#include "tester_debug.h"

CanBus::CanBus()
{
}

CanBusStatus CanBus::open(const std::string &can_name)
{
    // 1. Create a socket(int domain, int type, int protocol)
    if ((can_fd_ = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
    {
        qCCritical(c_can) << can_name.c_str() << " Error while Opening Socket";
        return CanBusStatus::STATUS_SOCKET_CREATE_ERROR;
    }

    // 2. Retrieve the interface index for the interface name
    //    ex: can0, can1, vcan0 etc
    strcpy(ifr_.ifr_name, can_name.c_str());
    ioctl(can_fd_, SIOCGIFINDEX, &ifr_);

    // 3. Bind the socket to the CAN Interface:
    memset(&addr_, 0, sizeof(addr_));
    addr_.can_family = AF_CAN;
    addr_.can_ifindex = ifr_.ifr_ifindex;

    if (bind(can_fd_, (struct sockaddr *)&addr_, sizeof(addr_)) < 0)
    {
        qCCritical(c_can) << can_name.c_str() << " Error in Socket bind";
        return CanBusStatus::STATUS_BIND_ERROR;
    }

    name_ = std::move(can_name);

    return CanBusStatus::STATUS_OK;
}

CanBusStatus CanBus::send(const CanFrame &msg)
{
    struct canfd_frame frame;

    frame.can_id = msg.can_id;
    frame.len = msg.can_dlc;
    copy(msg.data.begin(), msg.data.end(), frame.data);

    if (write(can_fd_, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame))
    {
        qCCritical(c_can) << name_.c_str()  << "[Error] Write";
        return CanBusStatus::STATUS_WRITE_ERROR;
    }

    return CanBusStatus::STATUS_OK;
}

CanBusStatus CanBus::recv(CanFrame &msg, int timeout)
{
    fd_set fds;
    struct timeval tv;
    struct canfd_frame frame;
    int ret;

    FD_ZERO(&fds);
    FD_SET(can_fd_, &fds);

    tv.tv_sec = timeout;
    tv.tv_usec = 0; // microseconds

    if (timeout != 0)
    {
        ret = select(can_fd_ + 1, &fds, NULL, NULL, &tv);
        if (0 == ret)
        { // timeout
            qCWarning(c_can) << name_.c_str() << "Select timeout";
            return CanBusStatus::STATUS_READ_TIMEOUT;
        }
    }

    nbytes_ = read(can_fd_, &frame, sizeof(struct can_frame));
    if (nbytes_ < 0)
    {
        qCCritical(c_can) << name_.c_str() << " Read Error";
        return CanBusStatus::STATUS_READ_ERROR;
    }

    msg.can_id = frame.can_id;
    msg.can_dlc = frame.len;
    msg.data.insert(msg.data.begin(), std::begin(frame.data), std::begin(frame.data) + frame.len);

    return CanBusStatus::STATUS_OK;
}