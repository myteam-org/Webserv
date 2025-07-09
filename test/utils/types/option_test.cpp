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
    types::Option<int> option = types::Option<int>(types::None());
    EXPECT_FALSE(option.isSome());
    EXPECT_TRUE(option.isNone());
}

TEST(OptionTest, UnwrapOnNoneThrows) {
    types::Option<int> option = types::Option<int>(types::None());
    EXPECT_THROW(option.unwrap(), std::runtime_error);
}

TEST(OptionTest, UnwrapOrWithSome) {
    types::Option<int> option(types::some(42));
    EXPECT_EQ(42, option.unwrapOr(0));
}

TEST(OptionTest, UnwrapOrWithNone) {
    types::Option<int> option = types::Option<int>(types::None());
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
        types::Option<Tracer> option = types::Option<Tracer>(types::None());
        EXPECT_EQ(0, Tracer::count);
    }
    EXPECT_EQ(0, Tracer::count);
}

TEST(OptionTest, StringOption) {
    types::Option<std::string> some_option(types::some(std::string("hello")));
    types::Option<std::string> none_option = types::Option<std::string>(types::None());
    
    EXPECT_EQ("hello", some_option.unwrap());
    EXPECT_EQ("default", none_option.unwrapOr("default"));
}

TEST(OptionTest, EqualityOperator) {
    types::Option<int> some1 = types::some(10);
    types::Option<int> some2 = types::some(10);
    types::Option<int> some3 = types::some(20);
    types::Option<int> none1 = types::none<int>();
    types::Option<int> none2 = types::none<int>();

    EXPECT_TRUE(some1 == some2);
    EXPECT_FALSE(some1 == some3);
    EXPECT_FALSE(some1 == none1);
    EXPECT_TRUE(none1 == none2);

    types::Option<std::string> str_some1 = types::some(std::string("test"));
    types::Option<std::string> str_some2 = types::some(std::string("test"));
    types::Option<std::string> str_none1 = types::none<std::string>();

    EXPECT_TRUE(str_some1 == str_some2);
    EXPECT_FALSE(str_some1 == str_none1);
}

}  // namespace

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
