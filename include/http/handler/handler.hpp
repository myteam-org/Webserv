#include "http/request/request.hpp"
#include "either.hpp"
#include "action.hpp"

namespace http {
    class IHandler {
    public:
        virtual ~IHandler() {}
        virtual Either<IAction *, Response> serve(const Request &request) = 0;
    };
} //namespace http
