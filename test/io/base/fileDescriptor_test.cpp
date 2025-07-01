#include <gtest/gtest.h>
#include <unistd.h>
#include "io/base/FileDescriptor.hpp"


namespace {

TEST(FileDescriptorTest, ConstructAndGetFd) {
    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    FileDescriptor fd(pipefd[0]);
    auto fd_opt = fd.getFd();
    ASSERT_TRUE(fd_opt.canUnwrap());
    EXPECT_EQ(fd_opt.unwrap(), pipefd[0]);

    close(pipefd[1]);
}

TEST(FileDescriptorTest, DestructorClosesFd) {
    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    {
        FileDescriptor fd(pipefd[0]);
    }
    char buf;
    EXPECT_EQ(read(pipefd[0], &buf, 1), -1);
    EXPECT_EQ(errno, EBADF);
    close(pipefd[1]);
}

TEST(FileDescriptorTest, SetFdReplacesAndClosesPrevious) {
    int pipefd1[2];
    int pipefd2[2];
    ASSERT_EQ(pipe(pipefd1), 0);
    ASSERT_EQ(pipe(pipefd2), 0);

    FileDescriptor fd(pipefd1[0]);
    fd.setFd(pipefd2[0]);

    char buf;
    EXPECT_EQ(read(pipefd1[0], &buf, 1), -1);
    EXPECT_EQ(errno, EBADF);

    EXPECT_NE(read(pipefd2[0], &buf, 0), -1);

    close(pipefd1[1]);
    close(pipefd2[1]);
}

TEST(FileDescriptorTest, InvalidFdReturnsNone) {
    FileDescriptor fd;
    auto opt = fd.getFd();
    EXPECT_FALSE(opt.canUnwrap());
}

} // namespace

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
