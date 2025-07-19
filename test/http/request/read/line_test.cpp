// #include <gtest/gtest.h>
// #include <string>
// #include <cstring>
// #include "io/input/read/buffer.hpp"
// #include "io/input/reader/reader.hpp"
// #include "http/request/read/line.hpp"
// #include "http/request/read/context.hpp"
// #include "http/config/config_resolver.hpp"
// #include "utils/types/result.hpp"
// #include "utils/types/option.hpp"
// #include "utils/types/error.hpp"
// #include "config/context/serverContext.hpp"

// namespace {

// // ダミーの Reader
// class DummyReader : public io::IReader {
// public:
//     explicit DummyReader(const std::string& input)
//         : inputData_(input), position_(0) {}

//     types::Result<std::size_t, error::AppError> read(char* destination, std::size_t byteCount) {
//         if (position_ >= inputData_.size()) {
//             return types::ok(0ul);
//         }
//         std::size_t copyLength = std::min(byteCount, inputData_.size() - position_);
//         memcpy(destination, inputData_.data() + position_, copyLength);
//         position_ += copyLength;
//         return types::ok(copyLength);
//     }

//     bool eof() { return position_ >= inputData_.size(); }

// private:
//     std::string inputData_;
//     std::size_t position_;
// };

// // ダミーの ConfigResolver
// class DummyConfigResolver : public http::config::IConfigResolver {
// public:
//     const ServerContext& choseServer(const std::string&) const {
//         static ServerContext dummy("server");
//         return dummy;
//     }
// };

// } // namespace

// TEST(ReadingRequestLineStateTest, ReturnsSuspendWhenBufferIsEmpty) {
//     DummyReader dummyReader("");
//     ReadBuffer readBuffer(dummyReader);
//     http::ReadingRequestLineState* state = new http::ReadingRequestLineState();
//     DummyConfigResolver resolver;
//     http::ReadContext ctx(resolver, state);

//     http::TransitionResult transitionResult = state->handle(ctx, readBuffer);

//     ASSERT_TRUE(transitionResult.getStatus().isOk());
//     EXPECT_EQ(transitionResult.getStatus().unwrap(), http::IState::kSuspend);
//     EXPECT_TRUE(transitionResult.getRequestLine().isNone());
// }

// TEST(ReadingRequestLineStateTest, ReturnsErrorWhenLineIsEmpty) {
//     DummyReader dummyReader("\r\n");
//     ReadBuffer readBuffer(dummyReader);
//     http::ReadingRequestLineState* state = new http::ReadingRequestLineState();
//     DummyConfigResolver resolver;
//     http::ReadContext ctx(resolver, state);

//     http::TransitionResult transitionResult = state->handle(ctx, readBuffer);

//     ASSERT_TRUE(transitionResult.getStatus().isErr());
//     EXPECT_EQ(transitionResult.getStatus().unwrapErr(), error::kIOUnknown);
//     EXPECT_TRUE(transitionResult.getRequestLine().isNone());
// }

// TEST(ReadingRequestLineStateTest, ReturnsDoneWhenLineIsPresent) {
//     DummyReader dummyReader("GET / HTTP/1.1\r\n");
//     ReadBuffer readBuffer(dummyReader);
//     http::ReadingRequestLineState* state = new http::ReadingRequestLineState();
//     DummyConfigResolver resolver;
//     http::ReadContext ctx(resolver, state);

//     http::TransitionResult transitionResult = state->handle(ctx, readBuffer);

//     ASSERT_TRUE(transitionResult.getStatus().isOk());
//     EXPECT_EQ(transitionResult.getStatus().unwrap(), http::IState::kDone);
//     ASSERT_TRUE(transitionResult.getRequestLine().isSome());
//     EXPECT_EQ(transitionResult.getRequestLine().unwrap(), "GET / HTTP/1.1");
// }

// int main(int argc, char **argv) {
//     ::testing::InitGoogleTest(&argc, argv);
//     return RUN_ALL_TESTS();
// }


#include <gtest/gtest.h>
#include <string>
#include <cstring>
#include "io/input/read/buffer.hpp"
#include "io/input/reader/reader.hpp"
#include "http/request/read/line.hpp"
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
        const std::size_t copyLength = std::min(byteCount, inputData_.size() - position_);
        memcpy(destination, inputData_.data() + position_, copyLength);
        position_ += copyLength;
        return types::ok(copyLength);
    }

    virtual bool eof() { return position_ >= inputData_.size(); }

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


} // namespace

TEST(ReadingRequestLineStateTest, ReturnsSuspendWhenBufferIsEmpty) {
    DummyReader dummyReader("");
    ReadBuffer readBuffer(dummyReader);
    http::ReadingRequestLineState state;
    DummyResolver resolver;
    http::ReadContext ctx(resolver, &state);

    http::TransitionResult transitionResult = state.handle(ctx, readBuffer);

    ASSERT_TRUE(transitionResult.getStatus().isOk());
    EXPECT_EQ(transitionResult.getStatus().unwrap(), http::IState::kSuspend);
    EXPECT_TRUE(transitionResult.getRequestLine().isNone());
}

TEST(ReadingRequestLineStateTest, ReturnsErrorWhenLineIsEmpty) {
    DummyReader dummyReader("\r\n");
    ReadBuffer readBuffer(dummyReader);
    http::ReadingRequestLineState state;
    DummyResolver resolver;
http::ReadContext ctx(resolver, &state);

    http::TransitionResult transitionResult = state.handle(ctx, readBuffer);

    ASSERT_TRUE(transitionResult.getStatus().isErr());
    EXPECT_EQ(transitionResult.getStatus().unwrapErr(), error::kIOUnknown);
    EXPECT_TRUE(transitionResult.getRequestLine().isNone());
}

TEST(ReadingRequestLineStateTest, ReturnsDoneWhenLineIsPresent) {
    DummyReader dummyReader("GET / HTTP/1.1\r\n");
    ReadBuffer readBuffer(dummyReader);
    http::ReadingRequestLineState state;
    DummyResolver resolver;
http::ReadContext ctx(resolver, &state);

    http::TransitionResult transitionResult = state.handle(ctx, readBuffer);

    ASSERT_TRUE(transitionResult.getStatus().isOk());
    EXPECT_EQ(transitionResult.getStatus().unwrap(), http::IState::kDone);
    ASSERT_TRUE(transitionResult.getRequestLine().isSome());
    EXPECT_EQ(transitionResult.getRequestLine().unwrap(), "GET / HTTP/1.1");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
