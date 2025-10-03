#include "config/parser.hpp"

#include <sys/types.h>

#include <cstddef>
#include <stdexcept>

#include "config/config.hpp"
#include "config/context/locationContext.hpp"
#include "config/context/serverContext.hpp"
#include "config/token.hpp"
#include "config/tokenizer.hpp"
#include "config/validator.hpp"
#include "utils/logger.hpp"
#include "utils/ip.hpp"

void (ConfigParser::* ConfigParser::funcServer_[FUNC_SERVER_SIZE])(
    ServerContext&, size_t&) = {
    &ConfigParser::setPort_,        &ConfigParser::setHost_,
    &ConfigParser::setserverName_,  &ConfigParser::setErrPage_,
    &ConfigParser::setMaxBodySize_, &ConfigParser::addLocation_};

void (ConfigParser::* ConfigParser::funcLocation_[FUNC_LOCATION_SIZE])(
    LocationContext&, size_t&) = {
    &ConfigParser::setRoot_,        &ConfigParser::setMethod_,
    &ConfigParser::setIndex_,       &ConfigParser::setAutoIndex_,
    &ConfigParser::setIsCgi_,       &ConfigParser::setRedirect_,
    &ConfigParser::setEnableUpload_};

ConfigParser::ConfigParser(ConfigTokenizer& tokenizer,
                           const std::string& confFile)
    : tokens_(tokenizer.getTokens()), depth_(0), confFile_(confFile) {
    LOG_INFO("ConfigParser created");
    makeVectorServer_();
}

ConfigParser::~ConfigParser() {
    LOG_INFO("ConfigParser destroyed");
}

void ConfigParser::makeVectorServer_() {
    LOG_INFO("Parsing server configurations...");
    for (size_t i = 0; i < this->tokens_.size(); ++i) {
        const TokenType type = this->tokens_[i].getType();
        const int lineNumber = this->tokens_[i].getLineNumber();
        if (type == BRACE) {
            updateDepth(tokens_[i], lineNumber);
            if (this->depth_ == 0) {
                LOG_INFO("Finished parsing server configurations");
                return;
            }
        } else if (type == SERVER) {
            i++;
            if (this->depth_ != 0 || i == this->tokens_.size()) {
                throwErr(this->tokens_[i].getText(), ": Syntax error: line",
                         tokens_[i].getLineNumber());
            }
            addServer_(i);
        } else if ((type >= LISTEN && type <= ENABLE_UPLOAD) &&
                   this->depth_ == 0) {
            throwErr(this->tokens_[i].getText(), ": Syntax error: line",
                     tokens_[i].getLineNumber());
        } else {
            continue;
        }
    }
}

void ConfigParser::updateDepth(const Token& token, int lineNumber) {
    const std::string text = token.getText();
    if (text == "{" && token.getPosition() == BEGINNING) {
        throwErr(text, ": Config brace close error: line", lineNumber);
    }
    if (text == "{") {
        this->depth_++;
    } else if (text == "}") {
        this->depth_--;
        if (this->depth_ < 0) {
            throwErr(text, ": Config brace close error: line", lineNumber);
        }
    }
}

void ConfigParser::addServer_(size_t& index) {
    LOG_INFO("Adding new server block");
    // NOLINTNEXTLINE(misc-const-correctness)
    ServerContext server("server");

    for (; index < this->tokens_.size(); ++(index)) {
        const int type = this->tokens_[index].getType();
        const int lineNum = this->tokens_[index].getLineNumber();
        if (type == SERVER || (type >= ROOT && type <= REDIRECT)) {
            throwErr(tokens_[index].getText(), ": invalid block member: line",
                     lineNum);
        } else if (type == BRACE) {
            updateDepth(this->tokens_[index], lineNum);
            if (this->depth_ == 0) {
                break;
            }
        } else if (type >= LISTEN && type <= LOCATION) {
            (this->*funcServer_[type])(server, index);
        } else {
            continue;
        }
    }
    this->servers_.push_back(server);
    LOG_INFO("Server block added");
}

void ConfigParser::setPort_(ServerContext& server, size_t& index) {
    const std::string portNumber = incrementAndCheckSize_(index);

    if (this->tokens_[index].getType() == VALUE &&
        Validator::number(portNumber, LISTEN)) {
        if (server.getListen() != 0) {
            throwErr(portNumber,
                     ": Multiple ports on a single virtual server are not "
                     "supported: line",
                     this->tokens_[index].getLineNumber());
        }
        server.setListen(static_cast<u_int16_t>(atoi(portNumber.c_str())));
    } else {
        throwErr(portNumber, ": port value error: line",
                 this->tokens_[index].getLineNumber());
    }
}

void ConfigParser::setHost_(ServerContext& server, size_t& index) {
    const std::string hostName = incrementAndCheckSize_(index);

    if (this->tokens_[index].getType() == VALUE) {
        if (hostName == "localhost" || utils::isCanonicalDecimalIPv4(hostName)) {
            server.setHost(hostName);
        } else {
            throwErr(hostName, ": host value error: line",
                this->tokens_[index].getLineNumber());
        }
    }
}

// 修正: server_names ディレクティブが複数値を持てるように
void ConfigParser::setserverName_(ServerContext& server, size_t& index) {
    // 1つ目
    std::string serverName = incrementAndCheckSize_(index);

    if (this->tokens_[index].getType() == VALUE) {
        server.addServerName(serverName);
        // 追加: 以降の値も全部追加
        while (index + 1 < this->tokens_.size() && this->tokens_[index + 1].getType() == VALUE) {
            ++index;
            serverName = this->tokens_[index].getText();
            server.addServerName(serverName);
        }
    } else {
        throwErr(serverName, ": server_name value error: line",
                 this->tokens_[index].getLineNumber());
    }
}

void ConfigParser::setMaxBodySize_(ServerContext& server, size_t& index) {
    const std::string maxBodySize = incrementAndCheckSize_(index);

    if (this->tokens_[index].getType() == VALUE &&
        Validator::number(maxBodySize, MAX_SIZE)) {
        server.setClientMaxBodySize(
            static_cast<size_t>(atoi(maxBodySize.c_str())));
    } else {
        throwErr(maxBodySize, ": client_max_body_size error: line",
                 this->tokens_[index].getLineNumber());
    }
}

void ConfigParser::setErrPage_(ServerContext& server, size_t& index) {
    const std::string errNumber = incrementAndCheckSize_(index);

    if (this->tokens_[index].getType() == VALUE &&
        Validator::number(errNumber, ERR_PAGE)) {
        const std::string pageName = incrementAndCheckSize_(index);
        const int num = atoi(errNumber.c_str());
        server.addMap(static_cast<http::HttpStatusCode>(num), pageName);
    } else {
        throwErr(errNumber, ": ErrorPage value error: line",
                 this->tokens_[index].getLineNumber());
    }
}

void ConfigParser::addLocation_(ServerContext& server, size_t& index) {
    LOG_INFO("Adding new location block");
    // NOLINTNEXTLINE(misc-const-correctness)
    LocationContext location("location");
    const std::string path = incrementAndCheckSize_(index);

    location.setPath(path);
    for (; index < this->tokens_.size(); ++(index)) {
        const std::string text = this->tokens_[index].getText();
        const int type = this->tokens_[index].getType();
        const int lineNum = this->tokens_[index].getLineNumber();
        if (type == SERVER || type == LOCATION ||
            (type >= LISTEN && type <= MAX_SIZE)) {
            throwErr(text, ": server or location in location error: line",
                     lineNum);
        } else if (type == BRACE) {
            updateDepth(tokens_[index], lineNum);
            if (this->depth_ == 1) {
                setDefaultMethod_(location);
                server.getLocation().push_back(location);
                break;
            }
        } else if (type >= ROOT && type <= ENABLE_UPLOAD) {
            (this->*funcLocation_[type - FUNC_SERVER_SIZE])(location, index);
        } else {
            continue;
        }
    }
    LOG_INFO("Location block added for path: " + path);
}

void ConfigParser::setDefaultMethod_(LocationContext& location) {
    OnOff* method = location.getMutableAllowedMethod();
    if (method[GET] == OFF && method[POST] == OFF && method[DELETE] == OFF) {
        location.setMethod(GET);
        location.setMethod(POST);
        location.setMethod(DELETE);
    }
}

void ConfigParser::setRoot_(LocationContext& location, size_t& index) {
    const std::string root = incrementAndCheckSize_(index);
    DocumentRootConfig& documentRootConfig = location.getDocumentRootConfig();

    if (this->tokens_[index].getType() == VALUE) {
        if (!Validator::isValidRoot(root, confFile_)) {
            throwErr(root, ": Invalid root directory: line ",
                     this->tokens_[index].getLineNumber());
        }
        documentRootConfig.setRoot(root);
    } else {
        throwErr(this->tokens_[index].getText(), ": Root value error: line ",
                 this->tokens_[index].getLineNumber());
    }
}

void ConfigParser::setMethod_(LocationContext& location, size_t& index) {
    incrementAndCheckSize_(index);

    for (; index < this->tokens_.size(); ++index) {
        const std::string method = this->tokens_[index].getText();
        const TokenType type = this->tokens_[index].getType();
        if (type == BRACE || (type >= ROOT && type <= REDIRECT)) {
            index--;
            return;
        }
        if (method == "GET" && location.getMutableAllowedMethod()[GET] == OFF) {
            location.setMethod(GET);
        } else if (method == "POST" &&
                   location.getMutableAllowedMethod()[POST] == OFF) {
            location.setMethod(POST);
        } else if (method == "DELETE" &&
                   location.getMutableAllowedMethod()[DELETE] == OFF) {
            location.setMethod(DELETE);
        } else {
            throwErr(method, ": Method value error: line ",
                     this->tokens_[index].getLineNumber());
        }
    }
}

void ConfigParser::setIndex_(LocationContext& location, size_t& index) {
    const std::string indexPage = incrementAndCheckSize_(index);

    DocumentRootConfig& documentRootConfig = location.getDocumentRootConfig();

    if (this->tokens_[index].getType() == VALUE) {
        if (!Validator::isValidIndexFile(indexPage)) {
            throwErr(indexPage, " : Invalid index file: line ",
                     this->tokens_[index].getLineNumber());
        }
        documentRootConfig.setIndex(indexPage);
    } else {
        throwErr(this->tokens_[index].getText(),
                 ": Index page value error: line ",
                 this->tokens_[index].getLineNumber());
    }
}

void ConfigParser::setAutoIndex_(LocationContext& location, size_t& index) {
    const std::string select = incrementAndCheckSize_(index);
    DocumentRootConfig& documentRootConfig = location.getDocumentRootConfig();

    if (this->tokens_[index].getType() == VALUE) {
        if (select == "ON") {
            documentRootConfig.setAutoIndex(ON);
        } else if (select == "OFF") {
            documentRootConfig.setAutoIndex(OFF);
        } else {
            throwErr(select, ": Unknown select error: line ",
                     this->tokens_[index].getLineNumber());
        }
    }
}

void ConfigParser::setIsCgi_(LocationContext& location, size_t& index) {
    const std::string select = incrementAndCheckSize_(index);
    DocumentRootConfig& documentRootConfig = location.getDocumentRootConfig();

    if (this->tokens_[index].getType() == VALUE) {
        if (select == "ON") {
            documentRootConfig.setCgiExtensions(ON);
        } else if (select == "OFF") {
            documentRootConfig.setCgiExtensions(OFF);
        } else {
            throwErr(select, ": Unknown select error: line ",
                     this->tokens_[index].getLineNumber());
        }
    }
}

void ConfigParser::setRedirect_(LocationContext& location, size_t& index) {
    const std::string redirect = incrementAndCheckSize_(index);

    if (this->tokens_[index].getType() == VALUE) {
        location.setRedirect(redirect);
    } else {
        throwErr(this->tokens_[index].getText(),
                 ": Redirect value error: line ",
                 this->tokens_[index].getLineNumber());
    }
}

void ConfigParser::setEnableUpload_(LocationContext& location, size_t& index) {
    const std::string select = incrementAndCheckSize_(index);
    DocumentRootConfig& documentRootConfig = location.getDocumentRootConfig();

    if (this->tokens_[index].getType() == VALUE) {
        if (select == "ON") {
            documentRootConfig.setEnableUpload(ON);
        } else if (select == "OFF") {
            documentRootConfig.setEnableUpload(OFF);
        } else {
            throwErr(select, ": Unknown select error: line ",
                     this->tokens_[index].getLineNumber());
        }
    }
}

std::string ConfigParser::incrementAndCheckSize_(size_t& index) {
    index++;
    if (index == this->tokens_.size()) {
        throwErr("", "Syntax error: line",
                 this->tokens_[index].getLineNumber());
    }
    return (this->tokens_[index].getText());
}

std::vector<ServerContext>& ConfigParser::getServer() {
    return (this->servers_);
}

const std::vector<ServerContext>& ConfigParser::getServer() const {
    return (this->servers_);
}

void ConfigParser::throwErr(const std::string& str1, const std::string& str2,
                            int lineNumber) {
    const std::string num = ConfigTokenizer::numberToStr(lineNumber);
    throw(std::runtime_error(str1 + str2 + num));
}
