#include <gtest/gtest.h>

#include "config/context/serverContext.hpp"
#include "http/config/config_resolver.hpp"
#include "http/request/read/chunked_body.hpp"
#include "http/request/read/context.hpp"
#include "http/request/read/length_body.hpp"
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

TEST(ReadingRequestBodyChunkedStateTest, ReadsChunkedBodyFully) {
    DummyReader reader("5\r\nHello\r\n0\r\n\r\n");
    ReadBuffer buf(reader);
    const ReadBuffer::LoadResult loadResult = buf.load();
    ASSERT_TRUE(loadResult.isOk()) << "buf.load() failed";

    http::ReadingRequestBodyChunkedState* state = new http::ReadingRequestBodyChunkedState();
    DummyResolver resolver;
    http::ReadContext ctx(resolver, state);

    http::HandleResult result = types::ok(http::IState::kSuspend);
    do {
        result = ctx.handle(buf);
        ASSERT_TRUE(result.isOk()) << "handle() failed: "
                                   << static_cast<int>(result.unwrapErr()) << std::endl;
    } while (result.unwrap() == http::IState::kSuspend);

    EXPECT_EQ(result.unwrap(), http::IState::kDone);
    EXPECT_EQ(ctx.getBody(), "Hello");
}


class DummyContext : public http::ReadContext {
   public:
    DummyContext(http::config::IConfigResolver& r, http::IState* s)
        : http::ReadContext(r, s) {}
};

// 追加先: test/http/request/read/body_chunked_test.cpp

TEST(ReadingRequestBodyChunkedStateTest, ParsesChunkedBodyCorrectly) {
    DummyReader reader("5\r\nHello\r\n5\r\nWorld\r\n0\r\n\r\n");
    ReadBuffer buf(reader);
    buf.load();

    http::BodyLengthConfig config = {0, 1024};
    http::ReadingRequestBodyChunkedState state;

    DummyResolver resolver;
    DummyContext ctx(resolver, NULL);

    http::TransitionResult result;

    result = state.handle(ctx, buf);  // kChunkReadSize
    EXPECT_EQ(result.getStatus().unwrap(), http::IState::kSuspend);

    result = state.handle(ctx, buf);  // kChunkReadData
    EXPECT_EQ(result.getStatus().unwrap(), http::IState::kSuspend);

    result = state.handle(ctx, buf);  // kChunkReadSize
    EXPECT_EQ(result.getStatus().unwrap(), http::IState::kSuspend);

    result = state.handle(ctx, buf);  // kChunkReadData
    EXPECT_EQ(result.getStatus().unwrap(), http::IState::kSuspend);

    result = state.handle(ctx, buf);  // kChunkReadSize (0)
    EXPECT_EQ(result.getStatus().unwrap(), http::IState::kSuspend);

    result = state.handle(ctx, buf);  // kChunkReadTrailer
    EXPECT_EQ(result.getStatus().unwrap(), http::IState::kSuspend);

    result = state.handle(ctx, buf);  // kChunkDone
    EXPECT_EQ(result.getStatus().unwrap(), http::IState::kDone);
    EXPECT_EQ(result.getBody().unwrap(), "HelloWorld");
}

TEST(ReadingRequestBodyChunkedStateTest, EndsImmediatelyWithZeroChunk) {
    DummyReader reader("0\r\n\r\n");
    ReadBuffer buf(reader);
    buf.load();

    http::ReadingRequestBodyChunkedState state;
    DummyResolver resolver;
    DummyContext ctx(resolver, NULL);

    http::TransitionResult result;
    result = state.handle(ctx, buf);  // kChunkReadSize → 0
    EXPECT_EQ(result.getStatus().unwrap(), http::IState::kSuspend);

    result = state.handle(ctx, buf);  // kChunkReadTrailer
    EXPECT_EQ(result.getStatus().unwrap(), http::IState::kSuspend);

    result = state.handle(ctx, buf);  // kChunkDone
    EXPECT_EQ(result.getStatus().unwrap(), http::IState::kDone);
    EXPECT_EQ(result.getBody().unwrap(), "");
}

TEST(ReadingRequestBodyChunkedStateTest, FailsOnInvalidChunkSize) {
    DummyReader reader("G\r\nHello\r\n");  // Gは16進数ではない
    ReadBuffer buf(reader);
    buf.load();

    http::ReadingRequestBodyChunkedState state;
    DummyResolver resolver;
    DummyContext ctx(resolver, NULL);

    http::TransitionResult result = state.handle(ctx, buf);
    EXPECT_TRUE(result.getStatus().isErr());
    EXPECT_EQ(result.getStatus().unwrapErr(), error::kBadRequest);
}

TEST(ReadingRequestBodyChunkedStateTest, SuspendsWhenChunkBodyIsIncomplete) {
    DummyReader reader("5\r\nHel");  // 5バイト要求だが 3バイトしかない
    ReadBuffer buf(reader);
    buf.load();

    http::ReadingRequestBodyChunkedState state;
    DummyResolver resolver;
    DummyContext ctx(resolver, NULL);

    http::TransitionResult result;
    result = state.handle(ctx, buf);  // kChunkReadSize
    EXPECT_EQ(result.getStatus().unwrap(), http::IState::kSuspend);

    result = state.handle(ctx, buf);  // kChunkReadData → buf 不足なので suspend
    EXPECT_EQ(result.getStatus().unwrap(), http::IState::kSuspend);
}

TEST(ReadingRequestBodyChunkedStateTest, SuspendsIfTrailerMissingEmptyLine) {
    DummyReader reader("0\r\nSome-Header: val\r\n");  // \r\nが最後にない
    ReadBuffer buf(reader);
    buf.load();

    http::ReadingRequestBodyChunkedState state;
    DummyResolver resolver;
    DummyContext ctx(resolver, NULL);

    http::TransitionResult result;
    result = state.handle(ctx, buf);  // kChunkReadSize
    EXPECT_EQ(result.getStatus().unwrap(), http::IState::kSuspend);

    result = state.handle(
        ctx, buf);  // kChunkReadTrailer → \r\n 足りないので suspend
    EXPECT_EQ(result.getStatus().unwrap(), http::IState::kSuspend);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
