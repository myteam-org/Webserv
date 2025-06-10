#pragma once

#include <cstddef>
class ConfigNode;
class ConfigTokenizer;

#include <vector>

#include "ConfigTokenizer.hpp"
#include "ServerContext.hpp"
#include "Token.hpp"

class ConfigParser {
       public:
        explicit ConfigParser(ConfigTokenizer& tokenizer);
        ~ConfigParser();

        std::vector<Token> tokens;
        const std::vector<ServerContext>& getServr() const;
        void throwErr(const std::string& str1, const std::string& str2,
                      int lineNumber);

       private:
        typedef void (ConfigParser::*funcPtr)(ServerContext& current, size_t* i);
        funcPtr func_[5];
        int depth_;
        std::vector<ServerContext> servers_;
        void makeVectorServer_();
        void updateDepth(const std::string& token, int lineNumber);
        void addServer_(size_t* i);
        void setPort_(ServerContext& current, size_t* i);
        void setHost_(ServerContext& current, size_t* i);
        void setErrPage_(ServerContext& current, size_t* i);
        void setMaxBodySize_(ServerContext& current, size_t* i);
        std::string incrementAndCheckSize_(size_t* i);
        // void addLocation();

        
        //        public:
        //         explicit ConfigParser(const ConfigTokenizer& tokenizer);
        //         ~ConfigParser();

        //         ConfigTokenizer tokens;
        //         ConfigNode* getRoot() const;
        //         static void deleteTree(ConfigNode* node);
        //         void throwErr(const std::string& str1, const std::string&
        //         str2,
        //                       const int number);

        //        private:
        //         int keyFlag_[16];
        //         ConfigNode* layers_[5];

        //         void makeConfTree_(const ConfigTokenizer& tokens);
        //         void updateDepth_(const std::string& token, const int
        //         lineNumber); void resetKeyFlag_(const int keyType); void
        //         addChild_(const Token& token, ConfigNode*& current,
        //                        ConfigNode* parent);
        //         void setValue_(const Token& token, ConfigNode* node);
};
