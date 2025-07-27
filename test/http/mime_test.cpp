#include <gtest/gtest.h>
#include "http/mime.hpp"

TEST(MimeTest, GetMimeType) {
    EXPECT_EQ(http::getMimeType("index.html"), "text/html");
    EXPECT_EQ(http::getMimeType("style.css"), "text/css");
    EXPECT_EQ(http::getMimeType("script.js"), "application/javascript");
    EXPECT_EQ(http::getMimeType("data.json"), "application/json");
    EXPECT_EQ(http::getMimeType("image.png"), "image/png");
    EXPECT_EQ(http::getMimeType("image.jpg"), "image/jpeg");
    EXPECT_EQ(http::getMimeType("document.txt"), "text/plain");
    EXPECT_EQ(http::getMimeType("archive.zip"), "application/octet-stream");
    EXPECT_EQ(http::getMimeType("noextension"), "application/octet-stream");
    EXPECT_EQ(http::getMimeType(".htaccess"), "application/octet-stream");
    EXPECT_EQ(http::getMimeType("file.TXT"), "text/plain"); // Case-insensitivity
}
