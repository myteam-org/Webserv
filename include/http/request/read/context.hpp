#pragma once

#include "state.hpp"
#include "http/request/read/raw_headers.hpp"

namespace http {

class IConfigResolver;

class ReadContext {
 public:
  ReadContext(IConfigResolver& resolver, IState* initial);
  ~ReadContext();

  HandleResult handle(ReadBuffer& buf);
  void changeState(IState* next);

  const IState* getState() const;
  IConfigResolver& getConfigResolver() const;
  const std::string& getRequestLine() const;
  const RawHeaders& getHeaders() const;
  const std::string& getBody() const;

 private:
  IState* state_;
  IConfigResolver& resolver_;
  std::string requestLine_;
  RawHeaders headers_;
  std::string body_;
};

}  // namespace http

