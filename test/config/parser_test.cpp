#include "parser.hpp"

#include <gtest/gtest.h>

#include <fstream>
#include <sstream>
#include <sys/stat.h>  // mkdir
#include <sys/types.h> // mode_t

#include "server.hpp"
#include "token.hpp"
#include "tokenizer.hpp"
#include "config.hpp"

class ConfigParserTest : public ::testing::Test {
   protected:
    void SetUp() override {
        // Setup code here
    }

    void TearDown() override {
        // Cleanup code here
        // Clean up temporary files
        for (const auto& filename : tempFiles_) {
            std::remove(filename.c_str());
        }
    }

    // Helper method to create a temporary config file and return
    // ConfigTokenizer
    std::unique_ptr<ConfigTokenizer> createTokenizerFromConfig(
        const std::string& configContent) {
        // Create a temporary file
        std::string filename =
            "test_config_" + std::to_string(fileCounter_++) + ".conf";
        tempFiles_.push_back(filename);

        std::ofstream file(filename);
        file << configContent;
        file.close();

        return std::make_unique<ConfigTokenizer>(filename);
    }

   private:
    static int fileCounter_;
    std::vector<std::string> tempFiles_;
};

int ConfigParserTest::fileCounter_ = 0;

// Test constructor and destructor
TEST_F(ConfigParserTest, ConstructorDestructorTest) {
    auto tokenizer = createTokenizerFromConfig("");

    EXPECT_NO_THROW({ ConfigParser parser(*tokenizer); });
}

// Test error page configuration
TEST_F(ConfigParserTest, ParseErrorPageConfiguration) {
    std::string config = R"(
server {
    error_page 404 /error/404.html;
}
)";

    auto tokenizer = createTokenizerFromConfig(config);
    ConfigParser parser(*tokenizer);
    const auto& servers = parser.getServer();

    EXPECT_EQ(servers.size(), 1);
    // Note: You'll need to add a getter method in ServerContext to test error
    // pages
}

// Test max body size configuration
TEST_F(ConfigParserTest, ParseMaxBodySizeConfiguration) {
    std::string config = R"(
server {
    client_max_body_size 1024;
}
)";

    auto tokenizer = createTokenizerFromConfig(config);
    ConfigParser parser(*tokenizer);
    const auto& servers = parser.getServer();

    EXPECT_EQ(servers.size(), 1);
    EXPECT_EQ(servers[0].getClientMaxBodySize(), 1024);
}

// Test max body size - invalid number
TEST_F(ConfigParserTest, ParseMaxBodySizeConfiguration2) {
    std::string config = R"(
server {
    client_max_body_size -100;
}
)";

    auto tokenizer = createTokenizerFromConfig(config);

    EXPECT_THROW({ ConfigParser parser(*tokenizer); }, std::runtime_error);
}

// Test max body size - invalid number
TEST_F(ConfigParserTest, ParseMaxBodySizeConfiguration3) {
    std::string config = R"(
server {
    client_max_body_size 999999999999999999;
}
)";

    auto tokenizer = createTokenizerFromConfig(config);

    EXPECT_THROW({ ConfigParser parser(*tokenizer); }, std::runtime_error);
}

// Test error handling - unmatched braces
TEST_F(ConfigParserTest, UnmatchedBracesError) {
    std::string config = R"(
server {
    listen 8080;
}
}
)";  // Extra closing brace

    auto tokenizer = createTokenizerFromConfig(config);

    EXPECT_THROW({ ConfigParser parser(*tokenizer); }, std::runtime_error);
}

// Test error handling - unmatched braces2
TEST_F(ConfigParserTest, UnmatchedBracesError2) {
    std::string config = R"(
    } server {
    )";  // Extra closing brace

    try {
        auto tokenizer = createTokenizerFromConfig(config);
        ConfigParser parser(*tokenizer);
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) {
        EXPECT_STREQ(e.what(), "} server {: Syntax error : line 1");
    } catch (...) {
        FAIL() << "Caught unknown exception type";
    }
}

// Test error handling - unmatched braces3
TEST_F(ConfigParserTest, UnmatchedBracesError3) {
    std::string config = R"(
    { server {
    )";  // Extra closing brace

    try {
        auto tokenizer = createTokenizerFromConfig(config);
        ConfigParser parser(*tokenizer);
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) {
        EXPECT_STREQ(e.what(), "{: Config brace close error: line2");
    } catch (...) {
        FAIL() << "Caught unknown exception type";
    }
}

// Test error handling - config beginning error
TEST_F(ConfigParserTest, ConfigFileBeginningError) {
    std::string config = R"(
    listen {
    )";  // Extra closing brace

    try {
        auto tokenizer = createTokenizerFromConfig(config);
        ConfigParser parser(*tokenizer);
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) {
        EXPECT_STREQ(e.what(), "listen: Syntax error: line2");
    } catch (...) {
        FAIL() << "Caught unknown exception type";
    }
}

// Test error handling - invalid port number
TEST_F(ConfigParserTest, InvalidPortNumberError) {
    std::string config = R"(
server {
    listen invalid_port;
}
)";

    auto tokenizer = createTokenizerFromConfig(config);

    EXPECT_THROW({ ConfigParser parser(*tokenizer); }, std::runtime_error);
}

// Test error handling - invalid port number
TEST_F(ConfigParserTest, InvalidPortNumberError2) {
    std::string config = R"(
server {
    listen -100;
}
)";

    auto tokenizer = createTokenizerFromConfig(config);

    EXPECT_THROW({ ConfigParser parser(*tokenizer); }, std::runtime_error);
}

// Test error handling - invalid port number
TEST_F(ConfigParserTest, InvalidPortNumberError3) {
    std::string config = R"(
server {
    listen 70000;
}
)";

    auto tokenizer = createTokenizerFromConfig(config);

    EXPECT_THROW({ ConfigParser parser(*tokenizer); }, std::runtime_error);
}

// Test error handling - invalid server block member error1
TEST_F(ConfigParserTest, InvalidSeerverBlockMemberError1) {
    std::string config = R"(
server {
    root /;
}
)";

    auto tokenizer = createTokenizerFromConfig(config);

    EXPECT_THROW({ ConfigParser parser(*tokenizer); }, std::runtime_error);
}

// Test error handling - invalid server block member error2
TEST_F(ConfigParserTest, InvalidSeerverBlockMemberError2) {
    std::string config = R"(
server {
    allow_method GET;
}
)";

    auto tokenizer = createTokenizerFromConfig(config);

    EXPECT_THROW({ ConfigParser parser(*tokenizer); }, std::runtime_error);
}

// Test error handling - invalid server block member error3
TEST_F(ConfigParserTest, InvalidSeerverBlockMemberError3) {
    std::string config = R"(
server {
    index index.html index.htm;
}
)";

    auto tokenizer = createTokenizerFromConfig(config);

    EXPECT_THROW({ ConfigParser parser(*tokenizer); }, std::runtime_error);
}

// Test error handling - invalid server block member error4
TEST_F(ConfigParserTest, InvalidSeerverBlockMemberError4) {
    std::string config = R"(
server {
    autoindex ON;
}
)";

    auto tokenizer = createTokenizerFromConfig(config);

    EXPECT_THROW({ ConfigParser parser(*tokenizer); }, std::runtime_error);
}

// Test error handling - invalid server block member error5
TEST_F(ConfigParserTest, InvalidSeerverBlockMemberError5) {
    std::string config = R"(
server {
    is_cgi ON;
}
)";

    auto tokenizer = createTokenizerFromConfig(config);

    EXPECT_THROW({ ConfigParser parser(*tokenizer); }, std::runtime_error);
}

// Test error handling - invalid server block member error6
TEST_F(ConfigParserTest, InvalidSeerverBlockMemberError6) {
    std::string config = R"(
server {
    redirect http://localhost:8080/;
}
)";

    auto tokenizer = createTokenizerFromConfig(config);

    EXPECT_THROW({ ConfigParser parser(*tokenizer); }, std::runtime_error);
}

// Test error handling - invalid location block member error1
TEST_F(ConfigParserTest, InvalidLocationBlockMemberError1) {
    std::string config = R"(
server {
    location / {
        location /;
    }
}
)";

    auto tokenizer = createTokenizerFromConfig(config);

    EXPECT_THROW({ ConfigParser parser(*tokenizer); }, std::runtime_error);
}

// Test error handling - invalid location block member error2
TEST_F(ConfigParserTest, InvalidLocationBlockMemberError2) {
    std::string config = R"(
server {
    location / {
        server {
        }
    }
}
)";

    auto tokenizer = createTokenizerFromConfig(config);

    EXPECT_THROW({ ConfigParser parser(*tokenizer); }, std::runtime_error);
}

// Test error handling - invalid location block member error3
TEST_F(ConfigParserTest, InvalidLocationBlockMemberError3) {
    std::string config = R"(
server {
    location / {
        listen 80;
    }
}
)";

    auto tokenizer = createTokenizerFromConfig(config);

    EXPECT_THROW({ ConfigParser parser(*tokenizer); }, std::runtime_error);
}

// Test error handling - invalid location block member error4
TEST_F(ConfigParserTest, InvalidLocationBlockMemberError4) {
    std::string config = R"(
server {
    location / {
        host localhost;
    }
}
)";

    auto tokenizer = createTokenizerFromConfig(config);

    EXPECT_THROW({ ConfigParser parser(*tokenizer); }, std::runtime_error);
}

// Test error handling - invalid location block member error5
TEST_F(ConfigParserTest, InvalidLocationBlockMemberError5) {
    std::string config = R"(
server {
    location / {
        server_names www;
    }
}
)";

    auto tokenizer = createTokenizerFromConfig(config);

    EXPECT_THROW({ ConfigParser parser(*tokenizer); }, std::runtime_error);
}

// Test error handling - invalid location block member error6
TEST_F(ConfigParserTest, InvalidLocationBlockMemberError6) {
    std::string config = R"(
server {
    location / {
        error_page 500 500.html;
    }
}
)";

    auto tokenizer = createTokenizerFromConfig(config);

    EXPECT_THROW({ ConfigParser parser(*tokenizer); }, std::runtime_error);
}

// Test error handling - invalid location block member error7
TEST_F(ConfigParserTest, InvalidLocationBlockMemberError7) {
    std::string config = R"(
server {
    location / {
        client_max_body_size 1000000;
    }
}
)";

    auto tokenizer = createTokenizerFromConfig(config);

    EXPECT_THROW({ ConfigParser parser(*tokenizer); }, std::runtime_error);
}

// Test error handling - no listen in server block
TEST_F(ConfigParserTest, NoListenInServerError) {
    std::string configText = R"(
server {
    host localhost;
    location / {
        root /;
        index index.html;
    }
}
)";

    const std::string dir = "./config_file";
    const std::string confFile = dir + "/temp_test.conf";

    // ディレクトリがなければ作成（0700 = 所有者に読み書き実行権限）
    mkdir(dir.c_str(), 0700);

    std::ofstream ofs(confFile.c_str());
    ASSERT_TRUE(ofs.is_open()) << "Failed to open config file for writing";
    ofs << configText;
    ofs.close();

    testing::internal::CaptureStderr();
    Config config(confFile);  // ← コンストラクタで checkAndEraseServerNode() 実行
    std::string output = testing::internal::GetCapturedStderr();

    // デバッグ表示
    std::cerr << "Captured stderr:\n" << output << std::endl;

    // エラー文の一部を検出
    EXPECT_NE(output.find("[ server removed: server or location block member error ]"), std::string::npos)
        << "Expected error message not found. Actual output:\n[" << output << "]";

    std::remove(confFile.c_str());
}

// Test error handling - no host in server block
TEST_F(ConfigParserTest, NoHostInServerError) {
    std::string configText = R"(
server {
    listen 80;
    location / {
        root /;
        index index.html;
    }
}
)";

    const std::string dir = "./config_file";
    const std::string confFile = dir + "/temp_test.conf";

    // ディレクトリがなければ作成（0700 = 所有者に読み書き実行権限）
    mkdir(dir.c_str(), 0700);

    std::ofstream ofs(confFile.c_str());
    ASSERT_TRUE(ofs.is_open()) << "Failed to open config file for writing";
    ofs << configText;
    ofs.close();

    testing::internal::CaptureStderr();
    Config config(confFile);  // ← コンストラクタで checkAndEraseServerNode() 実行
    std::string output = testing::internal::GetCapturedStderr();

    // デバッグ表示
    std::cerr << "Captured stderr:\n" << output << std::endl;

    // エラー文の一部を検出
    EXPECT_NE(output.find("[ server removed: server or location block member error ]"), std::string::npos)
        << "Expected error message not found. Actual output:\n[" << output << "]";

    std::remove(confFile.c_str());
}

// Test error handling - no location in server block
TEST_F(ConfigParserTest, NoLocationInServerError) {
    std::string configText = R"(
server {
    listen 80;
    host localhost;
}
)";

    const std::string dir = "./config_file";
    const std::string confFile = dir + "/temp_test.conf";

    // ディレクトリがなければ作成（0700 = 所有者に読み書き実行権限）
    mkdir(dir.c_str(), 0700);

    std::ofstream ofs(confFile.c_str());
    ASSERT_TRUE(ofs.is_open()) << "Failed to open config file for writing";
    ofs << configText;
    ofs.close();

    testing::internal::CaptureStderr();
    Config config(confFile);  // ← コンストラクタで checkAndEraseServerNode() 実行
    std::string output = testing::internal::GetCapturedStderr();

    // デバッグ表示
    std::cerr << "Captured stderr:\n" << output << std::endl;

    // エラー文の一部を検出
    EXPECT_NE(output.find("[ server removed: server or location block member error ]"), std::string::npos)
        << "Expected error message not found. Actual output:\n[" << output << "]";

    std::remove(confFile.c_str());
}

// Test error handling - no root in location block
TEST_F(ConfigParserTest, NoRootInLocationError) {
    std::string configText = R"(
server {
    listen 80;
    host localhost;
    location / {
        index index.html;
    }
}
)";

    const std::string dir = "./config_file";
    const std::string confFile = dir + "/temp_test.conf";

    // ディレクトリがなければ作成（0700 = 所有者に読み書き実行権限）
    mkdir(dir.c_str(), 0700);

    std::ofstream ofs(confFile.c_str());
    ASSERT_TRUE(ofs.is_open()) << "Failed to open config file for writing";
    ofs << configText;
    ofs.close();

    testing::internal::CaptureStderr();
    Config config(confFile);  // ← コンストラクタで checkAndEraseServerNode() 実行
    std::string output = testing::internal::GetCapturedStderr();

    // デバッグ表示
    std::cerr << "Captured stderr:\n" << output << std::endl;

    // エラー文の一部を検出
    EXPECT_NE(output.find("[ server removed: server or location block member error ]"), std::string::npos)
        << "Expected error message not found. Actual output:\n[" << output << "]";

    std::remove(confFile.c_str());
}

// Test error handling - no index in location block
TEST_F(ConfigParserTest, NoIndexInLocationError) {
    std::string configText = R"(
server {
    listen 80;
    host localhost;
    location / {
        root /;
    }
}
)";

    const std::string dir = "./config_file";
    const std::string confFile = dir + "/temp_test.conf";

    // ディレクトリがなければ作成（0700 = 所有者に読み書き実行権限）
    mkdir(dir.c_str(), 0700);

    std::ofstream ofs(confFile.c_str());
    ASSERT_TRUE(ofs.is_open()) << "Failed to open config file for writing";
    ofs << configText;
    ofs.close();

    testing::internal::CaptureStderr();
    Config config(confFile);  // ← コンストラクタで checkAndEraseServerNode() 実行
    std::string output = testing::internal::GetCapturedStderr();

    // デバッグ表示
    std::cerr << "Captured stderr:\n" << output << std::endl;

    // エラー文の一部を検出
    EXPECT_NE(output.find("[ server removed: server or location block member error ]"), std::string::npos)
        << "Expected error message not found. Actual output:\n[" << output << "]";

    std::remove(confFile.c_str());
}

// Test static throwErr method
TEST_F(ConfigParserTest, ThrowErrMethod) {
    EXPECT_THROW(
        { ConfigParser::throwErr("test", " error: line ", 42); },
        std::runtime_error);

    try {
        ConfigParser::throwErr("test", " error: line ", 42);
    } catch (const std::runtime_error& e) {
        std::string expected = "test error: line 42";
        EXPECT_EQ(std::string(e.what()), expected);
    }
}

// Test empty configuration
TEST_F(ConfigParserTest, EmptyConfiguration) {
    auto tokenizer = createTokenizerFromConfig("");
    ConfigParser parser(*tokenizer);
    const auto& servers = parser.getServer();

    EXPECT_EQ(servers.size(), 0);
}
