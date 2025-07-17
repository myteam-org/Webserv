#include "context.hpp"
#include "header.hpp"
#include "header_parseing_utils.hpp"
#include "reader.hpp"
#include "config/context/serverContext.hpp"
#include "http/config/config_resolver.hpp"

namespace http {

ReadContext::ReadContext(config::IConfigResolver& resolver, IState* initial)
    : state_(initial),
      resolver_(resolver)
      {}

ReadContext::~ReadContext() {
  delete state_;
}

// handle関数
// 1. 現在のstate_->handle(buf)を呼んで、処理を進める
// 2. 戻ってきたTransitionReultから
//   ・requestLine/headers/bodyの情報があれば保存
//   ・次のstateがあれば切り替える（古いstateはdelete）
// 3. stateの状態（kDoneなど）を返す

HandleResult ReadContext::handle(ReadBuffer& buf) {
  if (state_ == NULL) {
    return types::ok(IState::kDone);
  }

  const TransitionResult tr = state_->handle(buf);

  if (tr.getRequestLine().isSome()) {
    requestLine_ = tr.getRequestLine().unwrap();
  }
  if (tr.getHeaders().isSome()) {
    headers_ = tr.getHeaders().unwrap();
  }
  if (tr.getBody().isSome()) {
    body_ = tr.getBody().unwrap();
  }
  if (tr.getStatus().unwrap() == IState::kDone) {
    if (dynamic_cast<ReadingRequestHeadersState*>(state_) != NULL) {
      const std::string host = parser::extractHost(headers_);
      const ServerContext& config = resolver_.choseServer(host);
      maxBodySize_ = config.getClientMaxBodySize();
    }
  }

  if (tr.getNextState() != NULL) {
    delete state_;
    state_ = tr.getNextState();
  }

  return tr.getStatus();
}

// state切り替え機能
// 外部から明示的にstateを切り替えたいときに使う

void ReadContext::changeState(IState* next) {
  delete state_;
  state_ = next;
}

const IState* ReadContext::getState() const { return state_; }

config::IConfigResolver& ReadContext::getConfigResolver() const {
  return resolver_;
}

const std::string& ReadContext::getRequestLine() const {
  return requestLine_;
}

const RawHeaders& ReadContext::getHeaders() const { return headers_; }

const std::string& ReadContext::getBody() const { return body_; }

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
