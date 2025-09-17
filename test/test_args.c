/* ============================================================================
 *  COMMON-C ARGS MODULE TEST
 *  ELASTIC SOFTWORKS 2025
 *
 *  this file contains the test suite for the args module.
 * ========================================================================= */

#include "test_harness.h"
#include "commc/args.h"

/* 
	==================================
             --- TEST CASES ---
	==================================
*/

/* test case for parser creation and destruction */

TEST_CASE(args_create_destroy)

    /* create a parser instance and ensure it's not null */

    commc_args_parser_t* parser;
    parser = commc_args_parser_create();
    ASSERT_NOT_NULL(parser);

    /* destroy the parser and ensure no memory issues */

    commc_args_parser_destroy(parser);
    ASSERT_TRUE(1);                     /* this assertion is just to ensure the test case itself 
                                           passes if no other assertions are made */

END_TEST

/* test case for adding a flag */

TEST_CASE(args_add_flag)

    commc_args_parser_t* parser;
    parser = commc_args_parser_create();
    ASSERT_NOT_NULL(parser);

    /* add a flag: -v, --verbose */

    ASSERT_TRUE(commc_args_add_flag(parser, "-v", "--verbose", "Enable verbose output") == 0);

    /* add another flag: -h, --help */

    ASSERT_TRUE(commc_args_add_flag(parser, "-h", "--help", "Show help message") == 0);

    commc_args_parser_destroy(parser);

END_TEST

/* test case for adding an option */

TEST_CASE(args_add_option)

    commc_args_parser_t* parser;
    parser = commc_args_parser_create();
    ASSERT_NOT_NULL(parser);

    /* add an option: -o, --output with a default value */

    ASSERT_TRUE(commc_args_add_option(parser, "-o", "--output", "Output file", "default.txt") == 0);

    commc_args_parser_destroy(parser);

END_TEST

/* test case for adding a positional argument */

TEST_CASE(args_add_positional)

    commc_args_parser_t* parser;
    parser = commc_args_parser_create();
    ASSERT_NOT_NULL(parser);

    /* add a positional argument: input_file */

    ASSERT_TRUE(commc_args_add_positional(parser, "input_file", "Input file path") == 0);

    commc_args_parser_destroy(parser);

END_TEST

/* test case for parsing a simple command line */

TEST_CASE(args_parse_simple)

    commc_args_parser_t* parser;
    char* argv[] = {"./myapp", "-v", "--output=my_output.txt", "input.dat", "config.cfg"};
    int argc = sizeof(argv) / sizeof(argv[0]);
    
    parser = commc_args_parser_create();
    ASSERT_NOT_NULL(parser);

    /* add a flag and an option */

    ASSERT_TRUE(commc_args_add_flag(parser, "-v", "--verbose", "Enable verbose output") == 0);
    ASSERT_TRUE(commc_args_add_option(parser, "-o", "--output", "Output file", "default.txt") == 0);
    ASSERT_TRUE(commc_args_add_positional(parser, "input_file", "Input file path") == 0);
    ASSERT_TRUE(commc_args_add_positional(parser, "config_file", "Configuration file path") == 0);

    /* parse the arguments */

    ASSERT_TRUE(commc_args_parse(parser, argc, argv) == 1);

    /* verify parsed values */

    ASSERT_TRUE(commc_args_get_flag(parser, "verbose") == 1);
    ASSERT_STRING_EQUALS(commc_args_get_option(parser, "output"), "my_output.txt");
    ASSERT_STRING_EQUALS(commc_args_get_positional(parser, 0), "input.dat");
    ASSERT_STRING_EQUALS(commc_args_get_positional(parser, 1), "config.cfg");
    ASSERT_NULL(commc_args_get_positional(parser, 2)); /* ensure no extra positional args are found */

    commc_args_parser_destroy(parser);

END_TEST

/* test case for retrieving a present flag */

TEST_CASE(args_get_flag_present)

    commc_args_parser_t* parser;
    char* argv[] = {"./myapp", "-f"};
    int argc = sizeof(argv) / sizeof(argv[0]);
    
    parser = commc_args_parser_create();
    ASSERT_NOT_NULL(parser);
    ASSERT_TRUE(commc_args_add_flag(parser, "-f", "--flag", "A test flag") == 0);
    ASSERT_TRUE(commc_args_parse(parser, argc, argv) == 1);
    ASSERT_TRUE(commc_args_get_flag(parser, "flag") == 1);
    ASSERT_TRUE(commc_args_get_flag(parser, "f") == 1); /* Test short name */
    commc_args_parser_destroy(parser);

END_TEST

/* test case for retrieving an absent flag */

TEST_CASE(args_get_flag_absent)

    commc_args_parser_t* parser;
    char* argv[] = {"./myapp"}; /* No flag provided */
    int argc = sizeof(argv) / sizeof(argv[0]);
    
    parser = commc_args_parser_create();
    ASSERT_NOT_NULL(parser);
    ASSERT_TRUE(commc_args_add_flag(parser, "-f", "--flag", "A test flag") == 0);
    ASSERT_TRUE(commc_args_parse(parser, argc, argv) == 1);
    ASSERT_TRUE(commc_args_get_flag(parser, "flag") == 0);
    ASSERT_TRUE(commc_args_get_flag(parser, "f") == 0);
    commc_args_parser_destroy(parser);

END_TEST

/* test case for retrieving an option's value */

TEST_CASE(args_get_option_value)

    commc_args_parser_t* parser;
    char* argv[] = {"./myapp", "--output=custom.log"};
    int argc = sizeof(argv) / sizeof(argv[0]);
    
    parser = commc_args_parser_create();
    ASSERT_NOT_NULL(parser);
    ASSERT_TRUE(commc_args_add_option(parser, "-o", "--output", "Output file", "default.txt") == 0);
    ASSERT_TRUE(commc_args_parse(parser, argc, argv) == 1);
    ASSERT_STRING_EQUALS(commc_args_get_option(parser, "output"), "custom.log");
    ASSERT_STRING_EQUALS(commc_args_get_option(parser, "o"), "custom.log"); /* Test short name */
    commc_args_parser_destroy(parser);

END_TEST

/* test case for retrieving an option's default value */

TEST_CASE(args_get_option_default)

    commc_args_parser_t* parser;
    char* argv[] = {"./myapp"};
    int argc = sizeof(argv) / sizeof(argv[0]);
    
    parser = commc_args_parser_create();
    ASSERT_NOT_NULL(parser);
    ASSERT_TRUE(commc_args_add_option(parser, "-o", "--output", "Output file", "default.txt") == 0);
    ASSERT_TRUE(commc_args_parse(parser, argc, argv) == 1);
    ASSERT_STRING_EQUALS(commc_args_get_option(parser, "output"), "default.txt");
    commc_args_parser_destroy(parser);

END_TEST

/* test case for retrieving a valid positional argument */

TEST_CASE(args_get_positional_valid)

    commc_args_parser_t* parser;
    char* argv[] = {"./myapp", "input1.txt", "input2.txt"};
    int argc = sizeof(argv) / sizeof(argv[0]);
    
    parser = commc_args_parser_create();
    ASSERT_NOT_NULL(parser);
    ASSERT_TRUE(commc_args_add_positional(parser, "file1", "First input file") == 0);
    ASSERT_TRUE(commc_args_add_positional(parser, "file2", "Second input file") == 0);
    ASSERT_TRUE(commc_args_parse(parser, argc, argv) == 1);
    ASSERT_STRING_EQUALS(commc_args_get_positional(parser, 0), "input1.txt");
    ASSERT_STRING_EQUALS(commc_args_get_positional(parser, 1), "input2.txt");
    commc_args_parser_destroy(parser);

END_TEST

/* test case for retrieving an invalid positional argument index */

TEST_CASE(args_get_positional_invalid)

    commc_args_parser_t* parser;
    char* argv[] = {"./myapp", "input1.txt"};
    int argc = sizeof(argv) / sizeof(argv[0]);
    
    parser = commc_args_parser_create();
    ASSERT_NOT_NULL(parser);
    ASSERT_TRUE(commc_args_add_positional(parser, "file1", "First input file") == 0);
    ASSERT_TRUE(commc_args_parse(parser, argc, argv) == 1);
    ASSERT_NULL(commc_args_get_positional(parser, 1)); /* Index out of bounds */
    commc_args_parser_destroy(parser);

END_TEST

/* --- TEST RUNNER --- */

int main(void) {

    PRINT_TEST_BANNER();

    /* add all test cases to be run here */

    ADD_TEST(args_create_destroy);
    ADD_TEST(args_add_flag);
    ADD_TEST(args_add_option);
    ADD_TEST(args_add_positional);
    ADD_TEST(args_parse_simple);
    ADD_TEST(args_get_flag_present);
    ADD_TEST(args_get_flag_absent);
    ADD_TEST(args_get_option_value);
    ADD_TEST(args_get_option_default);
    ADD_TEST(args_get_positional_valid);
    ADD_TEST(args_get_positional_invalid);

    RUN_ALL_TESTS();
    PRINT_TEST_SUMMARY();

    /* return 1 if any tests failed, 0 otherwise */
    
    return commc_tests_failed > 0 ? 1 : 0;
}
