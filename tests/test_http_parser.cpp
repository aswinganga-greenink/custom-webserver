#include <gtest/gtest.h>

#include <string>

#include "httprequest.hpp"

TEST(HttpRequestTest, ParsesValidGetRequest) {
    std::string raw_data =
        "GET /css/style.css HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "Connection: keep-alive\r\n"
        "Accept: text/css\r\n"
        "\r\n";

    HttpRequest req;
    bool        success = req.parse(raw_data);

    EXPECT_TRUE(success);

    EXPECT_EQ(req.method, "GET");
    EXPECT_EQ(req.uri, "/css/style.css");
    EXPECT_EQ(req.version, "HTTP/1.1");

    EXPECT_EQ(req.headers["Host"], "localhost:8080");
    EXPECT_EQ(req.headers["Connection"], "keep-alive");
    EXPECT_EQ(req.headers["Accept"], "text/css");
}

TEST(HttpRequestTest, RejectsMalformedRequest) {
    std::string bad_data = "THIS IS NOT HTTP\r\n\r\n";

    HttpRequest req;
    bool        success = req.parse(bad_data);

    EXPECT_FALSE(success);
}