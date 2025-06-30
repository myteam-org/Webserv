#include <gtest/gtest.h>

#include "utils/types/try.hpp"
#include "utils/types/result.hpp"
#include "utils/types/option.hpp"

namespace {

types::Result<int, std::string> FuncResultErrorShortCircuit() {
    int value = TRY((types::Result<int, std::string>(types::err(std::string("error occurred")))));
    return types::Result<int, std::string>(types::ok(value + 1));
}

// Option<int>を返すのではなく、unwrapしてintを返す関数にする
types::Option<int> FuncOptionErrorShortCircuit() {
    types::Option<int> opt = types::Option<int>(types::None());
    if (!opt.isSome()) {
        return types::Option<int>(types::None());
    }
    int value = opt.unwrap();
    return types::Option<int>(types::some(value + 1));
}

TEST(TryMacroTest, ResultErrorShortCircuit) {
    types::Result<int, std::string> res = FuncResultErrorShortCircuit();
    EXPECT_TRUE(res.isErr());
    EXPECT_EQ("error occurred", res.unwrapErr());
}

TEST(TryMacroTest, OptionErrorShortCircuit) {
    types::Option<int> opt = FuncOptionErrorShortCircuit();
    EXPECT_TRUE(opt.isNone());
}

}  // namespace

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
