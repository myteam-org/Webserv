#include <gtest/gtest.h>
#include <unistd.h>
#include <sys/socket.h>
#include <signal.h>
#include <string.h>
#include <time.h>

#include "server/EpollEventNotifier.hpp"
#include "server/EpollEvent.hpp"
#include "io/base/FileDescriptor.hpp"

namespace {

// C++98準拠のsleep関数（usleepの代替）
static void msleep(int milliseconds) {
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

TEST(EpollEventNotifierTest, RegisterModifyUnregisterFd) {
    int sv[2];
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, sv), 0);

    {
        FileDescriptor fd1(sv[0]);
        FileDescriptor fd2(sv[1]);

        EpollEventNotifier notifier;
        // 🔧 重要：epollファイルディスクリプタを初期化！
        types::Result<types::Unit, int> openResult = notifier.open();
        ASSERT_TRUE(openResult.isOk()) << "notifier.open() failed";

        EpollEvent ev(EPOLLIN, NULL);
        types::Result<int, int> regResult = notifier.registerFd(fd1, ev);
        ASSERT_TRUE(regResult.isOk()) << "registerFd failed: " << strerror(regResult.unwrapErr());
        
        ev.addEvents(EPOLLOUT);
        types::Result<int, int> modResult = notifier.modifyFd(fd1, ev);
        ASSERT_TRUE(modResult.isOk()) << "modifyFd failed: " << strerror(modResult.unwrapErr());

        types::Result<int, int> unregResult = notifier.unregisterFd(fd1);
        ASSERT_TRUE(unregResult.isOk()) << "unregisterFd failed: " << strerror(unregResult.unwrapErr());
    }
}

TEST(EpollEventNotifierTest, WaitDetectsReadableFd) {
    int sv[2];
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, sv), 0);

    {
        FileDescriptor reader(sv[0]);
        FileDescriptor writer(sv[1]);

        EpollEventNotifier notifier;
        // 🔧 重要：epollファイルディスクリプタを初期化！
        types::Result<types::Unit, int> openResult = notifier.open();
        ASSERT_TRUE(openResult.isOk()) << "notifier.open() failed";

        EpollEvent ev(EPOLLIN, NULL);
        types::Result<int, int> regResult = notifier.registerFd(reader, ev);
        ASSERT_TRUE(regResult.isOk()) << "registerFd failed: " << strerror(regResult.unwrapErr());

        const char* msg = "ping";
        ssize_t write_result = write(writer.getFd().unwrap(), msg, strlen(msg));
        ASSERT_GT(write_result, 0) << "write failed: " << strerror(errno);

        std::vector<EpollEvent> events;
        const int max_retries = 5;
        bool detected = false;
        for (int i = 0; i < max_retries; ++i) {
            types::Result<std::vector<EpollEvent>, int> result = notifier.wait();
            if (result.isOk() && !result.unwrap().empty()) {
                events = result.unwrap();
                detected = true;
                break;
            }
            msleep(1); // wait 1ms（C++98準拠）
        }

        ASSERT_TRUE(detected) << "EPOLLIN not detected in retries";
        EXPECT_TRUE(events[0].hasEvents(EPOLLIN));
    }
}

TEST(EpollEventNotifierTest, RegisterInvalidFdReturnsErr) {
    EpollEventNotifier notifier;
    // 🔧 重要：epollファイルディスクリプタを初期化！
    types::Result<types::Unit, int> openResult = notifier.open();
    ASSERT_TRUE(openResult.isOk()) << "notifier.open() failed";

    FileDescriptor invalidFd(-1);
    EpollEvent ev(EPOLLIN, NULL);
    types::Result<int, int> result = notifier.registerFd(invalidFd, ev);
    ASSERT_TRUE(result.isErr());
    EXPECT_EQ(result.unwrapErr(), EINVAL);
}

TEST(EpollEventNotifierTest, WaitOnEmptyReturnsOkOrErr) {
    EpollEventNotifier notifier;
    // 🔧 重要：epollファイルディスクリプタを初期化！
    types::Result<types::Unit, int> openResult = notifier.open();
    ASSERT_TRUE(openResult.isOk()) << "notifier.open() failed";

    types::Result<std::vector<EpollEvent>, int> result = notifier.wait();

    if (result.isOk()) {
        EXPECT_TRUE(result.unwrap().empty());
    } else {
        int err = result.unwrapErr();
        EXPECT_TRUE(err == EINTR || err == EINVAL);
    }
}

} // namespace

int main(int argc, char** argv) {
    ::signal(SIGPIPE, SIG_IGN);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
