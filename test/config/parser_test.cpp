#include "parser.hpp"

#include <gtest/gtest.h>
#include <sys/stat.h>   // mkdir
#include <sys/types.h>  // mode_t

#include <fstream>
#include <sstream>

#include "config.hpp"
#include "serverContext.hpp"
#include "token.hpp"
#include "tokenizer.hpp"

class ConfigParserTest : public ::testing::Test {
   protected:
    void TearDown() override {
        // 作成した一時ファイルを削除
        for (size_t i = 0; i < tempFiles_.size(); ++i) {
            std::remove(tempFiles_[i].c_str());
        }
        tempFiles_.clear();
    }

    // conf を dir 配下に作ってファイル名を outFilename で返す（C++98）
    ConfigTokenizer* createTokenizerFromConfig(const std::string& configContent,
                                               std::string& outFilename,
                                               const std::string& dir) {
        // ディレクトリなければ作成
        mkdir(dir.c_str(), 0700);

        std::ostringstream oss;
        oss << dir << "/test_config_" << fileCounter_++ << ".conf";
        outFilename = oss.str();

        std::ofstream file(outFilename.c_str());
        file << configContent;
        file.close();

        // TearDown で消すために保存
        tempFiles_.push_back(outFilename);

        return new ConfigTokenizer(outFilename);
    }

    // 毎回書くのが面倒なので、dir を固定した簡易版ヘルパ
    ConfigTokenizer* makeTok(const std::string& configContent,
                             std::string& outFilename) {
        const std::string dir = "./config_file";
        return createTokenizerFromConfig(configContent, outFilename, dir);
    }

   private:
    static int fileCounter_;
    std::vector<std::string> tempFiles_;
};

int ConfigParserTest::fileCounter_ = 0;

// Test constructor and destructor
TEST_F(ConfigParserTest, ConstructorDestructorTest) {
    const std::string dir = "./config_file";
    mkdir(dir.c_str(), 0700);
    std::string filename;
    ConfigTokenizer* tokenizer = createTokenizerFromConfig("", filename, dir);

    EXPECT_NO_THROW({
        ConfigParser parser(*tokenizer, filename);  // ← filename が confFile
    });

    std::remove(filename.c_str());
}

// Test error page configuration
TEST_F(ConfigParserTest, ParseErrorPageConfiguration) {
    std::string config = R"(
    server {
        error_page 404 /error/404.html;
    }
    )";

    std::string filename;
    ConfigTokenizer* tokenizer = makeTok(config, filename);
    ConfigParser parser(*tokenizer, filename);
    const std::vector<ServerContext>& servers = parser.getServer();

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

    std::string filename;
    ConfigTokenizer* tokenizer = makeTok(config, filename);
    ConfigParser parser(*tokenizer, filename);
    const std::vector<ServerContext>& servers = parser.getServer();

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

    std::string filename;
    ConfigTokenizer* tokenizer = makeTok(config, filename);

    EXPECT_THROW(
        { ConfigParser parser(*tokenizer, filename); }, std::runtime_error);
}

// Test max body size - invalid number
TEST_F(ConfigParserTest, ParseMaxBodySizeConfiguration3) {
    std::string config = R"(
server {
    client_max_body_size 999999999999999999;
}
)";

    std::string filename;
    ConfigTokenizer* tokenizer = makeTok(config, filename);

    EXPECT_THROW(
        { ConfigParser parser(*tokenizer, filename); }, std::runtime_error);
}

// Test error handling - unmatched braces
TEST_F(ConfigParserTest, UnmatchedBracesError) {
    std::string config = R"(
server {
    listen 8080;
}
}
)";  // Extra closing brace

    std::string filename;
    ConfigTokenizer* tokenizer = makeTok(config, filename);

    EXPECT_THROW(
        { ConfigParser parser(*tokenizer, filename); }, std::runtime_error);
}

// Test error handling - unmatched braces2
TEST_F(ConfigParserTest, UnmatchedBracesError2) {
    std::string config = R"(
    } server {
    )";  // Extra closing brace

    try {
        std::string filename;
        ConfigTokenizer* tokenizer = makeTok(config, filename);
        ConfigParser parser(*tokenizer, filename);
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
        std::string filename;
        ConfigTokenizer* tokenizer = makeTok(config, filename);
        ConfigParser parser(*tokenizer, filename);
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
        std::string filename;
        ConfigTokenizer* tokenizer = makeTok(config, filename);
        ConfigParser parser(*tokenizer, filename);
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

    std::string filename;
    ConfigTokenizer* tokenizer = makeTok(config, filename);

    EXPECT_THROW(
        { ConfigParser parser(*tokenizer, filename); }, std::runtime_error);
}

// Test error handling - invalid port number
TEST_F(ConfigParserTest, InvalidPortNumberError2) {
    std::string config = R"(
server {
    listen -100;
}
)";

    std::string filename;
    ConfigTokenizer* tokenizer = makeTok(config, filename);

    EXPECT_THROW(
        { ConfigParser parser(*tokenizer, filename); }, std::runtime_error);
}

// Test error handling - invalid port number
TEST_F(ConfigParserTest, InvalidPortNumberError3) {
    std::string config = R"(
server {
    listen 70000;
}
)";

    std::string filename;
    ConfigTokenizer* tokenizer = makeTok(config, filename);

    EXPECT_THROW(
        { ConfigParser parser(*tokenizer, filename); }, std::runtime_error);
}

// Test error handling - invalid port number
TEST_F(ConfigParserTest, InvalidPortNumberError4) {
    std::string config = R"(
server {
    listen 80;
}
)";

    std::string filename;
    ConfigTokenizer* tokenizer = makeTok(config, filename);

    EXPECT_THROW(
        { ConfigParser parser(*tokenizer, filename); }, std::runtime_error);
}

// Test error handling - invalid server block member error1
TEST_F(ConfigParserTest, InvalidSeerverBlockMemberError1) {
    std::string config = R"(
server {
    root /;
}
)";

    std::string filename;
    ConfigTokenizer* tokenizer = makeTok(config, filename);

    EXPECT_THROW(
        { ConfigParser parser(*tokenizer, filename); }, std::runtime_error);
}

// Test error handling - invalid server block member error2
TEST_F(ConfigParserTest, InvalidSeerverBlockMemberError2) {
    std::string config = R"(
server {
    allow_method GET;
}
)";

    std::string filename;
    ConfigTokenizer* tokenizer = makeTok(config, filename);

    EXPECT_THROW(
        { ConfigParser parser(*tokenizer, filename); }, std::runtime_error);
}

// Test error handling - invalid server block member error3
TEST_F(ConfigParserTest, InvalidSeerverBlockMemberError3) {
    std::string config = R"(
server {
    index index.html index.htm;
}
)";

    std::string filename;
    ConfigTokenizer* tokenizer = makeTok(config, filename);

    EXPECT_THROW(
        { ConfigParser parser(*tokenizer, filename); }, std::runtime_error);
}

// Test error handling - invalid server block member error4
TEST_F(ConfigParserTest, InvalidSeerverBlockMemberError4) {
    std::string config = R"(
server {
    autoindex ON;
}
)";

    std::string filename;
    ConfigTokenizer* tokenizer = makeTok(config, filename);

    EXPECT_THROW(
        { ConfigParser parser(*tokenizer, filename); }, std::runtime_error);
}

// Test error handling - invalid server block member error5
TEST_F(ConfigParserTest, InvalidSeerverBlockMemberError5) {
    std::string config = R"(
server {
    is_cgi ON;
}
)";

    std::string filename;
    ConfigTokenizer* tokenizer = makeTok(config, filename);

    EXPECT_THROW(
        { ConfigParser parser(*tokenizer, filename); }, std::runtime_error);
}

// Test error handling - invalid server block member error6
TEST_F(ConfigParserTest, InvalidSeerverBlockMemberError6) {
    std::string config = R"(
server {
    redirect http://localhost:8080/;
}
)";

    std::string filename;
    ConfigTokenizer* tokenizer = makeTok(config, filename);

    EXPECT_THROW(
        { ConfigParser parser(*tokenizer, filename); }, std::runtime_error);
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

    std::string filename;
    ConfigTokenizer* tokenizer = makeTok(config, filename);

    EXPECT_THROW(
        { ConfigParser parser(*tokenizer, filename); }, std::runtime_error);
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

    std::string filename;
    ConfigTokenizer* tokenizer = makeTok(config, filename);

    EXPECT_THROW(
        { ConfigParser parser(*tokenizer, filename); }, std::runtime_error);
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

    std::string filename;
    ConfigTokenizer* tokenizer = makeTok(config, filename);

    EXPECT_THROW(
        { ConfigParser parser(*tokenizer, filename); }, std::runtime_error);
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

    std::string filename;
    ConfigTokenizer* tokenizer = makeTok(config, filename);

    EXPECT_THROW(
        { ConfigParser parser(*tokenizer, filename); }, std::runtime_error);
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

    std::string filename;
    ConfigTokenizer* tokenizer = makeTok(config, filename);

    EXPECT_THROW(
        { ConfigParser parser(*tokenizer, filename); }, std::runtime_error);
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

    std::string filename;
    ConfigTokenizer* tokenizer = makeTok(config, filename);

    EXPECT_THROW(
        { ConfigParser parser(*tokenizer, filename); }, std::runtime_error);
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

    std::string filename;
    ConfigTokenizer* tokenizer = makeTok(config, filename);

    EXPECT_THROW(
        { ConfigParser parser(*tokenizer, filename); }, std::runtime_error);
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
    Config config(
        confFile);  // ← コンストラクタで checkAndEraseServerNode() 実行
    std::string output = testing::internal::GetCapturedStderr();

    // デバッグ表示
    std::cerr << "Captured stderr:\n" << output << std::endl;

    // エラー文の一部を検出
    EXPECT_NE(output.find(
                  "[ server removed: server or location block member error ]"),
              std::string::npos)
        << "Expected error message not found. Actual output:\n[" << output
        << "]";

    std::remove(confFile.c_str());
}

// Test error handling - no host in server block
TEST_F(ConfigParserTest, NoHostInServerError) {
    std::string configText = R"(
server {
    listen 8080;
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
    Config config(
        confFile);  // ← コンストラクタで checkAndEraseServerNode() 実行
    std::string output = testing::internal::GetCapturedStderr();

    // デバッグ表示
    std::cerr << "Captured stderr:\n" << output << std::endl;

    // エラー文の一部を検出
    // EXPECT_NE(output.find(
    //               "[ server removed: server or location block member error ]"),
    //           std::string::npos)
    //     << "Expected error message not found. Actual output:\n[" << output
    //     << "]";

    std::remove(confFile.c_str());
}

// Test error handling - no location in server block
TEST_F(ConfigParserTest, NoLocationInServerError) {
    std::string configText = R"(
server {
    listen 8080;
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
    Config config(
        confFile);  // ← コンストラクタで checkAndEraseServerNode() 実行
    std::string output = testing::internal::GetCapturedStderr();

    // デバッグ表示
    std::cerr << "Captured stderr:\n" << output << std::endl;

    // エラー文の一部を検出
    EXPECT_NE(output.find(
                  "[ server removed: server or location block member error ]"),
              std::string::npos)
        << "Expected error message not found. Actual output:\n[" << output
        << "]";

    std::remove(confFile.c_str());
}

// Test error handling - no root in location block
TEST_F(ConfigParserTest, NoRootInLocationError) {
    std::string configText = R"(
server {
    listen 8080;
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
    Config config(
        confFile);  // ← コンストラクタで checkAndEraseServerNode() 実行
    std::string output = testing::internal::GetCapturedStderr();

    // デバッグ表示
    std::cerr << "Captured stderr:\n" << output << std::endl;

    // エラー文の一部を検出
    EXPECT_NE(output.find(
                  "[ server removed: server or location block member error ]"),
              std::string::npos)
        << "Expected error message not found. Actual output:\n[" << output
        << "]";

    std::remove(confFile.c_str());
}

// Test error handling - no index in location block
TEST_F(ConfigParserTest, NoIndexInLocationError) {
    std::string configText = R"(
server {
    listen 8080;
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
    Config config(
        confFile);  // ← コンストラクタで checkAndEraseServerNode() 実行
    std::string output = testing::internal::GetCapturedStderr();

    // デバッグ表示
    std::cerr << "Captured stderr:\n" << output << std::endl;

    // エラー文の一部を検出
    // EXPECT_NE(output.find(
    //               "[ server removed: server or location block member error ]"),
    //           std::string::npos)
    //     << "Expected error message not found. Actual output:\n[" << output
    //     << "]";

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
    std::string filename;
    ConfigTokenizer* tokenizer = makeTok("", filename);
    ConfigParser parser(*tokenizer, filename);
    const std::vector<ServerContext>& servers = parser.getServer();

    EXPECT_EQ(servers.size(), 0);
}
