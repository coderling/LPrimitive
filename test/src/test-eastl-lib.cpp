#include <EASTL/allocator.h>
#include <EASTL/string.h>
#include <EASTL/vector.h>
#include <gtest/gtest.h>
#include <EAAllocatorImpl.hpp>

TEST(TestEASTLLIB, test_eastl)
{
    auto vec = eastl::vector<int>();
    vec.push_back(1);
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec.size(), 1);

    using C_A = CDL::Primitive::EAAllocator<CDL::Primitive::DefaultMemoryAllocator>;
    auto vec_1 = eastl::vector<int, C_A>(C_A{CDL::Primitive::GetGlobalAllocator()});
    vec_1.push_back(2);
    EXPECT_EQ(vec_1[0], 2);
    EXPECT_EQ(vec_1.size(), 1);
    vec_1.clear();
    EXPECT_EQ(vec_1.size(), 0);
}