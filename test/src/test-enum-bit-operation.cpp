#include <EnumBitOperation.hpp>

#include <gtest/gtest.h>

TEST(EnumBitOperation, CEnum)
{
    typedef enum EA
    {
        EA_0 = 0,
        EA_1 = 1,
        EA_2 = 2,
        EA_3 = 4,
    } EA;
    EXPECT_EQ(EA_0 & EA_1, 0);
    EXPECT_EQ(EA_0 | EA_1, 1);
    EXPECT_EQ(EA_1 | EA_2, static_cast<EA>(1 | 2));
    EXPECT_EQ(EA_1 & EA_2, static_cast<EA>(0));
    EXPECT_EQ(~EA_1, static_cast<EA>(~1));
    EXPECT_EQ(EA_1 ^ EA_2, static_cast<EA>(1 ^ 2));
    EXPECT_EQ(EA_1 << 1, static_cast<EA>(1 << 1));
    EXPECT_EQ(EA_2 >> 1, static_cast<EA>(2 >> 1));
}

TEST(EnumBitOperation, CPPEnum)
{
    enum EA : uint32_t
    {
        EA_0 = 0,
        EA_1 = 1,
        EA_2 = 2,
        EA_3 = 4,
    };

    EXPECT_EQ(EA_0 & EA_1, 0);
    EXPECT_EQ(EA_0 | EA_1, 1);
    EXPECT_EQ(EA_1 | EA_2, static_cast<EA>(1 | 2));
    EXPECT_EQ(EA_1 & EA_2, static_cast<EA>(0));
    EXPECT_EQ(~EA_1, static_cast<EA>(~1));
    EXPECT_EQ(EA_1 ^ EA_2, static_cast<EA>(1 ^ 2));
    EXPECT_EQ(EA_1 << 1, static_cast<EA>(1 << 1));
    EXPECT_EQ(EA_2 >> 1, static_cast<EA>(2 >> 1));
}