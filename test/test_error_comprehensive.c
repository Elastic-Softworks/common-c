/*

    ===================================================

        COMMON-C TEST MODULE: ERROR COMPREHENSIVE TESTS
           ---
           comprehensive test coverage for error.c
           following established methodology template

    ===================================================
    
    Public Functions Tested:
    
    - commc_error_message()
    - commc_assert()
    - commc_report_error()
    - COMMC_ASSERT() (macro)
    
*/

#include    <stdio.h>
#include    <string.h>
#include    <stdlib.h>

#include    "test_harness.h"
#include    "../include/commc/error.h"

/*
    ==================================
             TEST FUNCTIONS
    ==================================
*/

/*

    test_error_message_valid_codes()
    ---
    tests error message retrieval for valid error codes

*/

void test_error_message_valid_codes(void) {

    const char* message;
    
    printf("\nTESTING: ERROR MESSAGE VALID CODES...\n\n");
    
    /* test all valid error codes */

    message = commc_error_message(COMMC_SUCCESS);
    ASSERT_NOT_NULL(message);
    ASSERT_TRUE(strlen(message) > 0);
    printf("  SUCCESS: '%s'\n", message);
    
    message = commc_error_message(COMMC_FAILURE);
    ASSERT_NOT_NULL(message);
    ASSERT_TRUE(strlen(message) > 0);
    printf("  FAILURE: '%s'\n", message);
    
    message = commc_error_message(COMMC_MEMORY_ERROR);
    ASSERT_NOT_NULL(message);
    ASSERT_TRUE(strlen(message) > 0);
    printf("  MEMORY_ERROR: '%s'\n", message);
    
    message = commc_error_message(COMMC_ARGUMENT_ERROR);
    ASSERT_NOT_NULL(message);
    ASSERT_TRUE(strlen(message) > 0);
    printf("  ARGUMENT_ERROR: '%s'\n", message);
    
    message = commc_error_message(COMMC_IO_ERROR);
    ASSERT_NOT_NULL(message);
    ASSERT_TRUE(strlen(message) > 0);
    printf("  IO_ERROR: '%s'\n", message);
    
    message = commc_error_message(COMMC_SYSTEM_ERROR);
    ASSERT_NOT_NULL(message);
    ASSERT_TRUE(strlen(message) > 0);
    printf("  SYSTEM_ERROR: '%s'\n", message);
    
    printf("PASSED: ERROR MESSAGE VALID CODES...\n\n");

}

/*

    test_error_message_invalid_codes()
    ---
    tests error message retrieval for invalid error codes

*/

void test_error_message_invalid_codes(void) {

    const char* message;
    
    printf("\nTESTING: ERROR MESSAGE INVALID CODES...\n\n");
    
    /* test invalid error codes - should return "unknown error" */

    message = commc_error_message(-1);  /* negative */
    ASSERT_NOT_NULL(message);
    ASSERT_TRUE(strlen(message) > 0);
    printf("  INVALID (-1): '%s'\n", message);
    
    message = commc_error_message(999);  /* too high */
    ASSERT_NOT_NULL(message);
    ASSERT_TRUE(strlen(message) > 0);
    printf("  INVALID (999): '%s'\n", message);
    
    /* both should be the same "unknown error" message */

    ASSERT_TRUE(strcmp(commc_error_message(-1), commc_error_message(999)) == 0);
    
    printf("PASSED: ERROR MESSAGE INVALID CODES\n\n");

}

/*

    test_error_message_content()
    ---
    tests that error messages contain expected keywords

*/

void test_error_message_content(void) {

    const char* message;
    
    printf("\nTESTING: ERROR MESSAGE CONTENT...\n\n");
    
    /* test that messages contain expected keywords */
    message = commc_error_message(COMMC_SUCCESS);
    ASSERT_TRUE(strstr(message, "no") != NULL || strstr(message, "success") != NULL);
    
    message = commc_error_message(COMMC_FAILURE);
    ASSERT_TRUE(strstr(message, "failure") != NULL);
    
    message = commc_error_message(COMMC_MEMORY_ERROR);
    ASSERT_TRUE(strstr(message, "memory") != NULL);
    
    message = commc_error_message(COMMC_ARGUMENT_ERROR);
    ASSERT_TRUE(strstr(message, "argument") != NULL);
    
    message = commc_error_message(COMMC_IO_ERROR);
    ASSERT_TRUE(strstr(message, "I/O") != NULL || strstr(message, "file") != NULL);
    
    message = commc_error_message(COMMC_SYSTEM_ERROR);
    ASSERT_TRUE(strstr(message, "system") != NULL);
    
    printf("PASSED: ERROR MESSAGE CONTENT\n\n");

}

/*

    test_assert_success_cases()
    ---
    tests commc_assert with conditions that should pass

*/

void test_assert_success_cases(void) {

    commc_error_t result;
    
    printf("\nTESTING: ASSERT SUCCESS CASES...\n\n");
    
    /* test assertions that should pass */

    result = commc_assert(1, "should pass");
    ASSERT_TRUE(result == COMMC_SUCCESS);
    
    result = commc_assert(42, "non-zero should pass");
    ASSERT_TRUE(result == COMMC_SUCCESS);
    
    result = commc_assert(-1, "negative non-zero should pass");
    ASSERT_TRUE(result == COMMC_SUCCESS);
    
    result = commc_assert(1 == 1, "equality should pass");
    ASSERT_TRUE(result == COMMC_SUCCESS);
    
    result = commc_assert(5 > 3, "comparison should pass");
    ASSERT_TRUE(result == COMMC_SUCCESS);
    
    printf("PASSED: ASSERT SUCCESS CASES\n\n");

}

/*

    test_assert_failure_cases()
    ---
    tests commc_assert with conditions that should fail

*/

void test_assert_failure_cases(void) {

    commc_error_t result;
    
    printf("\nTESTING: ASSERT FAILURE CASES...\n\n");
    
    /* test assertions that should fail */

    result = commc_assert(0, "zero should fail");
    ASSERT_TRUE(result == COMMC_FAILURE);
    
    result = commc_assert(1 == 0, "false equality should fail");
    ASSERT_TRUE(result == COMMC_FAILURE);
    
    result = commc_assert(3 > 5, "false comparison should fail");
    ASSERT_TRUE(result == COMMC_FAILURE);
    
    result = commc_assert(NULL != NULL, "null comparison should fail");
    ASSERT_TRUE(result == COMMC_FAILURE);
    
    printf("PASSED: ASSERT FAILURE CASES\n\n");

}

/*

    test_assert_with_null_message()
    ---
    tests commc_assert with null message parameter

*/

void test_assert_with_null_message(void) {

    commc_error_t result;
    
    printf("\nTESTING: ASSERT WITH NULL MESSAGE...\n\n");
    
    /* test assertion with null message (should handle gracefully) */

    result = commc_assert(1, NULL);
    ASSERT_TRUE(result == COMMC_SUCCESS);
    
    result = commc_assert(0, NULL);
    ASSERT_TRUE(result == COMMC_FAILURE);
    
    printf("PASSED: ASSERT WITH NULL MESSAGE\n\n");

}

/*

    test_report_error_valid_codes()
    ---
    tests commc_report_error with valid error codes

*/

void test_report_error_valid_codes(void) {

    printf("\nTESTING: REPORT ERROR VALID CODES...\n\n");
    
    /* test reporting all valid error codes */

    printf("  TESTING ERROR REPORTING (OUTPUT TO STDERR):\n");
    commc_report_error(COMMC_SUCCESS, "test_file.c", 123);
    commc_report_error(COMMC_FAILURE, "test_file.c", 124);
    commc_report_error(COMMC_MEMORY_ERROR, "test_file.c", 125);
    commc_report_error(COMMC_ARGUMENT_ERROR, "test_file.c", 126);
    commc_report_error(COMMC_IO_ERROR, "test_file.c", 127);
    commc_report_error(COMMC_SYSTEM_ERROR, "test_file.c", 128);
    
    /* no direct way to test stderr output, but if we get here without crashing, it worked */

    printf("PASSED: REPORT ERROR VALID CODES\n\n");

}

/*

    test_report_error_invalid_codes()
    ---
    tests commc_report_error with invalid error codes

*/

void test_report_error_invalid_codes(void) {

    printf("\nTESTING: REPORT ERROR INVALID CODES...\n\n");
    
    /* test reporting invalid error codes (should handle gracefully) */

    printf("  TESTING INVALID ERROR CODES (OUTPUT TO STDERR):\n");
    commc_report_error(-1, "test_file.c", 200);
    commc_report_error(999, "test_file.c", 201);
    
    printf("PASSED: REPORT ERROR INVALID CODES\n\n");

}

/*

    test_report_error_null_file()
    ---
    tests commc_report_error with null file parameter

*/

void test_report_error_null_file(void) {

    printf("\nTESTING: REPORT ERROR NULL FILE...\n\n");
    
    /* test reporting with null file (should handle gracefully) */

    printf("  TESTING NULL FILE PARAMETER (OUTPUT TO STDERR):\n");
    commc_report_error(COMMC_FAILURE, NULL, 300);
    
    printf("PASSED: REPORT ERROR NULL FILE\n\n");

}

/*

    test_report_error_negative_line()
    ---
    tests commc_report_error with negative line numbers

*/

void test_report_error_negative_line(void) {

    printf("\nTESTING: REPORT ERROR NEGATIVE LINE...\n\n");
    
    /* test reporting with negative line numbers (should handle gracefully) */
    printf("  TESTING NEGATIVE LINE NUMBERS (OUTPUT TO STDERR):\n");
    commc_report_error(COMMC_FAILURE, "test_file.c", -1);
    commc_report_error(COMMC_FAILURE, "test_file.c", -999);
    
    printf("PASSED: REPORT ERROR NEGATIVE LINE\n\n");

}

/*

    test_macro_assert_success()
    ---
    tests COMMC_ASSERT macro with conditions that should pass

*/

void test_macro_assert_success(void) {

    printf("\nTESTING: COMMC_ASSERT MACRO SUCCESS...\n\n");
    
    /* test macro assertions that should pass (no output expected) */

    COMMC_ASSERT(1, "macro should pass");
    COMMC_ASSERT(42, "macro non-zero should pass");
    COMMC_ASSERT(1 == 1, "macro equality should pass");
    COMMC_ASSERT(5 > 3, "macro comparison should pass");
    
    printf("PASSED: COMMC_ASSERT MACRO SUCCESS\n\n");

}

/*

    test_macro_assert_failure()
    ---
    tests COMMC_ASSERT macro with conditions that should fail

*/

void test_macro_assert_failure(void) {

    printf("\nTESTING: COMMC_ASSERT MACRO FAILURE...\n\n");
    
    printf("  TESTING MACRO FAILURES (OUTPUT TO STDERR EXPECTED):\n");
    
    /* test macro assertions that should fail (output to stderr expected) */

    COMMC_ASSERT(0, "macro zero should fail");
    COMMC_ASSERT(1 == 0, "macro false equality should fail");
    COMMC_ASSERT(3 > 5, "macro false comparison should fail");
    
    printf("PASSED: COMMC_ASSERT MACRO FAILURE\n\n");

}

/*

    test_error_enum_completeness()
    ---
    tests that all error enum values have corresponding messages

*/

void test_error_enum_completeness(void) {

    int i;
    const char* message;
    
    printf("\nTESTING: ERROR ENUM COMPLETENESS...\n\n");
    
    /* test that we have messages for all error codes from 0 to COMMC_SYSTEM_ERROR */

    for (i = COMMC_SUCCESS; i <= COMMC_SYSTEM_ERROR; i++) {

        message = commc_error_message((commc_error_t)i);
        ASSERT_NOT_NULL(message);
        ASSERT_TRUE(strlen(message) > 0);
        ASSERT_TRUE(strstr(message, "unknown") == NULL);  /* shouldn't be "unknown error" */
        printf("  CODE %d: '%s'\n", i, message);

    }
    
    printf("PASSED: ERROR ENUM COMPLETENESS\n\n");

}

/*

    test_complex_error_scenario()
    ---
    tests a complex scenario combining multiple error operations

*/

void test_complex_error_scenario(void) {

    commc_error_t result;
    const char* message;
    
    printf("\nTESTING: COMPLEX ERROR SCENARIO...\n\n");
    
    /* simulate a complex error handling scenario */
    
    /* step 1: successful operation */

    result = commc_assert(1, "initial operation successful");
    ASSERT_TRUE(result == COMMC_SUCCESS);
    message = commc_error_message(result);
    printf("  STEP 1: %s - %s\n", (result == COMMC_SUCCESS ? "SUCCESS" : "FAILURE"), message);
    
    /* step 2: argument validation failure */

    result = commc_assert(NULL != NULL, "null pointer validation");
    ASSERT_TRUE(result == COMMC_FAILURE);

    if (result != COMMC_SUCCESS) {

        commc_report_error(COMMC_ARGUMENT_ERROR, __FILE__, __LINE__);

    }
    
    /* step 3: memory allocation simulation */

    result = commc_assert(malloc(100) != NULL, "memory allocation check");
    
    if (result == COMMC_SUCCESS) {

        printf("  STEP 3: MEMORY ALLOCATION SUCCESSFUL\n");

    } else {

        commc_report_error(COMMC_MEMORY_ERROR, __FILE__, __LINE__);

    }
    
    /* step 4: macro assertion */

    COMMC_ASSERT(1, "final validation");
    
    /* step 5: test error message for all codes encountered */

    message = commc_error_message(COMMC_ARGUMENT_ERROR);
    ASSERT_NOT_NULL(message);
    printf("  ARGUMENT ERROR MESSAGE: %s\n", message);
    
    message = commc_error_message(COMMC_MEMORY_ERROR);
    ASSERT_NOT_NULL(message);
    printf("  MEMORY ERROR MESSAGE: %s\n", message);
    
    printf("PASSED: COMPLEX ERROR SCENARIO\n\n");

}

/*
    ==================================
             MAIN TEST RUNNER
    ==================================
*/

int main(void) {

    printf("\n");
    printf("========================================\n");
    printf("  COMMON-C ERROR COMPREHENSIVE TESTS\n");
    printf("========================================\n\n");

    MEMORY_LEAK_CHECK_START();

    /* error message tests */

    ADD_TEST(error_message_valid_codes);
    ADD_TEST(error_message_invalid_codes);
    ADD_TEST(error_message_content);
    ADD_TEST(error_enum_completeness);

    /* assertion tests */

    ADD_TEST(assert_success_cases);
    ADD_TEST(assert_failure_cases);
    ADD_TEST(assert_with_null_message);

    /* error reporting tests */

    ADD_TEST(report_error_valid_codes);
    ADD_TEST(report_error_invalid_codes);
    ADD_TEST(report_error_null_file);
    ADD_TEST(report_error_negative_line);

    /* macro tests */

    ADD_TEST(macro_assert_success);
    ADD_TEST(macro_assert_failure);

    /* integration tests */

    ADD_TEST(complex_error_scenario);

    RUN_ALL_TESTS();

    MEMORY_LEAK_CHECK_END();

    printf("========================================\n");
    printf("  ALL ERROR TESTS COMPLETED\n");
    printf("========================================\n\n");

    return 0;

}

/*
    ==================================
             --- EOF ---
    ==================================
*/