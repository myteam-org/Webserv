#include "http/handler/router/middleware/error_page.hpp"
#include "utils/string.hpp"
#include <fstream>
#include <sstream>
#include <unistd.h> // getcwd用
#include "http/response/builder.hpp"
#include "utils/logger.hpp" // ログ用追加

namespace http {

ErrorPage::ErrorPage(const ErrorPageMap& errorPageMap)
    : errorPageMap_(errorPageMap) {}

namespace {

// デバッグログ付き：エラーページファイル読み込み
std::string LoadErrorPageBodyFromFile(const std::string& filePath) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd))) {
        LOG_DEBUG(std::string("LoadErrorPageBodyFromFile: cwd = ") + cwd);
    }
    LOG_DEBUG(std::string("LoadErrorPageBodyFromFile: trying to open file: ") + filePath);

    const std::ifstream fileStream(filePath.c_str());
    std::ostringstream contentStream;
    if (fileStream) {
        LOG_DEBUG(std::string("LoadErrorPageBodyFromFile: file opened: ") + filePath);
        contentStream << fileStream.rdbuf();
    } else {
        LOG_DEBUG(std::string("LoadErrorPageBodyFromFile: file open FAILED: ") + filePath);
    }
    std::string body = contentStream.str();
    std::ostringstream oss;
    oss << "LoadErrorPageBodyFromFile: body size read = " << body.size();
    LOG_DEBUG(oss.str());
    return body;
}

const char* const kHtmlHeader =
    "<!DOCTYPE html>\n"
    "<html>\n"
    "<head><title>";

const char* const kHtmlMidHead =
    "</title></head>\n"
    "<body>\n"
    "<center><h1>";

const char* const kHtmlAfterBody =
    "</h1></center>\n"
    "<hr>\n"
    "<center>webserv/0.1.0</center>\n"
    "</body>\n"
    "</html>\n";

// デフォルトエラーページHTML生成
std::string BuildDefaultErrorPageBody(const int statusCode, const std::string& statusText) {
    std::ostringstream statusLineStream;
    statusLineStream << statusCode << " " << statusText;
    const std::string statusLine = statusLineStream.str();

    std::ostringstream htmlStream;
    htmlStream << kHtmlHeader << statusLine << kHtmlMidHead << statusLine << kHtmlAfterBody;
    return htmlStream.str();
}

}  // namespace

Either<IAction*, Response> ErrorPage::intercept(const Request& requestContext, IHandler& nextHandler) {
    LOG_DEBUG("ErrorPage::intercept: called");
    const Either<IAction*, Response> handlerResult = nextHandler.serve(requestContext);
    if (handlerResult.isLeft()) {
        LOG_DEBUG("ErrorPage::intercept: IAction returned, skipping error page handling");
        return handlerResult;
    }

    const Response response = handlerResult.unwrapRight();
    const HttpStatusCode statusCode = response.getStatusCode();
    {
        std::ostringstream oss;
        oss << "ErrorPage::intercept: statusCode=" << static_cast<int>(statusCode);
        LOG_DEBUG(oss.str());
    }
    if (statusCode < kStatusBadRequest) {
        LOG_DEBUG("ErrorPage::intercept: status is not error, returning normal response");
        return Right(response);
    }

    std::string errorPageBody;
    const ErrorPageMap::const_iterator errorPageIterator = errorPageMap_.find(statusCode);
    if (errorPageIterator != errorPageMap_.end()) {
        errorPageBody = LoadErrorPageBodyFromFile(errorPageIterator->second);
        std::ostringstream oss;
        oss << "ErrorPage::intercept: custom error page found for code " << static_cast<int>(statusCode)
            << " (" << errorPageIterator->second << ")";
        LOG_DEBUG(oss.str());
        // ★ここでbodyが空ならデフォルトページを返す★
        if (errorPageBody.empty()) {
            errorPageBody = BuildDefaultErrorPageBody(statusCode, http::getHttpStatusText(statusCode));
            LOG_DEBUG("ErrorPage::intercept: fallback to default error page because file was empty or not found");
        }
    } else {
        errorPageBody = BuildDefaultErrorPageBody(statusCode, http::getHttpStatusText(statusCode));
        std::ostringstream oss;
        oss << "ErrorPage::intercept: default error page generated for code " << static_cast<int>(statusCode);
        LOG_DEBUG(oss.str());
    }

    {
        std::ostringstream oss;
        oss << "ErrorPage::intercept: returning error page (body size=" << static_cast<unsigned long>(errorPageBody.size()) << ")";
        LOG_DEBUG(oss.str());
    }
    return Right(ResponseBuilder().html(errorPageBody, statusCode).build());
}

} // namespace http
