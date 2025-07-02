#include <gtest/gtest.h>
#include "io/input/read/buffer.hpp"
#include "io/input/reader/reader.hpp"
#include "http/request/read/utils.hpp"
#include "utils/types/result.hpp"
#include "utils/types/option.hpp"
#include "utils/types/error.hpp"
#include <cstring>

namespace {

class DummyReader : public io::IReader {
public:
    explicit DummyReader(const std::string& data) : data_(data), pos_(0) {}
    types::Result<std::size_t, error::AppError> read(char* dst, std::size_t nbyte) {
        if (pos_ >= data_.size()) return types::ok(0ul);
        const std::size_t len = std::min(nbyte, data_.size() - pos_);
        memcpy(dst, data_.data() + pos_, len);
        pos_ += len;
        return types::ok(len);
    }
    bool eof() { return pos_ >= data_.size(); }
private:
    std::string data_;
    std::size_t pos_;
};

TEST(GetLineTest, ReturnsLineWithoutCRLF) {
    DummyReader reader("abc\r\n");
    ReadBuffer buffer(reader);

    http::GetLineResult result = http::getLine(buffer);
    ASSERT_TRUE(result.isOk());
    types::Option<std::string> opt = result.unwrap();
    ASSERT_TRUE(opt.isSome());
    EXPECT_EQ(opt.unwrap(), "abc");
}

TEST(GetLineTest, ReturnsNoneIfNoDelimiter) {
    DummyReader reader("abcde");
    ReadBuffer buffer(reader);

    http::GetLineResult result = http::getLine(buffer);
    ASSERT_TRUE(result.isOk());
    types::Option<std::string> opt = result.unwrap();
    EXPECT_TRUE(opt.isNone());
}

TEST(GetLineTest, ReturnsErrorIfNotEndsWithCRLF) {
    DummyReader reader("abc\n");
    ReadBuffer buffer(reader);

    http::GetLineResult result = http::getLine(buffer);
    ASSERT_TRUE(result.isOk());
    types::Option<std::string> opt = result.unwrap();
    EXPECT_TRUE(opt.isNone());
}

TEST(GetLineTest, HandlesMultipleLines) {
    DummyReader reader("one\r\ntwo\r\nthree\r\n");
    ReadBuffer buffer(reader);

    std::cout << "Call 1" << std::endl;
    http::GetLineResult result = http::getLine(buffer);
    ASSERT_TRUE(result.isOk());
    types::Option<std::string> opt = result.unwrap();
    ASSERT_TRUE(opt.isSome());
    if (opt.isSome()) {
        EXPECT_EQ(opt.unwrap(), "one");
    }

    std::cout << "Call 2" << std::endl;
    result = http::getLine(buffer);
    ASSERT_TRUE(result.isOk());
    opt = result.unwrap();
    ASSERT_TRUE(opt.isSome());
    if (opt.isSome()) {
        EXPECT_EQ(opt.unwrap(), "two");
    }

    std::cout << "Call 3" << std::endl;
    result = http::getLine(buffer);
    ASSERT_TRUE(result.isOk());
    opt = result.unwrap();
    ASSERT_TRUE(opt.isSome());
    if (opt.isSome()) {
        EXPECT_EQ(opt.unwrap(), "three");
    }

    std::cout << "Call 4" << std::endl;
    result = http::getLine(buffer);
    ASSERT_TRUE(result.isOk());
    opt = result.unwrap();
    EXPECT_TRUE(opt.isNone());
}

} // namespace

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
