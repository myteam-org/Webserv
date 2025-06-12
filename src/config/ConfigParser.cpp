#include "ConfigParser.hpp"

#include <sys/types.h>

#include <cstddef>
#include <stdexcept>

#include "Config.hpp"
#include "ConfigTokenizer.hpp"
#include "LocationContext.hpp"
#include "ServerContext.hpp"
#include "Token.hpp"
#include "Validator.hpp"

void (ConfigParser::* ConfigParser::func_[FUNC_SIZE])(ServerContext&,
                                                      size_t&) = {
    &ConfigParser::setPort_,      &ConfigParser::setHost_,
    &ConfigParser::setErrPage_,   &ConfigParser::setMaxBodySize_,
    &ConfigParser::addLocation_,  &ConfigParser::setRoot_,
    &ConfigParser::setMethod_,    &ConfigParser::setIndex_,
    &ConfigParser::setAutoIndex_, &ConfigParser::setIsCgi_,
    &ConfigParser::setRedirect_};

ConfigParser::ConfigParser(ConfigTokenizer& tokenizer)
    : tokens_(tokenizer.getTokens()), depth_(0) {
        makeVectorServer_();
}

ConfigParser::~ConfigParser() {}

void ConfigParser::makeVectorServer_() {
        for (size_t i = 0; i < this->tokens_.size(); ++i) {
                const TokenType type = this->tokens_[i].getType();
                const std::string token = this->tokens_[i].getText();
                const int lineNumber = this->tokens_[i].getLineNumber();

                if (type == BRACE) {
                        updateDepth(token, lineNumber);
                        if (this->depth_ == 0) {
                                return;
                        }
                } else if (type == SERVER) {
                        i++;
                        if (i == this->tokens_.size()) {
                                throwErr(this->tokens_[i].getText(),
                                         ": Syntax error: line",
                                         tokens_[i].getLineNumber());
                        }
                        addServer_(i);
                } else {
                        continue;
                }
        }
}

void ConfigParser::updateDepth(const std::string& token, int lineNumber) {
        if (token == "{") {
                this->depth_++;
        } else if (token == "}") {
                this->depth_--;
                if (this->depth_ < 0) {
                        throwErr(token, ": Config brace close error: line",
                                 lineNumber);
                }
        }
}

void ConfigParser::addServer_(size_t& index) {
        // NOLINTNEXTLINE(misc-const-correctness)
        ServerContext server("server");

        for (; index < this->tokens_.size(); ++(index)) {
                const std::string text = this->tokens_[index].getText();
                const int type = this->tokens_[index].getType();
                const int lineNum = this->tokens_[index].getLineNumber();
                if (type == BRACE) {
                        updateDepth(text, lineNum);
                        if (this->depth_ == 0) { 
                                break;
                        }
                } else if (type >= LISTEN && type <= LOCATION) {
                        (this->*func_[type])(server, index);
                } else {
                        continue;
                }
        }
        this->servers_.push_back(server);
}

void ConfigParser::setPort_(ServerContext& server, size_t& index) {
        const std::string portNumber = incrementAndCheckSize_(index);

        if (this->tokens_[index].getType() == VALUE &&
            Validator::number(portNumber, LISTEN)) {
                server.setListen(
                    static_cast<u_int16_t>(atoi(portNumber.c_str())));

        } else {
                throwErr(portNumber, ": port value error: line",
                         this->tokens_[index].getLineNumber());
        }
}

void ConfigParser::setHost_(ServerContext& server, size_t& index) {
        const std::string hostName = incrementAndCheckSize_(index);

        if (this->tokens_[index].getType() == VALUE) {
                server.setHost(hostName);
        } else {
                throwErr(hostName, ": Host value error: line",
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
                server.addMap(atoi(errNumber.c_str()), pageName);
        } else {
                throwErr(errNumber, ": ErrorPage value error: line",
                         this->tokens_[index].getLineNumber());
        }
}

void ConfigParser::addLocation_(ServerContext& server, size_t& index) {
        // NOLINTNEXTLINE(misc-const-correctness)
        LocationContext location("location");
        const std::string path = incrementAndCheckSize_(index);

        location.setPath(path);
        server.getLocation().push_back(location);
        for (; index < this->tokens_.size(); ++(index)) {
                const std::string text = this->tokens_[index].getText();
                const int type = this->tokens_[index].getType();
                const int lineNum = this->tokens_[index].getLineNumber();
                if (type == BRACE) {
                        updateDepth(text, lineNum);
                        if (this->depth_ == 1) { 
                                break;
                        }
                } else if (type >= ROOT && type <= REDIRECT) {
                        (this->*func_[type])(server, index);
                } else {
                        continue;
                }
        }
}

std::vector<LocationContext>::iterator ConfigParser::getLocationLastNode_(
    ServerContext& server, size_t& index) {
        std::vector<LocationContext>& locations = server.getLocation();

        if (locations.empty()) {
                throwErr(this->tokens_[index].getText(),
                         ": No location node: line ",
                         this->tokens_[index].getLineNumber());
        }
        const std::vector<LocationContext>::iterator last = locations.end() - 1;
        return (last);
}

void ConfigParser::setRoot_(ServerContext& server, size_t& index) {
        const std::string root = incrementAndCheckSize_(index);
        const std::vector<LocationContext>::iterator last =
            getLocationLastNode_(server, index);

        if (this->tokens_[index].getType() == VALUE) {
                last->setRoot(root);
        } else {
                throwErr(this->tokens_[index].getText(),
                         ": Root value error: line ",
                         this->tokens_[index].getLineNumber());
        }
}

void ConfigParser::setMethod_(ServerContext& server, size_t& index) {
        const std::string method = incrementAndCheckSize_(index);

        for (; index < this->tokens_.size(); ++index) {
                const std::vector<LocationContext>::iterator last =
                    getLocationLastNode_(server, index);
                const TokenType type = this->tokens_[index].getType();

                if (type == VALUE && method == "GET") {
                        last->addMethod(GET);
                } else if (type == VALUE && method == "PUT") {
                        last->addMethod(PUT);
                } else if (type == VALUE && method == "DELETE") {
                        last->addMethod(DELETE);
                } else if (type == BRACE ||
                           (type >= ROOT && type < +REDIRECT)) {
                        return;
                } else {
                        throwErr(method, ": Method value error: line ",
                                 this->tokens_[index].getLineNumber());
                }
        }
}

void ConfigParser::setIndex_(ServerContext& server, size_t& index) {
        const std::string indexPage = incrementAndCheckSize_(index);
        const std::vector<LocationContext>::iterator last =
            getLocationLastNode_(server, index);

        if (this->tokens_[index].getType() == VALUE) {
                last->setIndex(indexPage);
        } else {
                throwErr(this->tokens_[index].getText(),
                         ": Index page value error: line ",
                         this->tokens_[index].getLineNumber());
        }
}

void ConfigParser::setAutoIndex_(ServerContext& server, size_t& index) {
        const std::string select = incrementAndCheckSize_(index);
        const std::vector<LocationContext>::iterator last =
            getLocationLastNode_(server, index);

        if (this->tokens_[index].getType() == VALUE) {
                if (select == "ON") {
                        last->setAutoIndex(ON);
                } else if (select == "OFF") {
                        last->setAutoIndex(OFF);
                } else {
                        throwErr(select, ": Unknown select error: line ",
                                 this->tokens_[index].getLineNumber());
                }
        }
}

void ConfigParser::setIsCgi_(ServerContext& server, size_t& index) {
        const std::string select = incrementAndCheckSize_(index);
        const std::vector<LocationContext>::iterator last =
            getLocationLastNode_(server, index);

        if (this->tokens_[index].getType() == VALUE) {
                if (select == "ON") {
                        last->setIsCgi(ON);
                } else if (select == "OFF") {
                        last->setIsCgi(OFF);
                } else {
                        throwErr(select, ": Unknown select error: line ",
                                 this->tokens_[index].getLineNumber());
                }
        }
}

void ConfigParser::setRedirect_(ServerContext& server, size_t& index) {
        const std::string redirect = incrementAndCheckSize_(index);
        const std::vector<LocationContext>::iterator last =
            getLocationLastNode_(server, index);

        if (this->tokens_[index].getType() == VALUE) {
                last->setRedirect(redirect);
        } else {
                throwErr(this->tokens_[index].getText(),
                         ": Redirect value error: line ",
                         this->tokens_[index].getLineNumber());
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

const std::vector<ServerContext>& ConfigParser::getServer() const {
        return (this->servers_);
}

void ConfigParser::throwErr(const std::string& str1, const std::string& str2,
                            int lineNumber) {
        const std::string num = ConfigTokenizer::numberToStr(lineNumber);
        throw(std::runtime_error(str1 + str2 + num));
}
