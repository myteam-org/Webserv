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
        static const int FUNC_SERVER_SIZE = 5;
        static const int FUNC_LOCATION_SIZE = 6;
        typedef void (ConfigParser::*funcServerPtr)
                                (ServerContext& server, size_t& index);
        static funcServerPtr funcServer_[FUNC_SERVER_SIZE];
        typedef void (ConfigParser::*funcLocationPtr)
                                (LocationContext& location, size_t& index);
        static funcLocationPtr funcLocation_[FUNC_LOCATION_SIZE];

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
        void setRoot_(LocationContext& location, size_t& index);
        void setMethod_(LocationContext& location, size_t& index);
        void setIndex_(LocationContext& location, size_t& index);
        void setAutoIndex_(LocationContext& location, size_t& index);
        void setIsCgi_(LocationContext& location, size_t& index);
        void setRedirect_(LocationContext& location, size_t& index);
        std::string incrementAndCheckSize_(size_t& index);
};
