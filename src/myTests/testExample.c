#include "../procnanny.c"
#include <../../googletest/googletest/gtest/gtest.h>

// Mock out external dependency on mylogger.o
void Logger_log(...){}

TEST(FactorialTest, Zero) {
    EXPECT_EQ(1, Factorial(0));
}