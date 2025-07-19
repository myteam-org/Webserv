#include "error_page.hpp"
#include "utils/string.hpp"
#include <fstream>
#include <sstream>

namespace http {

ErrorPage::ErrorPage(const ErrorPageMap& errorPageMap)
    : errorPageMap_(errorPageMap) {}

Either<IAction*, Response> ErrorPage::intercept(const Request& requestContext, IHandler& nextHandler) {
    const Either<IAction*, Response> handlerResult = nextHandler.serve(requestContext);
    if (handlerResult.isLeft()) {
        return handlerResult;
    }

    const Response response = handlerResult.unwrapRight();
    const HttpStatusCode statusCode = response.getStatusCode();
    if (statusCode < kStatusBadRequest) {
        return Right(response);
    }

    std::string errorPageBody;
    const ErrorPageMap::const_iterator errorPageIterator = errorPageMap_.find(statusCode);
    if (errorPageIterator != errorPageMap_.end()) {
        std::ifstream errorFileStream(errorPageIterator->second.c_str());
        std::ostringstream fileContentStream;
        fileContentStream << errorFileStream.rdbuf();
        errorPageBody = fileContentStream.str();
    } else {
        std::ostringstream statusLineStream;
        statusLineStream << statusCode << " " << getHttpStatusText(statusCode);
        const std::string statusLineString = statusLineStream.str();

        std::ostringstream htmlBodyStream;
        htmlBodyStream
            << "<!DOCTYPE html>\n"
            << "<html>\n"
            << "<head><title>" << statusLineString << "</title></head>\n"
            << "<body>\n"
            << "<center><h1>" << statusLineString << "</h1></center>\n"
            << "<hr>\n"
            << "<center>webserv/0.1.0</center>\n"
            << "</body>\n"
            << "</html>\n";
        errorPageBody = htmlBodyStream.str();
    }

    // return Right(ResponseBuilder().html(errorPageBody, statusCode).build());
}

} // namespace http
