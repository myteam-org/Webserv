
#include <gtest/gtest.h>
#include <string>
#include <cstring>

#include "io/input/read/buffer.hpp"
#include "io/input/reader/reader.hpp"
#include "http/request/read/header.hpp"
#include "utils/types/result.hpp"
#include "utils/types/option.hpp"
#include "utils/types/error.hpp"
#include "http/request/read/context.hpp"
#include "http/config/config_resolver.hpp"
#include "config/context/serverContext.hpp"

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
    types::Result<const ServerContext*, error::AppError>
    chooseServer(const std::string&) const {
        static ServerContext dummy("server");
        return types::ok<const ServerContext*>(&dummy);  // ポインタを返す
    }
};

}  // namespace

TEST(ReadingRequestHeadersStateTest, ReturnsSuspendWhenBufferIsEmpty) {
  DummyReader dummyReader("");
  ReadBuffer readBuffer(dummyReader);
  http::ReadingRequestHeadersState* state = new http::ReadingRequestHeadersState();
  DummyResolver resolver;
  http::ReadContext ctx(resolver, state);

  http::TransitionResult transitionResult = state->handle(ctx, readBuffer);

  ASSERT_TRUE(transitionResult.getStatus().isOk());
  EXPECT_EQ(transitionResult.getStatus().unwrap(), http::IState::kSuspend);
  EXPECT_TRUE(transitionResult.getHeaders().isNone());
}

TEST(ReadingRequestHeadersStateTest, ReturnsErrorWhenHeaderIsMalformed) {
  DummyReader dummyReader("InvalidHeaderLineWithoutColon\r\n");
  ReadBuffer readBuffer(dummyReader);
  http::ReadingRequestHeadersState* state = new http::ReadingRequestHeadersState();
  DummyResolver resolver;
  http::ReadContext ctx(resolver, state);

  http::TransitionResult transitionResult = state->handle(ctx, readBuffer);

  ASSERT_TRUE(transitionResult.getStatus().isErr());
  EXPECT_EQ(transitionResult.getStatus().unwrapErr(), error::kBadRequest);
}

TEST(ReadingRequestHeadersStateTest, ReturnsDoneWhenHeadersEndWithCRLF) {
  DummyReader dummyReader("Host: localhost\r\nUser-Agent: Test\r\n\r\n");
  ReadBuffer readBuffer(dummyReader);
  http::ReadingRequestHeadersState* state = new http::ReadingRequestHeadersState();
  DummyResolver resolver;
  http::ReadContext ctx(resolver, state);

  http::TransitionResult transitionResult = state->handle(ctx, readBuffer);

  ASSERT_TRUE(transitionResult.getStatus().isOk());
  EXPECT_EQ(transitionResult.getStatus().unwrap(), http::IState::kDone);
  EXPECT_FALSE(transitionResult.getHeaders().isNone());

  const RawHeaders& headers = transitionResult.getHeaders().unwrap();
  EXPECT_EQ(headers.find("Host")->second, "localhost");
  EXPECT_EQ(headers.find("User-Agent")->second, "Test");
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
