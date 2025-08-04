#pragma once

#include "http/request/read/state.hpp"
#include "http/request/read/raw_headers.hpp"
#include "http/config/config_resolver.hpp"

namespace http {

class IState;

class ReadContext {
 public:
  ReadContext(config::IConfigResolver& resolver, IState* initial);
  ~ReadContext();

  HandleResult handle(ReadBuffer& buf);
  void changeState(IState* next);

  const IState* getState() const;
  config::IConfigResolver& getConfigResolver() const;
  void setRequestLine(const std::string& line);
  const std::string& getRequestLine() const;
  void setHeaders(const RawHeaders& headers);
  const RawHeaders& getHeaders() const;
  void setBody(const std::string& body);
  const std::string& getBody() const;
  types::Option<IState*> createReadingBodyState(const RawHeaders& headers) const;
  void setServer(const ServerContext& server);
  const ServerContext& getServer() const;
  bool hasServer() const;

 private:
  IState* state_;
  http::config::IConfigResolver& resolver_;
  std::string requestLine_;
  RawHeaders headers_;
  std::string body_;
  size_t maxBodySize_ ;
  const ServerContext* server_;
};

}  // namespace http

// HTTPリクエストの読み取り処理におけるstateマシンの実行環境
// 現在のstate(IState)を持ち、stateの進行に合わせて状態やデータ
// （リクエストライン・ヘッダー・ボディ）を更新する役割を持つ
// クラス
//  ReadContext:状態と読み込み処理を管理するstateマシンのコンテキスト（状態保持）
//  IState:各段階（リクエストライン、ヘッダ、ボディなど）の処理単位（Stageパターン）
//  IConfigResolver:コンフィグ情報を提供する依存対象
//  ReadBuffer:ソケットなどからの読み込みバッファ
//  RawHeaders:HTTPヘッダー情報
