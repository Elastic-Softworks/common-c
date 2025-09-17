/* ============================================================================
 *  COMMON-C TEST HARNESS
 *  A lightweight, zero-dependency testing framework for C89.
 *  ELASTIC SOFTWORKS 2025
 *
 *  This header-only library provides a simple set of macros and functions
 *  for creating and running unit tests for the COMMON-C project. It is
 *  designed to be C89 compliant and easy to use, with a focus on clarity
 *  and educational value.
 *
 *  USAGE:
 *  1.  include this header in your test file.
 *  2.  define test cases using the TEST_CASE(name) ... END_TEST macros.
 *  3.  use the ASSERT_* macros inside your test cases to check conditions.
 *  4.  create a main function.
 *  5.  in main, register tests using ADD_TEST(name).
 *  6.  call RUN_ALL_TESTS() to execute them.
 *  7.  call PRINT_TEST_BANNER() at the start and PRINT_TEST_SUMMARY() at the end.
 *
 *  EXAMPLE:
 *      #include "test_harness.h"
 *
 *      TEST_CASE(example_test)
 *          int a = 1;
 *          ASSERT_TRUE(a == 1);
 *      END_TEST
 *
 *      int main(void) {
 *          PRINT_TEST_BANNER();
 *          ADD_TEST(example_test);
 *          RUN_ALL_TESTS();
 *          PRINT_TEST_SUMMARY();
 *          return commc_tests_failed > 0 ? 1 : 0;
 *      }
 * ========================================================================= */

#ifndef COMMC_TEST_HARNESS_H
#define COMMC_TEST_HARNESS_H

#include <stdio.h>
#include <string.h>

/* --- INTERNAL TEST STATE --- */
/* these variables track the state of the test run */

static int commc_tests_total = 0;
static int commc_tests_passed = 0;
static int commc_tests_failed = 0;

/* --- TEST REPORTING MACROS --- */
/* these macros are used to print formatted output for the test run */

#define PRINT_TEST_BANNER() \
    printf("====================================\n"); \
    printf("  C O M M O N - C   T E S T S\n"); \
    printf("====================================\n\n")

#define PRINT_TEST_SUMMARY() \
    printf("\n------------------------------------\n"); \
    printf("TEST SUMMARY:\n"); \
    printf("  TOTAL:  %d\n", commc_tests_total); \
    printf("  PASSED: %d\n", commc_tests_passed); \
    printf("  FAILED: %d\n", commc_tests_failed); \
    printf("====================================\n")

/* --- TEST DISCOVERY AND EXECUTION --- */
/* provides a simple test registration and execution system */

typedef void (*commc_test_func)(void);

static commc_test_func commc_test_suite[1024];
static const char* commc_test_names[1024];
static int commc_test_count = 0;

#define ADD_TEST(name) \
    do { \
        if (commc_test_count < 1024) { \
            commc_test_suite[commc_test_count] = test_##name; \
            commc_test_names[commc_test_count] = #name; \
            commc_test_count++; \
        } \
    } while (0)

#define RUN_ALL_TESTS() \
    do { \
        int i; \
        for (i = 0; i < commc_test_count; i++) { \
            printf("--- RUNNING TEST: %s ---\n", commc_test_names[i]); \
            commc_test_suite[i](); \
        } \
    } while (0)

/* --- TEST CASE DEFINITION MACROS --- */
/* use these macros to define a test case function */

#define TEST_CASE(name) \
    static void test_##name(void)

#define END_TEST /* serves as a syntactic marker */

/* --- ASSERTION MACROS --- */
/* use these macros within a TEST_CASE to validate conditions */

#define ASSERT_TRUE(condition) \
    do { \
        commc_tests_total++; \
        if (condition) { \
            printf("  [PASS] %s:%d: %s\n", __FILE__, __LINE__, #condition); \
            commc_tests_passed++; \
        } else { \
            printf("  [FAIL] %s:%d: %s\n", __FILE__, __LINE__, #condition); \
            commc_tests_failed++; \
        } \
    } while (0)

#define ASSERT_FALSE(condition) ASSERT_TRUE(!(condition))

#endif /* COMMC_TEST_HARNESS_H */
