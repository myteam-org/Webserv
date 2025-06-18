#include <gtest/gtest.h>
#include "utils/types/result.hpp"

namespace {

// グローバルスコープに移動
struct Tracer {
    static int count;
    Tracer() { ++count; }
    Tracer(const Tracer&) { ++count; }
    ~Tracer() { --count; }
};
int Tracer::count = 0;

TEST(ResultTest, OkConstruction) {
    types::Result<int, std::string> result(types::ok(42));
    EXPECT_TRUE(result.is_ok());
    EXPECT_FALSE(result.is_err());
    EXPECT_EQ(42, result.unwrap());
}

TEST(ResultTest, ErrConstruction) {
    types::Result<int, std::string> result(types::err(std::string("error")));
    EXPECT_FALSE(result.is_ok());
    EXPECT_TRUE(result.is_err());
    EXPECT_EQ("error", result.unwrap_err());
}

TEST(ResultTest, UnwrapOnErrThrows) {
    types::Result<int, std::string> result(types::err(std::string("error")));
    EXPECT_THROW(result.unwrap(), std::runtime_error);
}

TEST(ResultTest, UnwrapErrOnOkThrows) {
    types::Result<int, std::string> result(types::ok(42));
    EXPECT_THROW(result.unwrap_err(), std::runtime_error);
}

TEST(ResultTest, OkDestruction) {
    {
        types::Result<Tracer, std::string> result(types::ok(Tracer()));
        EXPECT_EQ(1, Tracer::count);
    }
    EXPECT_EQ(0, Tracer::count);
}

TEST(ResultTest, ErrDestruction) {
    {
        types::Result<int, Tracer> result(types::err(Tracer()));
        EXPECT_EQ(1, Tracer::count);
    }
    EXPECT_EQ(0, Tracer::count);
}

}  // namespace

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
