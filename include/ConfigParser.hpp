#pragma once

class Token;

#include <sys/stat.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "Token.hpp"

#define FILE_NAME "./config_file/default.conf"

class ConfigParser {
 public:
  explicit ConfigParser(std::string& filename);
  ~ConfigParser();

  const std::vector<Token>& getTokens() const;

 private:
  void tokenize_(std::string& filename);
  std::vector<Token> tokens_;
};
