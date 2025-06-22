#include <gtest/gtest.h>

#include "utils/types/try.hpp"
#include "utils/types/result.hpp"
#include "utils/types/option.hpp"

namespace {

TEST(TryMacroTest, ResultErrorShortCircuit) {
    auto func = []() -> types::Result<int, std::string> {
        // テンプレート引数のカンマを正しく解釈させるため括弧で囲む
        int value = TRY((types::Result<int, std::string>(types::err(std::string("error occurred")))));
        return types::Result<int, std::string>(types::ok(value + 1));
    };

    auto res = func();
    EXPECT_TRUE(res.isErr());
    EXPECT_EQ("error occurred", res.unwrapErr());
}

TEST(TryMacroTest, OptionErrorShortCircuit) {
    auto func = []() -> types::Option<int> {
        int value = TRY((types::Option<int>(types::none)));
        return types::Option<int>(types::some(value + 1));
    };

    auto opt = func();
    EXPECT_TRUE(opt.isNone());
}

}  // namespace

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

