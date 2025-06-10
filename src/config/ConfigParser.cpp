#include "ConfigParser.hpp"
#include <sys/types.h>

#include <cstddef>
#include <stdexcept>

#include "ConfigTokenizer.hpp"
#include "ServerContext.hpp"
#include "Token.hpp"
#include "Validator.hpp"

ConfigParser::ConfigParser(ConfigTokenizer &tokenizer)
    : tokens_(tokenizer.getTokens()), depth_(0) {
        this->func_[0] = &ConfigParser::setPort_;
        this->func_[1] = &ConfigParser::setHost_;
        this->func_[2] = &ConfigParser::setErrPage_;
        this->func_[3] = &ConfigParser::setMaxBodySize_;
        // func_[4] = &ConfigParser::setPort_;
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

void ConfigParser::updateDepth(const std::string &token, int lineNumber) {
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
        ServerContext current("server");

        for (; index < this->tokens_.size(); ++(index)) {
                const std::string text = this->tokens_[index].getText();
                const int type = this->tokens_[index].getType();
                const int lineNum = this->tokens_[index].getLineNumber();
                if (type == BRACE) {
                        updateDepth(text, lineNum);
                        if (this->depth_ == 0) break;
                } else if (type >= LISTEN && type <= MAX_SIZE) {
                        (this->*func_[type])(current, index);
                } else {
                        continue;
                }
        }
        this->servers_.push_back(current);
}

void ConfigParser::setPort_(ServerContext &current, size_t& index) {
        const std::string portNumber = incrementAndCheckSize_(index);

        if (this->tokens_[index].getType() == VALUE &&
            Validator::number(portNumber, LISTEN)) {
                    current.setListen(static_cast<u_int16_t>(atoi(portNumber.c_str())));

        } else {
                throwErr(portNumber, ": Port value error: line",
                         this->tokens_[index].getLineNumber());
        }
}

void ConfigParser::setHost_(ServerContext &current, size_t& index) {
        const std::string hostName = incrementAndCheckSize_(index);
        if (this->tokens_[index].getType() == VALUE) {
                current.setHost(hostName);
        } else {
                throwErr(hostName, ": Host value error: line",
                         this->tokens_[index].getLineNumber());
        }
}

void ConfigParser::setMaxBodySize_(ServerContext &current, size_t& index) {
        const std::string maxBodySize = incrementAndCheckSize_(index);

        if (this->tokens_[index].getType() == VALUE &&
            Validator::number(maxBodySize, MAX_SIZE)) {
                    current.setClientMaxBodySize(static_cast<size_t>(atoi(maxBodySize.c_str())));
        } else {
                throwErr(maxBodySize, ": Port value error: line",
                         this->tokens_[index].getLineNumber());
        }
}

void ConfigParser::setErrPage_(ServerContext &current, size_t& index) {
        const std::string errNumber = incrementAndCheckSize_(index);

        if (this->tokens_[index].getType() == VALUE &&
            Validator::number(errNumber, ERR_PAGE)) {
                const std::string pageName = incrementAndCheckSize_(index);
                current.addMap(atoi(errNumber.c_str()), pageName);
        } else {
                throwErr(errNumber, ": ErrorPage value error: line",
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

const std::vector<ServerContext> &ConfigParser::getServr() const {
        return (this->servers_);
}

void ConfigParser::throwErr(const std::string &str1, const std::string &str2,
                            int lineNumber) {
        const std::string num = ConfigTokenizer::numberToStr(lineNumber);
        throw(std::runtime_error(str1 + str2 + num));
}
