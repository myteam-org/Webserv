#include <gtest/gtest.h>
#include <unistd.h>
#include <sys/socket.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <time.h>    // C++98でのsleep代替

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
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, sv), 0) << "socketpair failed: " << strerror(errno);

    {
        FileDescriptor fd1(sv[0]);
        FileDescriptor fd2(sv[1]);

        EpollEventNotifier notifier;
        
        // epollファイルディスクリプタを初期化
        types::Result<types::Unit, int> openResult = notifier.open();
        ASSERT_TRUE(openResult.isOk()) << "notifier.open() failed: " << openResult.unwrapErr();

        // FDを登録
        EpollEvent ev(EPOLLIN, NULL);
        types::Result<int, int> regResult = notifier.registerFd(fd1, ev);
        ASSERT_TRUE(regResult.isOk()) << "registerFd failed: " << regResult.unwrapErr();

        // イベントを変更（同じEpollEventオブジェクトを再利用）
        ev.addEvents(EPOLLOUT);
        types::Result<int, int> modResult = notifier.modifyFd(fd1, ev);
        ASSERT_TRUE(modResult.isOk()) << "modifyFd failed: " << modResult.unwrapErr();

        // FDの登録解除
        types::Result<int, int> unregResult = notifier.unregisterFd(fd1);
        ASSERT_TRUE(unregResult.isOk()) << "unregisterFd failed: " << unregResult.unwrapErr();
    }
    
    // FileDescriptorのデストラクタが自動的にcloseを呼ぶ
}

TEST(EpollEventNotifierTest, WaitDetectsReadableFd) {
    int sv[2];
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, sv), 0) << "socketpair failed: " << strerror(errno);

    {
        FileDescriptor reader(sv[0]);
        FileDescriptor writer(sv[1]);

        EpollEventNotifier notifier;
        
        // epollファイルディスクリプタを初期化
        types::Result<types::Unit, int> openResult = notifier.open();
        ASSERT_TRUE(openResult.isOk()) << "notifier.open() failed";

        // 読み込み可能イベントを監視するようにFDを登録
        EpollEvent ev(EPOLLIN, NULL);
        types::Result<int, int> regResult = notifier.registerFd(reader, ev);
        ASSERT_TRUE(regResult.isOk()) << "registerFd failed: " << regResult.unwrapErr();

        // データを書き込んで読み込み可能状態にする
        const char* msg = "ping";
        ssize_t write_result = write(writer.getFd().unwrap(), msg, strlen(msg));
        ASSERT_GT(write_result, 0) << "write failed: " << strerror(errno);

        // データが確実に書き込まれるまで少し待つ
        msleep(10); // 10ms

        // epoll_waitでイベントを検出
        bool detected = false;
        const int max_retries = 5;
        
        for (int i = 0; i < max_retries && !detected; ++i) {
            types::Result<std::vector<EpollEvent>, int> result = notifier.wait();
            
            if (result.isOk()) {
                std::vector<EpollEvent> events = result.unwrap();
                if (!events.empty()) {
                    detected = true;
                    EXPECT_TRUE(events[0].hasEvents(EPOLLIN)) << "Expected EPOLLIN event not found";
                    break;
                }
            } else {
                int err = result.unwrapErr();
                // EINTRは無視して再試行
                if (err != EINTR) {
                    FAIL() << "wait() failed: " << strerror(err);
                }
            }
            
            // 短い間隔で再試行
            msleep(5); // 5ms
        }

        ASSERT_TRUE(detected) << "EPOLLIN event not detected after " << max_retries << " retries";
    }
}

TEST(EpollEventNotifierTest, RegisterInvalidFdReturnsErr) {
    EpollEventNotifier notifier;
    
    // epollファイルディスクリプタを初期化
    ASSERT_TRUE(notifier.open().isOk());
    
    FileDescriptor invalidFd(-1);
    EpollEvent ev(EPOLLIN, NULL);
    
    types::Result<int, int> result = notifier.registerFd(invalidFd, ev);
    ASSERT_TRUE(result.isErr());
    
    int error = result.unwrapErr();
    // 無効なFDの場合、EINVALが返される
    EXPECT_EQ(error, EINVAL) << "Unexpected error: " << strerror(error);
}

TEST(EpollEventNotifierTest, WaitOnEmptyReturnsOkOrErr) {
    EpollEventNotifier notifier;
    
    // epollファイルディスクリプタを初期化
    ASSERT_TRUE(notifier.open().isOk());
    
    types::Result<std::vector<EpollEvent>, int> result = notifier.wait();

    if (result.isOk()) {
        // 何もFDが登録されていない場合、空のベクトル（タイムアウト）
        std::vector<EpollEvent> events = result.unwrap();
        EXPECT_TRUE(events.empty()) << "Expected no events when no FDs are registered";
    } else {
        // エラーの場合、妥当なエラーコードを確認
        int err = result.unwrapErr();
        EXPECT_EQ(err, EINTR) << "Unexpected error code: " << strerror(err);
    }
}

}  // namespace

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
