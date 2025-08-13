#include <unity.h>

#include <unity.h>
#include "../src/app/notes.h"

void test_notes15_constructor()
{
    unsigned long timestamps[15] = {1, 2, 3, 4, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    const Notes15 notes15(timestamps);

    TEST_ASSERT_EQUAL(1, notes15.get(0));
    TEST_ASSERT_EQUAL(2, notes15.get(1));
    TEST_ASSERT_EQUAL(5, notes15.get(4));
    TEST_ASSERT_EQUAL(0, notes15.get(5));
}

void test_notes15_get_bounds()
{
    unsigned long timestamps[15] = {0};
    const Notes15 notes15(timestamps);

    TEST_ASSERT_EQUAL(0, notes15.get(-1));
    TEST_ASSERT_EQUAL(0, notes15.get(15));
    TEST_ASSERT_EQUAL(0, notes15.get(0));
}

void test_notes15_inequality()
{
    unsigned long timestamps1[15] = {1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    unsigned long timestamps2[15] = {1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    unsigned long timestamps3[15] = {1, 2, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    const Notes15 notes1(timestamps1);
    const Notes15 notes2(timestamps2);
    const Notes15 notes3(timestamps3);

    TEST_ASSERT_FALSE(notes1 != notes2);
    TEST_ASSERT_TRUE(notes1 != notes3);
}

void test_notes15_filter()
{
    unsigned long timestamps[15] = {7, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0};
    const Notes15 notes15(timestamps);
    Notes15Filter filter;

    const Notes15 result = filter.latest(notes15, 3);

    TEST_ASSERT_EQUAL(7, result.get(0));
    TEST_ASSERT_EQUAL(6, result.get(1));
    TEST_ASSERT_EQUAL(5, result.get(2));
    TEST_ASSERT_EQUAL(0, result.get(3));
}

void setUp()
{
}

void tearDown()
{
}

int main()
{
    UNITY_BEGIN();

    RUN_TEST(test_notes15_constructor);
    RUN_TEST(test_notes15_get_bounds);
    RUN_TEST(test_notes15_inequality);
    RUN_TEST(test_notes15_filter);

    UNITY_END();
}
