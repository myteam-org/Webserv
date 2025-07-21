#include "error_page.hpp"
#include "utils/string.hpp"
#include <fstream>
#include <sstream>

namespace http {

ErrorPage::ErrorPage(const ErrorPageMap& errorPageMap)
    : errorPageMap_(errorPageMap) {}

namespace {

std::string LoadErrorPageBodyFromFile(const std::string& filePath) {
    const std::ifstream fileStream(filePath.c_str());
    std::ostringstream contentStream;
    if (fileStream) {
        contentStream << fileStream.rdbuf();
    }
    return contentStream.str();
}

std::string BuildDefaultErrorPageBody(int statusCode, const std::string& statusText) {
    std::ostringstream statusLineStream;
    statusLineStream << statusCode << " " << statusText;
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
    return htmlBodyStream.str();
}

}  // namespace

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
        errorPageBody = LoadErrorPageBodyFromFile(errorPageIterator->second);
    } else {
        errorPageBody = BuildDefaultErrorPageBody(statusCode, http::getHttpStatusText(statusCode));
    }

    // 実装に応じて以下を有効化
    // return Right(ResponseBuilder().html(errorPageBody, statusCode).build());
    return Right(Response(statusCode));
}

} // namespace http
