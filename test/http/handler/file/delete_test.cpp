#include <gtest/gtest.h>
#include "http/handler/file/delete.hpp"
#include "http/request/request.hpp"
#include "config/context/documentRootConfig.hpp"
#include <fstream>

namespace http {

class DeleteFileHandlerTest : public ::testing::Test {
protected:
    void SetUp() override {
        docRootConfig_.setRoot("/tmp");
        std::ofstream ofs("/tmp/test.txt");
        ofs << "test";
        ofs.close();
    }

    void TearDown() override {
        std::remove("/tmp/test.txt");
    }

    DocumentRootConfig docRootConfig_;
};

TEST_F(DeleteFileHandlerTest, DeleteExistingFile) {
    DeleteFileHandler handler(docRootConfig_);
    Request request(kMethodDelete, "/test.txt");

    Either<IAction *, Response> result = handler.serve(request);

    ASSERT_TRUE(result.isRight());
    EXPECT_EQ(result.unwrapRight().getStatusCode(), kStatusNoContent);

    std::ifstream ifs("/tmp/test.txt");
    EXPECT_FALSE(ifs.is_open());
}

TEST_F(DeleteFileHandlerTest, DeleteNonExistingFile) {
    DeleteFileHandler handler(docRootConfig_);
    Request request(kMethodDelete, "/non_existing.txt");

    Either<IAction *, Response> result = handler.serve(request);

    ASSERT_TRUE(result.isRight());
    EXPECT_EQ(result.unwrapRight().getStatusCode(), kStatusNotFound);
}

}
