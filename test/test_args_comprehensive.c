/* ============================================================================
 *  COMMON-C ARGS MODULE TEST - COMPREHENSIVE COVERAGE
 *  ELASTIC SOFTWORKS 2025
 *
 *  this file contains the complete test suite for the args module,
 *  providing comprehensive coverage of all public functions, edge cases,
 *  error conditions, and memory management scenarios.
 *
 *  this serves as the template for all PHASE II.B module testing,
 *  demonstrating proper use of the test harness, C-FORM documentation
 *  standards, and educational testing approaches.
 * ========================================================================= */

#include "test_harness.h"
#include "commc/args.h"

/* 
	==================================
             --- TEST CASES ---
	==================================
*/

/*

         test_args_parser_lifecycle()
	       ---
	       tests basic parser creation and destruction.
	       this verifies the fundamental memory management
	       of the parser object itself.

*/

TEST_CASE(test_args_parser_lifecycle)

    commc_args_parser_t* parser;

    MEMORY_LEAK_CHECK_START();

    /* create a parser instance and ensure it's not null */

    parser = commc_args_parser_create();
    ASSERT_NOT_NULL(parser);

    /* destroy the parser and verify proper cleanup */

    commc_args_parser_destroy(parser);

    /* verify no memory leaks occurred during lifecycle */

    ASSERT_NO_MEMORY_LEAKS();

END_TEST

/*

         test_args_null_safety()
	       ---
	       tests null pointer handling across all functions.
	       ensures robust error handling when invalid parameters
	       are passed to any public API function.

*/

TEST_CASE(test_args_null_safety)

    int result_int;
    const char* result_str;

    /* test null parser handling for all add functions */

    result_int = commc_args_add_flag(NULL, "v", "verbose", "test");
    ASSERT_TRUE(result_int == 0);

    result_int = commc_args_add_option(NULL, "o", "output", "test", "default");
    ASSERT_TRUE(result_int == 0);

    result_int = commc_args_add_positional(NULL, "file", "test");
    ASSERT_TRUE(result_int == 0);

    /* test null parser handling for parse function */

    result_int = commc_args_parse(NULL, 1, NULL);
    ASSERT_TRUE(result_int == 0);

    /* test null parser handling for get functions */

    result_int = commc_args_get_flag(NULL, "test");
    ASSERT_TRUE(result_int == 0);

    result_str = commc_args_get_option(NULL, "test");
    ASSERT_NULL(result_str);

    result_str = commc_args_get_positional(NULL, 0);
    ASSERT_NULL(result_str);

END_TEST

/*

         test_args_add_flag_success()
	       ---
	       tests successful flag addition with various combinations
	       of short names, long names, and descriptions.

*/

TEST_CASE(test_args_add_flag_success)

    commc_args_parser_t* parser;
    int result;

    MEMORY_LEAK_CHECK_START();

    parser = commc_args_parser_create();
    ASSERT_NOT_NULL(parser);

    /* test flag with both short and long names */

    result = commc_args_add_flag(parser, "v", "verbose", "Enable verbose output");
    ASSERT_TRUE(result == 1);

    /* test flag with only short name */

    result = commc_args_add_flag(parser, "h", NULL, "Show help");
    ASSERT_TRUE(result == 1);

    /* test flag with only long name */

    result = commc_args_add_flag(parser, NULL, "debug", "Enable debug mode");
    ASSERT_TRUE(result == 1);

    /* test flag with no description */

    result = commc_args_add_flag(parser, "q", "quiet", NULL);
    ASSERT_TRUE(result == 1);

    commc_args_parser_destroy(parser);
    ASSERT_NO_MEMORY_LEAKS();

END_TEST

/*

         test_args_add_option_success()
	       ---
	       tests successful option addition with various combinations
	       of short names, long names, descriptions, and default values.

*/

TEST_CASE(test_args_add_option_success)

    commc_args_parser_t* parser;
    int result;

    MEMORY_LEAK_CHECK_START();

    parser = commc_args_parser_create();
    ASSERT_NOT_NULL(parser);

    /* test option with all parameters */

    result = commc_args_add_option(parser, "o", "output", "Output file", "default.txt");
    ASSERT_TRUE(result == 1);

    /* test option with no default value */

    result = commc_args_add_option(parser, "c", "config", "Config file", NULL);
    ASSERT_TRUE(result == 1);

    /* test option with only short name */

    result = commc_args_add_option(parser, "f", NULL, "Input file", "input.dat");
    ASSERT_TRUE(result == 1);

    commc_args_parser_destroy(parser);
    ASSERT_NO_MEMORY_LEAKS();

END_TEST

/*

         test_args_add_positional_success()
	       ---
	       tests successful positional argument addition.

*/

TEST_CASE(test_args_add_positional_success)

    commc_args_parser_t* parser;
    int result;

    MEMORY_LEAK_CHECK_START();

    parser = commc_args_parser_create();
    ASSERT_NOT_NULL(parser);

    /* test multiple positional arguments */

    result = commc_args_add_positional(parser, "file1", "First input file");
    ASSERT_TRUE(result == 1);

    result = commc_args_add_positional(parser, "file2", "Second input file");
    ASSERT_TRUE(result == 1);

    result = commc_args_add_positional(parser, "output", "Output file");
    ASSERT_TRUE(result == 1);

    commc_args_parser_destroy(parser);
    ASSERT_NO_MEMORY_LEAKS();

END_TEST

/*

         test_args_parsing_success()
	       ---
	       tests successful parsing of a complex command line
	       with flags, options, and positional arguments.

*/

TEST_CASE(test_args_parsing_success)

    commc_args_parser_t* parser;
    char* argv[] = {"./program", "-v", "--output=results.txt", "input.dat", "config.ini"};
    int argc = 5;
    int result;

    MEMORY_LEAK_CHECK_START();

    parser = commc_args_parser_create();
    ASSERT_NOT_NULL(parser);

    /* add arguments */

    result = commc_args_add_flag(parser, "v", "verbose", "Enable verbose output");
    ASSERT_TRUE(result == 1);

    result = commc_args_add_option(parser, "o", "output", "Output file", "default.txt");
    ASSERT_TRUE(result == 1);

    result = commc_args_add_positional(parser, "input_file", "Input data file");
    ASSERT_TRUE(result == 1);

    result = commc_args_add_positional(parser, "config_file", "Configuration file");
    ASSERT_TRUE(result == 1);

    /* parse the command line */

    result = commc_args_parse(parser, argc, argv);
    ASSERT_TRUE(result == 1);

    commc_args_parser_destroy(parser);
    ASSERT_NO_MEMORY_LEAKS();

END_TEST

/*

         test_args_flag_retrieval()
	       ---
	       tests flag value retrieval after parsing.

*/

TEST_CASE(test_args_flag_retrieval)

    commc_args_parser_t* parser;
    char* argv1[] = {"./program", "-v", "-d"};
    char* argv2[] = {"./program"};
    int argc1 = 3;
    int result;

    MEMORY_LEAK_CHECK_START();

    parser = commc_args_parser_create();
    ASSERT_NOT_NULL(parser);

    /* add flags */

    result = commc_args_add_flag(parser, "v", "verbose", "Enable verbose output");
    ASSERT_TRUE(result == 1);

    result = commc_args_add_flag(parser, "d", "debug", "Enable debug mode");
    ASSERT_TRUE(result == 1);

    result = commc_args_add_flag(parser, "q", "quiet", "Enable quiet mode");
    ASSERT_TRUE(result == 1);

    /* parse command line with some flags set */

    result = commc_args_parse(parser, argc1, argv1);
    ASSERT_TRUE(result == 1);

    /* test retrieval of set flags */

    ASSERT_TRUE(commc_args_get_flag(parser, "verbose") == 1);
    ASSERT_TRUE(commc_args_get_flag(parser, "v") == 1);
    ASSERT_TRUE(commc_args_get_flag(parser, "debug") == 1);
    ASSERT_TRUE(commc_args_get_flag(parser, "d") == 1);

    /* test retrieval of unset flag */

    ASSERT_TRUE(commc_args_get_flag(parser, "quiet") == 0);
    ASSERT_TRUE(commc_args_get_flag(parser, "q") == 0);

    commc_args_parser_destroy(parser);

    /* test absent flags in separate parser */

    parser = commc_args_parser_create();
    ASSERT_NOT_NULL(parser);

    result = commc_args_add_flag(parser, "v", "verbose", "Enable verbose output");
    ASSERT_TRUE(result == 1);

    result = commc_args_parse(parser, 1, argv2);
    ASSERT_TRUE(result == 1);

    ASSERT_TRUE(commc_args_get_flag(parser, "verbose") == 0);
    ASSERT_TRUE(commc_args_get_flag(parser, "v") == 0);

    commc_args_parser_destroy(parser);
    ASSERT_NO_MEMORY_LEAKS();

END_TEST

/*

         test_args_option_retrieval()
	       ---
	       tests option value retrieval with explicit values and defaults.

*/

TEST_CASE(test_args_option_retrieval)

    commc_args_parser_t* parser;
    char* argv[] = {"./program", "--output=custom.log", "-c", "my_config.ini"};
    char* argv2[] = {"./program"};
    int argc = 4;
    int result;

    MEMORY_LEAK_CHECK_START();

    parser = commc_args_parser_create();
    ASSERT_NOT_NULL(parser);

    /* add options */

    result = commc_args_add_option(parser, "o", "output", "Output file", "default.txt");
    ASSERT_TRUE(result == 1);

    result = commc_args_add_option(parser, "c", "config", "Config file", "default.cfg");
    ASSERT_TRUE(result == 1);

    result = commc_args_add_option(parser, "l", "log", "Log level", NULL);
    ASSERT_TRUE(result == 1);

    /* parse the command line */

    result = commc_args_parse(parser, argc, argv);
    ASSERT_TRUE(result == 1);

    /* test retrieval of set options */

    ASSERT_STRING_EQUALS(commc_args_get_option(parser, "output"), "custom.log");
    ASSERT_STRING_EQUALS(commc_args_get_option(parser, "o"), "custom.log");
    ASSERT_STRING_EQUALS(commc_args_get_option(parser, "config"), "my_config.ini");
    ASSERT_STRING_EQUALS(commc_args_get_option(parser, "c"), "my_config.ini");

    /* test option with default (not provided on command line) */

    ASSERT_STRING_EQUALS(commc_args_get_option(parser, "log"), "");

    commc_args_parser_destroy(parser);

    /* test default values when options not provided */

    parser = commc_args_parser_create();
    ASSERT_NOT_NULL(parser);

    result = commc_args_add_option(parser, "o", "output", "Output file", "default.txt");
    ASSERT_TRUE(result == 1);

    result = commc_args_parse(parser, 1, argv2);
    ASSERT_TRUE(result == 1);

    ASSERT_STRING_EQUALS(commc_args_get_option(parser, "output"), "default.txt");

    commc_args_parser_destroy(parser);
    ASSERT_NO_MEMORY_LEAKS();

END_TEST

/*

         test_args_positional_retrieval()
	       ---
	       tests positional argument retrieval.

*/

TEST_CASE(test_args_positional_retrieval)

    commc_args_parser_t* parser;
    char* argv[] = {"./program", "file1.txt", "file2.txt", "output.txt"};
    int argc = 4;
    int result;

    MEMORY_LEAK_CHECK_START();

    parser = commc_args_parser_create();
    ASSERT_NOT_NULL(parser);

    /* add positional arguments */

    result = commc_args_add_positional(parser, "input1", "First input file");
    ASSERT_TRUE(result == 1);

    result = commc_args_add_positional(parser, "input2", "Second input file");
    ASSERT_TRUE(result == 1);

    result = commc_args_add_positional(parser, "output", "Output file");
    ASSERT_TRUE(result == 1);

    /* parse the command line */

    result = commc_args_parse(parser, argc, argv);
    ASSERT_TRUE(result == 1);

    /* test retrieval */

    ASSERT_STRING_EQUALS(commc_args_get_positional(parser, 0), "file1.txt");
    ASSERT_STRING_EQUALS(commc_args_get_positional(parser, 1), "file2.txt");
    ASSERT_STRING_EQUALS(commc_args_get_positional(parser, 2), "output.txt");

    /* test out of bounds access */

    ASSERT_NULL(commc_args_get_positional(parser, 3));

    commc_args_parser_destroy(parser);
    ASSERT_NO_MEMORY_LEAKS();

END_TEST

/*

         test_args_mixed_scenarios()
	       ---
	       tests complex scenarios mixing flags, options, and positional args.

*/

TEST_CASE(test_args_mixed_scenarios)

    commc_args_parser_t* parser;
    char* argv[] = {"./program", "-v", "--config=app.ini", "-o", "out.log", "input.dat"};
    int argc = 6;
    int result;

    MEMORY_LEAK_CHECK_START();

    parser = commc_args_parser_create();
    ASSERT_NOT_NULL(parser);

    /* setup comprehensive argument set */

    result = commc_args_add_flag(parser, "v", "verbose", "Verbose output");
    ASSERT_TRUE(result == 1);

    result = commc_args_add_option(parser, "c", "config", "Config file", "default.ini");
    ASSERT_TRUE(result == 1);

    result = commc_args_add_option(parser, "o", "output", "Output file", "stdout");
    ASSERT_TRUE(result == 1);

    result = commc_args_add_positional(parser, "input", "Input file");
    ASSERT_TRUE(result == 1);

    /* parse and verify */

    result = commc_args_parse(parser, argc, argv);
    ASSERT_TRUE(result == 1);

    ASSERT_TRUE(commc_args_get_flag(parser, "verbose") == 1);
    ASSERT_STRING_EQUALS(commc_args_get_option(parser, "config"), "app.ini");
    ASSERT_STRING_EQUALS(commc_args_get_option(parser, "output"), "out.log");
    ASSERT_STRING_EQUALS(commc_args_get_positional(parser, 0), "input.dat");

    commc_args_parser_destroy(parser);
    ASSERT_NO_MEMORY_LEAKS();

END_TEST

/*

         test_args_edge_cases()
	       ---
	       tests edge cases and boundary conditions.

*/

TEST_CASE(test_args_edge_cases)

    commc_args_parser_t* parser;
    char* argv1[] = {"./program"};  /* no arguments */
    int result;

    MEMORY_LEAK_CHECK_START();

    parser = commc_args_parser_create();
    ASSERT_NOT_NULL(parser);

    /* add arguments but don't provide any on command line */

    result = commc_args_add_flag(parser, "v", "verbose", "Verbose flag");
    ASSERT_TRUE(result == 1);

    result = commc_args_add_option(parser, "o", "output", "Output", "default.txt");
    ASSERT_TRUE(result == 1);

    /* test parsing with no arguments */

    result = commc_args_parse(parser, 1, argv1);
    ASSERT_TRUE(result == 1);

    /* verify defaults */

    ASSERT_TRUE(commc_args_get_flag(parser, "verbose") == 0);
    ASSERT_STRING_EQUALS(commc_args_get_option(parser, "output"), "default.txt");

    commc_args_parser_destroy(parser);
    ASSERT_NO_MEMORY_LEAKS();

END_TEST

/* --- TEST RUNNER --- */

int main(void) {

    PRINT_TEST_BANNER();

    printf("OUTPUT: TESTING ARGS MODULE COMPREHENSIVELY...\n\n");

    /* add all test cases to be run here */

    ADD_TEST(test_args_parser_lifecycle);
    ADD_TEST(test_args_null_safety);
    ADD_TEST(test_args_add_flag_success);
    ADD_TEST(test_args_add_option_success);
    ADD_TEST(test_args_add_positional_success);
    ADD_TEST(test_args_parsing_success);
    ADD_TEST(test_args_flag_retrieval);
    ADD_TEST(test_args_option_retrieval);
    ADD_TEST(test_args_positional_retrieval);
    ADD_TEST(test_args_mixed_scenarios);
    ADD_TEST(test_args_edge_cases);

    RUN_ALL_TESTS();
    PRINT_TEST_SUMMARY();

    return commc_tests_failed > 0 ? 1 : 0;
}