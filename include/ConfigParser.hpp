#pragma once

#include <cstddef>
class ConfigTokenizer;
class LocationContext;
class ServerContext;

#include <vector>

#include "ConfigTokenizer.hpp"
#include "Token.hpp"

class ConfigParser {
       public:
        explicit ConfigParser(ConfigTokenizer& tokenizer);
        ~ConfigParser();

        const std::vector<ServerContext>& getServer() const;
        static void throwErr(const std::string& str1, const std::string& str2,
                             int lineNumber);

       private:
        static const int FUNC_SIZE = 11;
        typedef void (ConfigParser::*funcPtr)(ServerContext& server,
                                              size_t& index);
        static funcPtr func_[FUNC_SIZE];

        std::vector<Token> tokens_;
        int depth_;
        std::vector<ServerContext> servers_;

        void makeVectorServer_();
        void updateDepth(const std::string& token, int lineNumber);
        void addServer_(size_t& index);
        void setPort_(ServerContext& server, size_t& index);
        void setHost_(ServerContext& server, size_t& index);
        void setMaxBodySize_(ServerContext& server, size_t& index);
        void setErrPage_(ServerContext& server, size_t& index);
        void addLocation_(ServerContext& server, size_t& index);
        std::vector<LocationContext>::iterator getLocationLastNode_(
            ServerContext& server, size_t& index);
        void setRoot_(ServerContext& server, size_t& index);
        void setMethod_(ServerContext& server, size_t& index);
        void setIndex_(ServerContext& server, size_t& index);
        void setAutoIndex_(ServerContext& server, size_t& index);
        void setIsCgi_(ServerContext& server, size_t& index);
        void setRedirect_(ServerContext& server, size_t& index);
        std::string incrementAndCheckSize_(size_t& index);
};
