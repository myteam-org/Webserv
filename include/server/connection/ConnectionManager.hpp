
#include "ConnectionSocket.hpp"
#include "Connection.hpp"

class ConnectionManager {
public:
    types::Result<const Connection&, std::string> getConnectionByFd(FileDescriptor& fd)const;
    types::Result<int, int> registerConnection(Connection& conn);
    types::Result<int, int> unregisterConnection(int fd);
    ConnectionManager();
    ~ConnectionManager();

private:
    std::map<int, Connection*> connectionMap_;
    ConnectionManager (const ConnectionManager&);
    ConnectionManager& operator=(const ConnectionManager&);
};
