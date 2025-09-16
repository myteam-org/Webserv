#pragma once
// c++98

#include "http/request/read/context.hpp"
#include "http/request/parse/request_parser.hpp"
#include "utils/types/result.hpp"
#include "utils/types/option.hpp"
#include "utils/types/error.hpp"

namespace http {
namespace config { class IConfigResolver; }
class Request;

/*
 * RequestReader
 *  - I/O(load)は呼び出し元で済ませ、このクラスはバッファ内データを
 *    state machine で進めながら必要に応じパース。
 *  - 戻り値:
 *      Ok(None)         -> 追加データ待ち
 *      Ok(Some(Request)) -> 1件完成
 *      Err(AppError)    -> パース/仕様エラー等
 */
class RequestReader {
public:
    explicit RequestReader(config::IConfigResolver& resolver);
    ~RequestReader();

    typedef types::Result<types::Option<Request>, error::AppError> ReadRequestResult;

    ReadRequestResult readRequest(ReadBuffer& buf);

private:
    types::Result<types::Unit, error::AppError> ensureParserInited_();
    types::Result<types::Unit, error::AppError> parseHeadersIfNeeded_();
    types::Result<types::Unit, error::AppError> parseBodyIfNeeded_();
    types::Result<Request, error::AppError>     buildIfReady_();
    void destroyParser_();

    ReadContext ctx_;
    // 値ではなくポインタを Option で保持（unwrap はコピーにならない）
    types::Option<parse::RequestParser*> parser_;
    bool headersParsed_;
    bool bodyParsed_;
};

} // namespace http
