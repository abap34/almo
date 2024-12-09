#include "gtest/gtest.h"
#include <gtest/gtest.h>
#include "src/parse.hpp"

TEST(BasicTest, Test) {
    EXPECT_EQ(
        1,
        1        
    );
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
