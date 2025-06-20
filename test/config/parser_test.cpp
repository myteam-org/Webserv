#include <gtest/gtest.h>
#include <fstream>
#include <sstream>
#include "parser.hpp"
#include "tokenizer.hpp"
#include "server.hpp"
#include "token.hpp"

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

    // Helper method to create a temporary config file and return ConfigTokenizer
    std::unique_ptr<ConfigTokenizer> createTokenizerFromConfig(const std::string& configContent) {
        // Create a temporary file
        std::string filename = "test_config_" + std::to_string(fileCounter_++) + ".conf";
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
    
    EXPECT_NO_THROW({
        ConfigParser parser(*tokenizer);
    });
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
    // Note: You'll need to add a getter method in ServerContext to test error pages
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



// Test error handling - unmatched braces
TEST_F(ConfigParserTest, UnmatchedBracesError) {
    std::string config = R"(
server {
    listen 8080;
}
}
)"; // Extra closing brace
    
    auto tokenizer = createTokenizerFromConfig(config);
    
    EXPECT_THROW({
        ConfigParser parser(*tokenizer);
    }, std::runtime_error);
}



// Test error handling - invalid port number
TEST_F(ConfigParserTest, InvalidPortNumberError) {
    std::string config = R"(
server {
    listen invalid_port;
}
)";
    
    auto tokenizer = createTokenizerFromConfig(config);
    
    EXPECT_THROW({
        ConfigParser parser(*tokenizer);
    }, std::runtime_error);
}


// Test static throwErr method
TEST_F(ConfigParserTest, ThrowErrMethod) {
    EXPECT_THROW({
        ConfigParser::throwErr("test", " error: line ", 42);
    }, std::runtime_error);
    
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
