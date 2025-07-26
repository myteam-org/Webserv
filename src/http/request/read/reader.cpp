#include "reader.hpp"
#include "state.hpp"
#include "context.hpp"
#include "line.hpp"
#include "utils/types/result.hpp"
#include "utils/types/option.hpp"
#include "utils/types/error.hpp"
#include "io/input/read/buffer.hpp"
#include "config/config.hpp"
#include "header.hpp"
#include "body.hpp"
#include "utils/types/try.hpp"
#include "http/request/request.hpp"

namespace http {

RequestReader::RequestReader(config::IConfigResolver& resolver)
    : ctx_(resolver, new ReadingRequestLineState()) {
}

// readRequest関数
// 1. バッファ読み込み 読み込みバイト数を保持、失敗時はErrでかえす
// 2. stateを順に進める
//    ・現在のstate(例：リクエストライン読み取り)に処理を委譲
//    ・handle()はTransitionResultを返し、次のstateに自動的に推移する
// 3. エラー処理　state内でエラーが発生したらErrを返す
// 4. サスペンド処理（データ待ち）
//    ・kSuspendは「まだ読み込みが不完全」
//    ・load()で読み込めたバイト数が0->EOF到達->エラー
//    ・読み込み途中->Noneを返す
// 5. 最後に全てのstateが終了(getState()==NULL)していたら、
//    リクエスト生成処理は別で行う前提なので、Noneを返す

RequestReader::ReadRequestResult RequestReader::readRequest(
    ReadBuffer& buf) {

  const std::size_t loaded = TRY(buf.load());

  while (ctx_.getState() != NULL) {
    const types::Result<IState::HandleStatus, error::AppError> handleResult =
        ctx_.handle(buf);

    if (handleResult.isErr()) {
      return types::Result<types::Option<Request>, error::AppError>(
          types::err(handleResult.unwrapErr()));
    }

    if (handleResult.unwrap() == IState::kSuspend) {
      if (loaded == 0) {
        return types::Result<types::Option<Request>, error::AppError>(
            types::err(error::kIOUnknown));
      }
      return types::Result<types::Option<Request>, error::AppError>(
          types::ok(types::none<Request>()));
    }
  }

  return types::Result<types::Option<Request>, error::AppError>(
      types::ok(types::none<Request>()));
}

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
