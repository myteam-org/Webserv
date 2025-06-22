#pragma once

#include <cstddef>

class ISocket {
	virtual ~ISocket();
	virtual int getFD()=0;
}
