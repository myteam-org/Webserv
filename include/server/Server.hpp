#include <serverContext.hpp>
#include "config"
class Server {
   public:
    Server(const Config &serverConfig);
    ~VirtualServer();

    const ServerContext &getServerConfig() const;
    http::Router &getRouter();

   private:
    static void registerHandlers(http::RouterBuilder &routerBuilder,
                                 const LocationContext &locationContext);
    void setupRouter();

    const ServerContext serverConfig_;
    const std::string bindAddress_;
    http::Router *router_;
};
