#include <iostream>
#include <stdint.h>

class ISocketAddr {
public:
    virtual ~ISocketAddr() {}
    virtual std::string getAddress() const = 0;
    virtual uint16_t getPort() const = 0;
};
