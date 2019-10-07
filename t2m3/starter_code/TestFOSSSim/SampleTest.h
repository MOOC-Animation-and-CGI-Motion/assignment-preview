#ifndef __SAMPLE_TEST_H__
#define __SAMPLE_TEST_H__

#include <gtest/gtest.h>
#include <string>

TEST(TestTestOne, One)
{
  ASSERT_EQ(NULL,NULL);
  EXPECT_EQ(1,1);
  EXPECT_NE(-1,1);
  EXPECT_TRUE(true);
}

TEST(TestTestOne, TWO)
{
  EXPECT_EQ(std::string("123"),std::string("123"));
}

TEST(TestTestTwo, THREE)
{
  EXPECT_NE(std::string("123"),std::string("456"));
}

#endif
