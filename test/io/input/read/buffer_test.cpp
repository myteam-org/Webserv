#include <gtest/gtest.h>
#include "io/input/read/buffer.hpp"
#include "io/input/reader/reader.hpp"
#include "utils/types/result.hpp"

#include <cstring>

namespace {

class DummyReader : public io::IReader {
public:
    explicit DummyReader(const std::string& data) : data_(data), pos_(0) {}

    types::Result<std::size_t, error::AppError> read(char* dst, std::size_t nbyte) override {
        if (pos_ >= data_.size()) return types::ok(0ul);
        const std::size_t len = std::min(nbyte, data_.size() - pos_);
        memcpy(dst, data_.data() + pos_, len);
        pos_ += len;
        return types::ok(len);
    }

    bool eof() override { return pos_ >= data_.size(); }

private:
    std::string data_;
    std::size_t pos_;
};

TEST(ReadBufferTest, ConsumeUntilDelimiterAcrossLoads) {
    class PartialReader : public io::IReader {
    public:
        PartialReader() : part_(0) {}
        types::Result<std::size_t, error::AppError> read(char* dst, std::size_t nbyte) {
            const char* data[] = { "abc\r", "\ndef", "" };
            const std::size_t sizes[] = { 4, 4, 0 };
            if (part_ >= 3) return types::ok(0ul);
            std::size_t len = std::min(nbyte, sizes[part_]);
            memcpy(dst, data[part_], len);
            ++part_;
            return types::ok(len);
        }
        bool eof() { return part_ >= 3; }
    private:
        int part_;
    };

    PartialReader reader;
    ReadBuffer buffer(reader);

    types::Result<types::Option<std::string>, error::AppError> result = buffer.consumeUntil("\r\n");
    ASSERT_TRUE(result.isOk());
    types::Option<std::string> lineOpt = result.unwrap();
    ASSERT_TRUE(lineOpt.isSome());
    EXPECT_EQ(lineOpt.unwrap(), "abc\r\n");

    result = buffer.consumeUntil("\r\n");
    ASSERT_TRUE(result.isOk());
    lineOpt = result.unwrap();
    ASSERT_TRUE(lineOpt.isNone());
}

TEST(ReadBufferTest, ConsumeUntilOnlyDelimiter) {
    DummyReader reader("\r\n");
    ReadBuffer buffer(reader);

    types::Result<types::Option<std::string>, error::AppError> result = buffer.consumeUntil("\r\n");
    ASSERT_TRUE(result.isOk());
    types::Option<std::string> lineOpt = result.unwrap();
    ASSERT_TRUE(lineOpt.isSome());
    EXPECT_EQ(lineOpt.unwrap(), "\r\n");

    result = buffer.consumeUntil("\r\n");
    ASSERT_TRUE(result.isOk());
    lineOpt = result.unwrap();
    ASSERT_TRUE(lineOpt.isNone());
}

TEST(ReadBufferTest, ConsumeUntilDelimiterAtEnd) {
    DummyReader reader("abc\r\n");
    ReadBuffer buffer(reader);

    types::Result<types::Option<std::string>, error::AppError> result = buffer.consumeUntil("\r\n");
    ASSERT_TRUE(result.isOk());
    types::Option<std::string> lineOpt = result.unwrap();
    ASSERT_TRUE(lineOpt.isSome());
    EXPECT_EQ(lineOpt.unwrap(), "abc\r\n");

    result = buffer.consumeUntil("\r\n");
    ASSERT_TRUE(result.isOk());
    lineOpt = result.unwrap();
    ASSERT_TRUE(lineOpt.isNone());
}

TEST(ReadBufferTest, ConsumeUntilMultipleCallsNoData) {
    DummyReader reader("");
    ReadBuffer buffer(reader);

    types::Result<types::Option<std::string>, error::AppError> result = buffer.consumeUntil("\r\n");
    ASSERT_TRUE(result.isOk());
    types::Option<std::string> lineOpt = result.unwrap();
    ASSERT_TRUE(lineOpt.isNone());

    result = buffer.consumeUntil("\r\n");
    ASSERT_TRUE(result.isOk());
    lineOpt = result.unwrap();
    ASSERT_TRUE(lineOpt.isNone());
}

TEST(ReadBufferTest, BasicLoadAndConsume) {
    DummyReader reader("hello_buffer");
    ReadBuffer buffer(reader);

    types::Result<std::size_t, error::AppError> result = buffer.load();
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.unwrap(), 12ul);

    EXPECT_EQ(buffer.consume(5), "hello");
    EXPECT_EQ(buffer.consume(10), "_buffer");
}

TEST(ReadBufferTest, MultipleConsumes) {
    DummyReader reader("12345");
    ReadBuffer buffer(reader);
    buffer.load();

    EXPECT_EQ(buffer.consume(2), "12");
    EXPECT_EQ(buffer.consume(2), "34");
    EXPECT_EQ(buffer.consume(2), "5");
    EXPECT_EQ(buffer.consume(1), "");
}

TEST(ReadBufferTest, LoadWhenEof) {
    DummyReader reader("");
    ReadBuffer buffer(reader);
    types::Result<std::size_t, error::AppError> result = buffer.load();
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(result.unwrap(), 0ul);
}

}  // namespace

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
