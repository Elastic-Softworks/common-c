/* ============================================================================
 *  COMMON-C TEST TEMPLATE
 *  EXAMPLE MODULE TEST FILE
 *  ELASTIC SOFTWORKS 2025
 *
 *  this file serves as a template for creating new test suites for the
 *  modules in the COMMON-C library. it demonstrates the basic structure,
 *  including how to include the test harness, define test cases, and
 *  use the assertion macros.
 *
 *  INSTRUCTIONS:
 *  1.  copy this file to a new file named `test_MODULENAME.c`.
 *  2.  replace "template" in the test case names with the module name.
 *  3.  include the header for the module you are testing.
 *  4.  add your test cases, using the existing ones as a guide.
 *  5.  add the new test file to the makefile's test target.
 * ========================================================================= */

#include "test_harness.h"

/* #include "commc/your_module.h" */

/* --- TEST CASES --- */

TEST_CASE(template_example_success)

    /* this is an example of a test that should pass */

    int a = 5;
    int b = 5;
    ASSERT_TRUE(a == b);
    ASSERT_INT_EQUALS(5, a);
    ASSERT_STRING_EQUALS("hello", "hello");
    ASSERT_NOT_NULL(&a);

END_TEST

TEST_CASE(template_example_failure)

    /* this is an example of a test that should fail */

    int x = 10;
    int y = 12;
    char* ptr = NULL;
    ASSERT_INT_EQUALS(x, y);        /* this will fail */
    ASSERT_FALSE(x < y);            /* this will fail */
    ASSERT_NOT_NULL(ptr);           /* this will fail */

END_TEST

/* --- TEST RUNNER --- */

int main(void) {

    PRINT_TEST_BANNER();

    /* add all test cases to be run here */

    ADD_TEST(template_example_success);
    ADD_TEST(template_example_failure);

    RUN_ALL_TESTS();
    PRINT_TEST_SUMMARY();

    /* return 1 if any tests failed, 0 otherwise */

    return commc_tests_failed > 0 ? 1 : 0;

}
