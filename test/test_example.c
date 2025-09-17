/* ============================================================================
 *  COMMON-C EXAMPLE TEST
 *  HARNESS TEST FILE
 *  ELASTIC SOFTWORKS 2025
 *
 *  this file demonstrates the basic usage of the test harness and serves
 *  as a simple check to ensure the testing framework itself is working
 *  correctly.
 * ========================================================================= */

#include "test_harness.h"

/* --- TEST CASES --- */

TEST_CASE(harness_check_success)

    /* this test confirms that basic assertions are working */

    int value = 1;
    const char* text = "common-c";

    ASSERT_TRUE(1);
    ASSERT_FALSE(0);
    ASSERT_INT_EQUALS(1, value);
    ASSERT_STRING_EQUALS("common-c", text);
    ASSERT_NOT_NULL(&value);

END_TEST

TEST_CASE(harness_check_null)

    /* this test confirms null pointer checks are working */

    void* ptr = NULL;
    ASSERT_NULL(ptr);

END_TEST


/* --- TEST RUNNER --- */

int main(void) {

    PRINT_TEST_BANNER();

    /* add all test cases to be run here */

    ADD_TEST(harness_check_success);
    ADD_TEST(harness_check_null);

    RUN_ALL_TESTS();
    PRINT_TEST_SUMMARY();

    /* return 1 if any tests failed, 0 otherwise */

    return commc_tests_failed > 0 ? 1 : 0;

}
