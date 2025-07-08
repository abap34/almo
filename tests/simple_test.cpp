#include <gtest/gtest.h>
#include <fstream>
#include <sstream>

// 簡単なAPIテスト
TEST(SimpleTest, BasicAssertions) {
    EXPECT_EQ(1 + 1, 2);
    EXPECT_TRUE(true);
    EXPECT_FALSE(false);
}

TEST(SimpleTest, StringOperations) {
    std::string test = "hello";
    EXPECT_EQ(test.length(), 5);
    EXPECT_EQ(test + " world", "hello world");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}