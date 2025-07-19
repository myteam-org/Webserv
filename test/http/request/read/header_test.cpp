#include <gtest/gtest.h>
#include <string>
#include <cstring>
#include "io/input/read/buffer.hpp"
#include "io/input/reader/reader.hpp"
#include "http/request/read/header.hpp"
#include "utils/types/result.hpp"
#include "utils/types/option.hpp"
#include "utils/types/error.hpp"

namespace {

class DummyReader : public io::IReader {
public:
    explicit DummyReader(const std::string& input)
        : inputData_(input), position_(0) {}

    virtual types::Result<std::size_t, error::AppError> read(char* destination, std::size_t byteCount) {
        if (position_ >= inputData_.size()) {
            return types::ok(0ul);
        }
        std::size_t copyLength = std::min(byteCount, inputData_.size() - position_);
        memcpy(destination, inputData_.data() + position_, copyLength);
        position_ += copyLength;
        return types::ok(copyLength);
    }

    virtual bool eof() {
        return position_ >= inputData_.size();
    }

private:
    std::string inputData_;
    std::size_t position_;
};

class DummyResolver : public http::config::IConfigResolver {
public:
    const ServerContext& choseServer(const std::string&) const {
        static ServerContext dummy("server");
        return dummy;
    }
};


}  // namespace

TEST(ReadingRequestHeadersStateTest, EmptyBufferReturnsSuspend) {
    DummyReader dummyReader("");
    ReadBuffer readBuffer(dummyReader);
    http::ReadingRequestHeadersState state;
    DummyResolver resolver;
    http::ReadContext ctx(resolver, &state);  // ← ctx を定義！
    http::TransitionResult result = state.handle(ctx, readBuffer);

    ASSERT_TRUE(result.getStatus().isOk());
    EXPECT_EQ(result.getStatus().unwrap(), http::IState::kSuspend);
    EXPECT_TRUE(result.getHeaders().isNone());
}

TEST(ReadingRequestHeadersStateTest, InvalidHeaderReturnsError) {
    DummyReader dummyReader("InvalidHeader\r\n\r\n");
    ReadBuffer readBuffer(dummyReader);
    http::ReadingRequestHeadersState state;
    DummyResolver resolver;
    http::ReadContext ctx(resolver, &state);  // ← ctx を定義！
    http::TransitionResult result = state.handle(ctx, readBuffer);

    ASSERT_TRUE(result.getStatus().isErr());
    EXPECT_EQ(result.getStatus().unwrapErr(), error::kIOUnknown);
    EXPECT_TRUE(result.getHeaders().isNone());
}

TEST(ReadingRequestHeadersStateTest, ValidHeadersReturnsDone) {
    DummyReader dummyReader("Host: localhost\r\nUser-Agent: curl\r\n\r\n");
    ReadBuffer readBuffer(dummyReader);
    http::ReadingRequestHeadersState state;
    DummyResolver resolver;
    http::ReadContext ctx(resolver, &state);  // ← ctx を定義！
    http::TransitionResult result = state.handle(ctx, readBuffer);

    ASSERT_TRUE(result.getStatus().isOk());
    EXPECT_EQ(result.getStatus().unwrap(), http::IState::kDone);

    ASSERT_TRUE(result.getHeaders().isSome());
    const RawHeaders& headers = result.getHeaders().unwrap();

    ASSERT_EQ(headers.size(), 2);
    EXPECT_EQ(headers.find("Host")->second, " localhost");
    EXPECT_EQ(headers.find("User-Agent")->second, " curl");
}

TEST(ReadingRequestHeadersStateTest, HeaderWithoutColonReturnsError) {
    DummyReader dummyReader("Host localhost\r\n\r\n");
    ReadBuffer readBuffer(dummyReader);
    http::ReadingRequestHeadersState state;
    DummyResolver resolver;
    http::ReadContext ctx(resolver, &state);  // ← ctx を定義！
    http::TransitionResult result = state.handle(ctx, readBuffer);

    ASSERT_TRUE(result.getStatus().isErr());
    EXPECT_TRUE(result.getHeaders().isNone());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
