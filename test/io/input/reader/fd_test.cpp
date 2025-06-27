#include <gtest/gtest.h>
#include "io/input/reader/fd.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <cstdio>

namespace {

std::string createTempFileWithContent(const std::string& content) {
    const char* tmpPath = "/tmp/fdreader_test_XXXXXX";
    char pathBuf[] = "/tmp/fdreader_test_XXXXXX";
    int fd = mkstemp(pathBuf);
    if (fd != -1) {
        ::write(fd, content.c_str(), content.size());
        ::close(fd);
    }
    return std::string(pathBuf);
}

TEST(FdReaderTest, ReadAllData) {
    std::string expected = "fd_reader_test";
    std::string path = createTempFileWithContent(expected);
    int fd = ::open(path.c_str(), O_RDONLY);
    ASSERT_NE(fd, -1);

    io::FdReader reader(fd);
    char buf[64] = {};

    // 1回目：全部読める
    auto r1 = reader.read(buf, sizeof(buf));
    ASSERT_TRUE(r1.isOk());
    EXPECT_EQ(std::string(buf, r1.unwrap()), expected);
    EXPECT_FALSE(reader.eof());

    // 2回目：EOF なので 0
    auto r2 = reader.read(buf, sizeof(buf));
    ASSERT_TRUE(r2.isOk());
    EXPECT_EQ(r2.unwrap(), 0ul);
    EXPECT_TRUE(reader.eof());

    ::close(fd);
    ::remove(path.c_str());
}

TEST(FdReaderTest, ReadEmptyFile) {
    std::string path = createTempFileWithContent("");
    int fd = ::open(path.c_str(), O_RDONLY);
    ASSERT_NE(fd, -1);

    io::FdReader reader(fd);
    char buf[16];
    auto result = reader.read(buf, sizeof(buf));
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.unwrap(), 0ul);
    EXPECT_TRUE(reader.eof());

    ::close(fd);
    ::remove(path.c_str());
}

}  // namespace

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

