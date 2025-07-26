#pragma once

#include "utils.hpp"
#include "context.hpp"

namespace http {

  namespace config {
    class IConfigResolver;
  }
class Request;

class RequestReader {
 public:
  explicit RequestReader(config::IConfigResolver& resolver);

  typedef types::Result<types::Option<Request>, error::AppError>
      ReadRequestResult;

  ReadRequestResult readRequest(ReadBuffer& buf);

 private:
  ReadContext ctx_;
};

}  // namespace http

// HTTPリクエストを段階的に読み取るためのクラスRequestReaderの実装
// 状態ごとに分割されたstateパターンを使ってReadBufferから順次リクエストを解析していく
// クラスの目的：ReadBufferを使ってHTTPリクエスト全体を読み取る
// stateマシン（ReadContext）を使って、リクエストライン->ヘッダー->ボディと段階的に進む
// 結果はResult<Option<Request>, AppError>として返す
//  ・成功+データ：Ok(Some(Request))
//  ・未完了：Ok(None)
//  ・エラー：Err(AppError)
// メンバ変数：ReadContext ctx_
//  ・stateマシンの実体
//  ・最初はReadingRequestLineStateから開始
//  ・stateを切り替えながらbufを読み進める
