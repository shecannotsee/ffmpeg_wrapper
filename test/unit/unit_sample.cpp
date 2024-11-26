#include <gtest/gtest.h>
#include <cal.h>

TEST(unit, sample) {
  EXPECT_EQ(cal<int>::add(2,3), 5);
}