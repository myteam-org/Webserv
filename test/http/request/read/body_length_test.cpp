#include <gtest/gtest.h>
#include <string>
#include <cstring>

#include "http/request/read/length_body.hpp"
#include "io/input/read/buffer.hpp"
#include "io/input/reader/reader.hpp"
#include "utils/types/result.hpp"
#include "utils/types/option.hpp"
#include "utils/types/error.hpp"

namespace {

// 普通のメモリベースReader
class DummyReader : public io::IReader {
public:
    explicit DummyReader(const std::string& input)
        : inputData_(input), position_(0) {}

    types::Result<std::size_t, error::AppError> read(char* dest, std::size_t nbyte) {
        if (position_ >= inputData_.size()) {
            return types::ok(0ul);
        }
        const std::size_t copyLen = std::min(nbyte, inputData_.size() - position_);
        memcpy(dest, inputData_.data() + position_, copyLen);
        position_ += copyLen;
        return types::ok(copyLen);
    }

    bool eof() {
        return position_ >= inputData_.size();
    }

private:
    std::string inputData_;
    std::size_t position_;
};

// 2段階でデータを返すReader（"He", "llo"）
class MultiStageReader : public io::IReader {
public:
    MultiStageReader() : stage_(0), position_(0) {}

    types::Result<std::size_t, error::AppError> read(char* dest, std::size_t nbyte) {
        const std::vector<std::string> stages = { "He", "llo" };
        if (stage_ >= stages.size()) {
            return types::ok(0ul);
        }

        const std::string& current = stages[stage_];
        std::size_t copyLen = std::min(nbyte, current.size() - position_);
        memcpy(dest, current.data() + position_, copyLen);
        position_ += copyLen;

        if (position_ >= current.size()) {
            stage_++;
            position_ = 0;
        }

        return types::ok(copyLen);
    }

    bool eof() {
        return stage_ >= 2;
    }

private:
    std::size_t stage_;
    std::size_t position_;
};

// 読み込み失敗を強制するReader
class FailingReader : public io::IReader {
public:
    types::Result<std::size_t, error::AppError> read(char*, std::size_t) {
        return types::err(error::kIOUnknown);
    }

    bool eof() {
        return false;
    }
};

}  // namespace

// ====== テスト本体 ======

TEST(ReadingRequestBodyLengthStateTest, ReturnsSuspendWhenNoDataAvailable) {
    DummyReader dummyReader("");
    ReadBuffer readBuffer(dummyReader);
    http::ReadingRequestBodyLengthState state(5);

    http::TransitionResult result = state.handle(readBuffer);

    ASSERT_TRUE(result.getStatus().isOk());
    EXPECT_EQ(result.getStatus().unwrap(), http::IState::kSuspend);
    EXPECT_TRUE(result.getBody().isNone());
}

TEST(ReadingRequestBodyLengthStateTest, ReturnsSuspendIfPartialDataAvailable) {
    DummyReader dummyReader("He");
    ReadBuffer readBuffer(dummyReader);
    readBuffer.load();

    http::ReadingRequestBodyLengthState state(5);
    http::TransitionResult result = state.handle(readBuffer);

    ASSERT_TRUE(result.getStatus().isOk());
    EXPECT_EQ(result.getStatus().unwrap(), http::IState::kSuspend);
    EXPECT_TRUE(result.getBody().isNone());
}

TEST(ReadingRequestBodyLengthStateTest, ReturnsDoneWhenEnoughDataAvailable) {
    DummyReader dummyReader("Hello");
    ReadBuffer readBuffer(dummyReader);
    readBuffer.load();

    http::ReadingRequestBodyLengthState state(5);
    http::TransitionResult result = state.handle(readBuffer);

    ASSERT_TRUE(result.getStatus().isOk());
    EXPECT_EQ(result.getStatus().unwrap(), http::IState::kDone);
    ASSERT_TRUE(result.getBody().isSome());
    EXPECT_EQ(result.getBody().unwrap(), "Hello");
}

TEST(ReadingRequestBodyLengthStateTest, HandlesMultiplePartialLoads) {
    MultiStageReader multiReader;
    ReadBuffer readBuffer(multiReader);
    http::ReadingRequestBodyLengthState state(5);

    readBuffer.load();  // "He"
    http::TransitionResult result = state.handle(readBuffer);
    EXPECT_EQ(result.getStatus().unwrap(), http::IState::kSuspend);

    readBuffer.load();  // "llo"
    result = state.handle(readBuffer);
    EXPECT_EQ(result.getStatus().unwrap(), http::IState::kDone);
    ASSERT_TRUE(result.getBody().isSome());
    EXPECT_EQ(result.getBody().unwrap(), "Hello");
}

TEST(ReadingRequestBodyLengthStateTest, ReturnsErrorOnReadFailure) {
    FailingReader failingReader;
    ReadBuffer readBuffer(failingReader);
    http::ReadingRequestBodyLengthState state(5);

    auto result = readBuffer.load();
    ASSERT_TRUE(result.isErr());

    http::TransitionResult transitionResult = state.handle(readBuffer);
    ASSERT_TRUE(transitionResult.getStatus().isOk());
    EXPECT_EQ(transitionResult.getStatus().unwrap(), http::IState::kSuspend);
    EXPECT_TRUE(transitionResult.getBody().isNone());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
