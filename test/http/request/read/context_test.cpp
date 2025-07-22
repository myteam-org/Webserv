#include "http/request/read/context.hpp"

#include <gtest/gtest.h>

#include <string>

#include "http/config/config_resolver.hpp"
#include "http/request/read/state.hpp"
#include "io/input/read/buffer.hpp"
#include "io/input/reader/reader.hpp"
#include "utils/types/result.hpp"

// ---- Begin: IConfigResolver仮実装 ----
namespace http {
class IConfigResolver {
   public:
    virtual ~IConfigResolver() {}
    // 必要であれば純粋仮想関数をここに追加
};
}  // namespace http
// ---- End: IConfigResolver仮実装 ----

// ダミーConfigResolver
class DummyConfigResolver : public http::config::IConfigResolver {
   public:
    DummyConfigResolver() {}
    virtual ~DummyConfigResolver() {}
    const ServerContext& choseServer(const std::string& host) const {
        (void)host;
        static ServerContext dummy("dummy_server");
        return dummy;  // テスト用の適当なオブジェクトを返す
    }
};

// テスト用ダミーState
class DummyState : public http::IState {
   public:
    DummyState() : handleCallCount_(0) {}
    virtual ~DummyState() {}

    virtual http::TransitionResult handle(http::ReadContext& ctx,
                                          ReadBuffer& buf) {
        (void)ctx;
        (void)buf;
        ++handleCallCount_;
        http::TransitionResult result;
        if (handleCallCount_ == 1) {
            result.setStatus(types::ok(http::IState::kDone));
            result.setRequestLine(types::some(std::string("REQLINE")));
        } else {
            result.setStatus(types::ok(http::IState::kSuspend));
        }
        return result;
    }

   private:
    int handleCallCount_;
};

TEST(ReadContextTest, HandleUpdatesRequestLineAndStatus) {
    DummyConfigResolver resolver;
    http::ReadContext context(resolver, new DummyState());

    class DummyReader : public io::IReader {
       public:
        DummyReader() {}
        virtual types::Result<std::size_t, error::AppError> read(char*,
                                                                 std::size_t) {
            return types::ok(0ul);
        }
        virtual bool eof() { return true; }
    } dummyReader;
    ReadBuffer buffer(dummyReader);

    types::Result<http::IState::HandleStatus, error::AppError> result =
        context.handle(buffer);

    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.unwrap(), http::IState::kDone);
    EXPECT_EQ(context.getRequestLine(), "REQLINE");
}

TEST(ReadContextTest, ChangeStateWorks) {
    DummyConfigResolver resolver;
    http::ReadContext context(resolver, new DummyState());

    EXPECT_TRUE(context.getState() != NULL);

    DummyState* newState = new DummyState();
    context.changeState(newState);

    EXPECT_EQ(context.getState(), newState);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
