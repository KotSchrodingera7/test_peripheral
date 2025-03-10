#ifndef __CANBUS_H__
#define __CANBUS_H__

#include <iostream>
#include <string>
#include <vector>

#include <unistd.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <linux/can.h>
#include <linux/can/raw.h>
#include <cstdint>

struct CanFrame
{
	uint32_t can_id;
	uint8_t can_dlc;
	uint8_t flags;
	// uint8_t data[64];
	std::vector<uint8_t> data;
};

enum class CanBusStatus : uint8_t 
{
	STATUS_OK = 1,
	STATUS_SOCKET_CREATE_ERROR = 1 << 1,
	STATUS_INTERFACE_NAME_TO_IDX_ERROR = 1 << 2,
	STATUS_BIND_ERROR = 1 << 3,
	STATUS_WRITE_ERROR = 1 << 4,
	STATUS_READ_ERROR = 1 << 5,
	STATUS_READ_TIMEOUT = 1 << 6
};

class CanBus
{
public:
	CanBus();
	~CanBus() {
		if( can_fd_ >= 0 )
		{
			close(can_fd_);
		}
	};

	CanBusStatus open(const std::string &can_name = "can0");
	CanBusStatus send(const CanFrame &msg);
	CanBusStatus recv(CanFrame &msg, int timeout = 0);
	int canStatus = 0;

private:
	int can_fd_;
	int nbytes_;
	struct ifreq ifr_; // for saving config of interface
	struct sockaddr_can addr_;
	std::string name_;
};

#endif // __CANBUS_H__