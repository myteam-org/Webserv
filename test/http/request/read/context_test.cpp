#include <gtest/gtest.h>
#include <string>
#include "http/request/read/context.hpp"
#include "http/request/read/state.hpp"
#include "utils/types/result.hpp"
#include "io/input/read/buffer.hpp"
#include "io/input/reader/reader.hpp"

// ---- Begin: IConfigResolver仮実装 ----
namespace http {
class IConfigResolver {
public:
    virtual ~IConfigResolver() {}
    // 必要であれば純粋仮想関数をここに追加
};
}
// ---- End: IConfigResolver仮実装 ----

// ダミーConfigResolver
class DummyConfigResolver : public http::IConfigResolver {
public:
    DummyConfigResolver() {}
    virtual ~DummyConfigResolver() {}
    // 必要な仮想関数実装（現状は空でOK）
};

// テスト用ダミーState
class DummyState : public http::IState {
public:
    DummyState() : handleCallCount_(0) {}
    virtual ~DummyState() {}

    virtual http::TransitionResult handle(ReadBuffer& /*buf*/) {
        ++handleCallCount_;
        http::TransitionResult result;
        if (handleCallCount_ == 1) {
            result.setStatus(types::ok(http::IState::kDone));
            result.setRequestLine(types::Option<std::string>(types::some(std::string("REQLINE"))));
        }
        return result;
    }

    int handleCallCount_;
};

TEST(ReadContextTest, HandleUpdatesRequestLineAndStatus) {
    DummyConfigResolver resolver;
    http::ReadContext context(resolver, new DummyState());

    class DummyReader : public io::IReader {
    public:
        DummyReader() {}
        virtual types::Result<std::size_t, error::AppError> read(char*, std::size_t) { return types::ok(0ul); }
        virtual bool eof() { return true; }
    } dummyReader;
    ReadBuffer buffer(dummyReader);

    types::Result< http::IState::HandleStatus, error::AppError> result = context.handle(buffer);

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
