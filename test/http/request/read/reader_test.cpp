#include <gtest/gtest.h>
#include <string>
#include <cstring>
#include "io/input/read/buffer.hpp"
#include "io/input/reader/reader.hpp"
#include "http/request/read/reader.hpp"
#include "http/request/request.hpp"
#include "utils/types/result.hpp"
#include "utils/types/option.hpp"
#include "utils/types/error.hpp"

// --- ダミーIConfigResolverインターフェースをここで仮定定義 ---
namespace http {
class IConfigResolver {
public:
    virtual ~IConfigResolver() {}
    // 必要な純粋仮想関数をここに追加（現状は空でOK）
};
} // namespace http

namespace http {

class DummyConfigResolver : public IConfigResolver {
public:
    DummyConfigResolver() {}
    virtual ~DummyConfigResolver() {}
    // 必要であれば純粋仮想関数をここで実装
};

}  // namespace http

namespace {

class DummyReader : public io::IReader {
public:
    explicit DummyReader(const std::string& inputData)
        : inputData_(inputData), position_(0) {}

    virtual types::Result<std::size_t, error::AppError> read(char* destination, std::size_t byteCount) {
        if (position_ >= inputData_.size()) {
            return types::ok(0ul);
        }
        const std::size_t copyLength = std::min(byteCount, inputData_.size() - position_);
        memcpy(destination, inputData_.data() + position_, copyLength);
        position_ += copyLength;
        return types::ok(copyLength);
    }

    virtual bool eof() { return position_ >= inputData_.size(); }

private:
    std::string inputData_;
    std::size_t position_;
};

}  // namespace

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
