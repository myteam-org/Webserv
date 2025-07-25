#include "http/request/read/body.hpp"

#include <gtest/gtest.h>

#include <string>

#include "http/config/config_resolver.hpp"
#include "http/request/read/chunked_body.hpp"
#include "config/context/serverContext.hpp"
#include "http/request/read/context.hpp"
#include "http/request/read/raw_headers.hpp"
#include "io/input/read/buffer.hpp"
#include "io/input/reader/reader.hpp"
#include "state.hpp"
#include "utils/types/error.hpp"

namespace {

class DummyReader : public io::IReader {
   public:
    explicit DummyReader(const std::string& input) : input_(input), pos_(0) {}

    types::Result<std::size_t, error::AppError> read(char* dest,
                                                     std::size_t n) {
        if (pos_ >= input_.size()) return types::ok(0ul);
        std::size_t len = std::min(n, input_.size() - pos_);
        memcpy(dest, input_.data() + pos_, len);
        pos_ += len;
        return types::ok(len);
    }

    bool eof() { return pos_ >= input_.size(); }

   private:
    std::string input_;
    std::size_t pos_;
};

class DummyResolver : public http::config::IConfigResolver {
   public:
    const ServerContext& choseServer(const std::string&) const {
        static ServerContext dummy("server");
        return dummy;
    }
};

}  // namespace

// === TEST 1 ===
// Content-Length が存在する場合、LengthState に切り替える
TEST(ReadingRequestBodyStateTest,
     SwitchesToLengthStateWhenContentLengthPresent) {
    DummyReader reader("Hello");
    ReadBuffer buf(reader);

    const ReadBuffer::LoadResult loadResult = buf.load();
    ASSERT_TRUE(loadResult.isOk()) << "buf.load() failed";

    
    RawHeaders headers;
    headers["Content-Length"] = "5";

    http::BodyLengthConfig cfg = {5, 1024};
    http::ReadingRequestBodyState* state =
        new http::ReadingRequestBodyState(http::kContentLength, cfg);

    DummyResolver resolver;
    http::ReadContext ctx(resolver, state);

    http::HandleResult result = ctx.handle(buf);
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(result.unwrap(), http::IState::kDone);
    EXPECT_EQ(ctx.getBody(), "Hello");
}

// === TEST 2 ===
// Transfer-Encoding: chunked がある場合、ChunkedState
// に切り替わるか（モックが必要なため簡易検証）
TEST(ReadingRequestBodyStateTest,
SwitchesToChunkedStateWhenTransferEncodingPresent) {
    DummyReader reader("5\r\nHello\r\n0\r\n\r\n");
    ReadBuffer buf(reader);
    buf.load();

    RawHeaders headers;
    headers["Transfer-Encoding"] = "chunked";

    http::BodyLengthConfig cfg = {0, 1024};  // Content-Length 無視
    http::ReadingRequestBodyState* state = new
    http::ReadingRequestBodyState(http::kChunked, cfg);

    DummyResolver resolver;
    http::ReadContext ctx(resolver, state);

    http::HandleResult result = types::ok(http::IState::kSuspend);
    do {
        result = ctx.handle(buf);
    } while (result.isOk() && result.unwrap() == http::IState::kSuspend);
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(result.unwrap(), http::IState::kDone);
    EXPECT_EQ(ctx.getBody(), "Hello");
}

// === TEST 3 ===
// 不正なボディ種別を渡した場合のエラー処理（例として kNone を想定）
TEST(ReadingRequestBodyStateTest, ReturnsErrorIfNoValidBodyEncoding) {
    RawHeaders headers;
    headers.insert(std::make_pair(
        "Dummy", "value"));  // Content-Length も Transfer-Encoding もない

    DummyReader reader("");
    ReadBuffer buf(reader);

    DummyResolver resolver;
    http::ReadContext ctx(resolver, NULL);

    // ボディ不要 → NULL が返る
    http::IState* next = ctx.createReadingBodyState(headers).unwrapOr(NULL);
    EXPECT_EQ(next, static_cast<http::IState*>(NULL));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
