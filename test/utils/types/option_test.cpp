#include <gtest/gtest.h>
#include "utils/types/option.hpp"

namespace {

// グローバルスコープに移動
struct Tracer {
    static int count;
    Tracer() { ++count; }
    Tracer(const Tracer&) { ++count; }
    ~Tracer() { --count; }
};

int Tracer::count = 0;

TEST(OptionTest, SomeConstruction) {
    types::Option<int> option(types::some(42));
    EXPECT_TRUE(option.isSome());
    EXPECT_FALSE(option.isNone());
    EXPECT_EQ(42, option.unwrap());
}

TEST(OptionTest, NoneConstruction) {
    types::Option<int> option(types::none);
    EXPECT_FALSE(option.isSome());
    EXPECT_TRUE(option.isNone());
}

TEST(OptionTest, UnwrapOnNoneThrows) {
    types::Option<int> option(types::none);
    EXPECT_THROW(option.unwrap(), std::runtime_error);
}

TEST(OptionTest, UnwrapOrWithSome) {
    types::Option<int> option(types::some(42));
    EXPECT_EQ(42, option.unwrapOr(0));
}

TEST(OptionTest, UnwrapOrWithNone) {
    types::Option<int> option(types::none);
    EXPECT_EQ(0, option.unwrapOr(0));
}

TEST(OptionTest, SomeDestruction) {
    {
        types::Option<Tracer> option(types::some(Tracer()));
        EXPECT_EQ(1, Tracer::count);
    }
    EXPECT_EQ(0, Tracer::count);
}

TEST(OptionTest, NoneDestruction) {
    {
        types::Option<Tracer> option(types::none);
        EXPECT_EQ(0, Tracer::count);
    }
    EXPECT_EQ(0, Tracer::count);
}

TEST(OptionTest, StringOption) {
    types::Option<std::string> some_option(types::some(std::string("hello")));
    types::Option<std::string> none_option(types::none);
    
    EXPECT_EQ("hello", some_option.unwrap());
    EXPECT_EQ("default", none_option.unwrapOr("default"));
}

}  // namespace

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
