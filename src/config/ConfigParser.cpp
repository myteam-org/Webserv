#include "ConfigParser.hpp"

#include <cstddef>
#include <stdexcept>

#include "ConfigTokenizer.hpp"
#include "ServerContext.hpp"
#include "Token.hpp"
#include "Validator.hpp"

ConfigParser::ConfigParser(ConfigTokenizer &tokenizer)
    : tokens(tokenizer.getTokens()), depth_(0) {
        this->func_[0] = &ConfigParser::setPort_;
        this->func_[1] = &ConfigParser::setHost_;
        this->func_[2] = &ConfigParser::setErrPage_;
        this->func_[3] = &ConfigParser::setMaxBodySize_;
        // func_[4] = &ConfigParser::setPort_;
        makeVectorServer_();
}

ConfigParser::~ConfigParser() {}

void ConfigParser::makeVectorServer_() {
        for (size_t i = 0; i < this->tokens.size(); ++i) {
                TokenType type = this->tokens[i].getType();
                const std::string token = this->tokens[i].getText();
                int lineNumber = this->tokens[i].getLineNumber();

                if (type == BRACE) {
                        updateDepth(token, lineNumber);

                } else if (type == SERVER) {
                        i++;
                        if (i == this->tokens.size())
                                throwErr(this->tokens[i].getText(),
                                         ": Syntax error: line",
                                         tokens[i].getLineNumber());
                        addServer_(&i);
                } else {
                        continue;
                }
        }
}

void ConfigParser::updateDepth(const std::string &token, int lineNumber) {
        std::string num = ConfigTokenizer::numberToStr(lineNumber);
        if (token == "{") {
                this->depth_++;
        } else if (token == "}") {
                this->depth_--;
                if (this->depth_ < 0)
                        throwErr(token, ": Config brace close error: line",
                                 lineNumber);
        }
}

void ConfigParser::addServer_(size_t *i) {
        ServerContext current("server");

        for (; *i < this->tokens.size(); ++(*i)) {
                const std::string text = this->tokens[*i].getText();
                int type = this->tokens[*i].getType();
                int lineNum = this->tokens[*i].getLineNumber();
                if (type == BRACE) {
                        updateDepth(text, lineNum);
                        if (this->depth_ == 0) break;
                } else if (type >= LISTEN && type <= MAX_SIZE) {
                        std::cout << "type = " << type << std::endl;
                        (this->*func_[type])(current, i);
                } else {
                        continue;
                }
        }
        this->servers_.push_back(current);
}

void ConfigParser::setPort_(ServerContext &current, size_t *i) {
        std::string portNumber = incrementAndCheckSize_(i);

        if (this->tokens[*i].getType() == VALUE &&
            Validator::number(portNumber, LISTEN) == true)
                current.setListen((u_int16_t)atoi(portNumber.c_str()));
        else
                throwErr(portNumber, ": Port value error: line",
                         this->tokens[*i].getLineNumber());
}

void ConfigParser::setHost_(ServerContext &current, size_t *i) {
        std::string hostName = incrementAndCheckSize_(i);
        if (this->tokens[*i].getType() == VALUE)
                current.setHost(hostName);
        else
                throwErr(hostName, ": Host value error: line",
                         this->tokens[*i].getLineNumber());
}

void ConfigParser::setMaxBodySize_(ServerContext &current, size_t *i) {
        std::string maxBodySize = incrementAndCheckSize_(i);

        if (this->tokens[*i].getType() == VALUE &&
            Validator::number(maxBodySize, MAX_SIZE) == true)
                current.setClientMaxBodySize((size_t)atoi(maxBodySize.c_str()));
        else
                throwErr(maxBodySize, ": Port value error: line",
                         this->tokens[*i].getLineNumber());
}

void ConfigParser::setErrPage_(ServerContext &current, size_t *i) {
        std::string errNumber = incrementAndCheckSize_(i);

        if (this->tokens[*i].getType() == VALUE &&
            Validator::number(errNumber, ERR_PAGE) == true) {
                std::string pageName = incrementAndCheckSize_(i);
                current.addMap(atoi(errNumber.c_str()), pageName);
        } else {
                throwErr(errNumber, ": ErrorPage value error: line",
                         this->tokens[*i].getLineNumber());
        }
}

std::string ConfigParser::incrementAndCheckSize_(size_t *i) {
        (*i)++;
        if (*i == this->tokens.size())
                throwErr("", "Syntax error: line",
                         this->tokens[*i].getLineNumber());
        return (this->tokens[*i].getText());
}

const std::vector<ServerContext> &ConfigParser::getServr() const {
        return (this->servers_);
}

void ConfigParser::throwErr(const std::string &str1, const std::string &str2,
                            int lineNumber) {
        std::string num = ConfigTokenizer::numberToStr(lineNumber);
        throw(std::runtime_error(str1 + str2 + num));
}
