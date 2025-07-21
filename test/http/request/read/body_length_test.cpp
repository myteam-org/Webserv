#include <gtest/gtest.h>
#include <string>
#include <cstring>

#include "http/request/read/length_body.hpp"
#include "io/input/read/buffer.hpp"
#include "io/input/reader/reader.hpp"
#include "utils/types/result.hpp"
#include "utils/types/option.hpp"
#include "utils/types/error.hpp"
#include "http/request/read/context.hpp"

namespace {

class DummyReader : public io::IReader {
 public:
  explicit DummyReader(const std::string& input) : input_(input), pos_(0) {}

  types::Result<std::size_t, error::AppError> read(char* dest, std::size_t nbyte) {
    if (pos_ >= input_.size()) return types::ok(0ul);
    std::size_t len = std::min(nbyte, input_.size() - pos_);
    memcpy(dest, input_.data() + pos_, len);
    pos_ += len;
    return types::ok(len);
  }

  bool eof() { return pos_ >= input_.size(); }

 private:
  std::string input_;
  std::size_t pos_;
};

class DummyContext : public http::ReadContext {
 public:
  DummyContext(http::config::IConfigResolver& r, http::IState* s) : http::ReadContext(r, s) {}
};

class DummyResolver : public http::config::IConfigResolver {
 public:
  const ServerContext& choseServer(const std::string&) const {
    static ServerContext dummy("server");
    return dummy;
  }
};

}  // namespace

TEST(ReadingRequestBodyLengthStateTest, ReturnsErrorIfBodyTooLarge) {
  DummyReader reader("HelloWorld");
  ReadBuffer buf(reader);
  http::BodyLengthConfig config = {10, 5};  // contentLength > clientMaxBodySize
  http::ReadingRequestBodyLengthState state(config);

  DummyResolver resolver;
  DummyContext ctx(resolver, NULL);

  auto result = state.handle(ctx, buf);
  EXPECT_TRUE(result.getStatus().isErr());
  EXPECT_EQ(result.getStatus().unwrapErr(), error::kRequestEntityTooLarge);
}

TEST(ReadingRequestBodyLengthStateTest, ReturnsSuspendIfBufferEmpty) {
  DummyReader reader("");
  ReadBuffer buf(reader);
  buf.load();
  http::BodyLengthConfig config = {5, 10};
  http::ReadingRequestBodyLengthState state(config);

  DummyResolver resolver;
  DummyContext ctx(resolver, NULL);

  auto result = state.handle(ctx, buf);
  EXPECT_TRUE(result.getStatus().isOk());
  EXPECT_EQ(result.getStatus().unwrap(), http::IState::kSuspend);
}

TEST(ReadingRequestBodyLengthStateTest, ReturnsDoneIfBodyFullyRead) {
  DummyReader reader("Hello");
  ReadBuffer buf(reader);
  buf.load();
  http::BodyLengthConfig config = {5, 10};
  http::ReadingRequestBodyLengthState state(config);

  DummyResolver resolver;
  DummyContext ctx(resolver, NULL);

  auto result = state.handle(ctx, buf);
  EXPECT_TRUE(result.getStatus().isOk());
  EXPECT_EQ(result.getStatus().unwrap(), http::IState::kDone);
  EXPECT_EQ(result.getBody().unwrap(), "Hello");
}

TEST(ReadingRequestBodyLengthStateTest, ReturnsSuspendIfPartiallyRead) {
    // 最初の5バイトのみ与える
    DummyReader reader("Hello");
    ReadBuffer buf(reader);
    http::BodyLengthConfig config = {10, 1024};
    http::ReadingRequestBodyLengthState state(config);

    DummyResolver resolver;
    DummyContext ctx(resolver, NULL);

    buf.load();  // "Hello"を読み込む
    auto result1 = state.handle(ctx, buf);

    EXPECT_TRUE(result1.getStatus().isOk());
    EXPECT_EQ(result1.getStatus().unwrap(), http::IState::kSuspend);
    EXPECT_TRUE(result1.getBody().isNone());

    // リーダーを切り替えて次のデータ "World" を渡す
    DummyReader reader2("World");
    ReadBuffer buf2(reader2);  // 新しいリーダーで新バッファ
    buf2.load();

    // ここで読み取り状態を引き継いで再実行するには、state のインスタンスをそのまま使う必要あり
    auto result2 = state.handle(ctx, buf2);

    EXPECT_TRUE(result2.getStatus().isOk());
    EXPECT_EQ(result2.getStatus().unwrap(), http::IState::kDone);
    EXPECT_TRUE(result2.getBody().isSome());
    EXPECT_EQ(result2.getBody().unwrap(), "HelloWorld");
}

#include <gtest/gtest.h>

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
