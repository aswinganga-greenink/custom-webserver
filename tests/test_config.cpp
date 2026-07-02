#include <gtest/gtest.h>

#include <sstream>
#include <string>

#include "config.hpp"

TEST(ConfigParserTest, ExtractsVariablesCorrectly) {
    std::string mock_file_data =
        "port=8080\n"
        "worker_threads=8\n"
        "document_root=/var/www/html\n"
        "proxy_route=/api\n"
        "proxy_target_ip=127.0.0.1\n"
        "proxy_target_port=3000\n";

    std::stringstream stream(mock_file_data);
    ConfigParser      config;
    config.load_from_stream(stream);

    EXPECT_EQ(config.port, 8080);
    EXPECT_EQ(config.worker_threads, 8);
    EXPECT_EQ(config.document_root, "/var/www/html");
    EXPECT_EQ(config.proxy_route, "/api");
    EXPECT_EQ(config.proxy_target_ip, "127.0.0.1");
    EXPECT_EQ(config.proxy_target_port, 3000);
}

TEST(ConfigParserTest, IgnoresCommentsAndWhitespace) {
    std::string mock_file_data =
        "# This is a comment\n"
        "   port   =   9000   \n";

    std::stringstream stream(mock_file_data);
    ConfigParser      config;
    config.load_from_stream(stream);

    EXPECT_EQ(config.port, 9000);
}