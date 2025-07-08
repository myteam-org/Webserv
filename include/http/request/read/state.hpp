#pragma once

#include "utils/types/result.hpp"
#include "utils/types/option.hpp"
#include "utils/types/error.hpp"
#include "buffer.hpp"
#include "utils.hpp"
#include "http/request/read/raw_headers.hpp"

namespace http {

// IState(インターフェース)
// ・HTTPリクエストの「現在の読み取り状態」を表すインターフェース
// ・handle()を実装することで、そのstateでの処理内容を定義できる
// ・処理結果としてTransitionResultを返す
// ・状態の例：
//   ・ReadingRequestLineState
//   ・ReadingHeadersState
//   ・ReadingBodyState

class IState {
 public:
  enum HandleStatus { kSuspend, kDone };
  virtual ~IState() {}
  virtual struct TransitionResult handle(ReadBuffer& buf) = 0;
};

// TransitionResult
// 1回目のhandle()呼び出しで得られる「状態遷移の結果」

class TransitionResult {
 private:
  IState* nextState_;                       // 次の状態(nullptrなら終了)
  types::Option<std::string> requestLine_;  // リクエストラインが取得できたか
  types::Option<RawHeaders> headers_;       // ヘッダーが取得できたか
  types::Option<std::string> body_;         // ボディが取得できたか
  types::Result<IState::HandleStatus, error::AppError> status_; // 状態：続ける/完了/エラー

 public:
  TransitionResult()
      : nextState_(NULL),
        requestLine_(types::none<std::string>()),
        headers_(types::none<RawHeaders>()),
        body_(types::none<std::string>()),
        status_(types::ok(IState::kSuspend)) {} // 一時停止（データ待ち）

  IState* getNextState() const { return nextState_; }
  void setNextState(IState* nextState) { nextState_ = nextState; }

  const types::Option<std::string>& getRequestLine() const { return requestLine_; }
  void setRequestLine(const types::Option<std::string>& requestLine) {
    requestLine_ = requestLine;
  }

  const types::Option<RawHeaders>& getHeaders() const { return headers_; }
  void setHeaders(const types::Option<RawHeaders>& headers) {
    headers_ = headers;
  }

  const types::Option<std::string>& getBody() const { return body_; }
  void setBody(const types::Option<std::string>& body) { body_ = body; }

  const types::Result<IState::HandleStatus, error::AppError>& getStatus() const {
    return status_;
  }
  void setStatus(
      const types::Result<IState::HandleStatus, error::AppError>& status) {
    status_ = status;
  }
};

typedef types::Result<IState::HandleStatus, error::AppError> HandleResult;

}  // namespace http

// HTTPリクエストの読み取り処理におけるstate(状態)遷移の仕組みを定義する
// 状態ごとに異なる解析処理を行い、次の状態に進める「stateマシンパターン」を実装する
