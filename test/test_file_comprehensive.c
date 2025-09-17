/*
    ===================================================

        COMMON-C TEST MODULE: FILE COMPREHENSIVE TESTS
           ---
           comprehensive test coverage for file.c
           following established methodology template

        Note: This test creates temporary test files and
        directories which are cleaned up during testing.

    ===================================================
    
    Public Functions Tested:
    
    - commc_file_open()
    - commc_file_close()
    - commc_file_read_all_text()
    - commc_file_write_all_text()
    - commc_file_read_all_bytes()
    - commc_file_write_all_bytes()
    - commc_file_exists()
    - commc_file_delete()
    - commc_dir_create()
    - commc_dir_delete()
    
*/

#include    <stdio.h>
#include    <string.h>
#include    <stdlib.h>

#include    "test_harness.h"
#include    "../include/commc/file.h"
#include    "../include/commc/error.h"

/* test file/directory paths */

#define TEST_TEXT_FILE "test_temp.txt"
#define TEST_BINARY_FILE "test_temp.bin"
#define TEST_NONEXISTENT_FILE "test_nonexistent_file_xyz.txt"
#define TEST_DIR "test_temp_dir"
#define TEST_DIR_FILE "test_temp_dir/inner_file.txt"

/*
    ==================================
             TEST FUNCTIONS
    ==================================
*/

/*

    test_file_mode_enum()
    ---
    tests that all file mode enum values are handled

*/

void test_file_mode_enum(void) {

    FILE* file;
    const char* test_content = "test content";
    
    printf("TESTING: FILE MODE ENUM HANDLING...\n");
    
    /* create a test file first */

    commc_file_write_all_text(TEST_TEXT_FILE, test_content);
    
    /* test all file modes */

    file = commc_file_open(TEST_TEXT_FILE, COMMC_FILE_READ);
    ASSERT_NOT_NULL(file);
    commc_file_close(file);
    
    file = commc_file_open(TEST_TEXT_FILE, COMMC_FILE_WRITE);
    ASSERT_NOT_NULL(file);
    commc_file_close(file);
    
    file = commc_file_open(TEST_TEXT_FILE, COMMC_FILE_APPEND);
    ASSERT_NOT_NULL(file);
    commc_file_close(file);
    
    file = commc_file_open(TEST_TEXT_FILE, COMMC_FILE_READ_WRITE);
    ASSERT_NOT_NULL(file);
    commc_file_close(file);
    
    file = commc_file_open(TEST_TEXT_FILE, COMMC_FILE_READ_APPEND);
    ASSERT_NOT_NULL(file);
    commc_file_close(file);
    
    /* cleanup */

    commc_file_delete(TEST_TEXT_FILE);
    
    printf("PASSED: FILE MODE ENUM HANDLING\n\n");

}

/*

    test_file_open_close_lifecycle()
    ---
    tests basic file opening and closing operations

*/

void test_file_open_close_lifecycle(void) {

    FILE* file;
    const char* test_content = "lifecycle test content";
    
    printf("TESTING: FILE OPEN/CLOSE LIFECYCLE...\n");
    
    /* create test file first */

    commc_file_write_all_text(TEST_TEXT_FILE, test_content);
    ASSERT_TRUE(commc_file_exists(TEST_TEXT_FILE));
    
    /* test opening existing file */

    file = commc_file_open(TEST_TEXT_FILE, COMMC_FILE_READ);
    ASSERT_NOT_NULL(file);
    
    /* test closing file */

    commc_file_close(file);
    
    /* test creating new file */

    commc_file_delete(TEST_TEXT_FILE);  /* ensure clean state */
    file = commc_file_open(TEST_TEXT_FILE, COMMC_FILE_WRITE);
    ASSERT_NOT_NULL(file);
    commc_file_close(file);
    
    /* verify file was created */

    ASSERT_TRUE(commc_file_exists(TEST_TEXT_FILE));
    
    /* cleanup */

    commc_file_delete(TEST_TEXT_FILE);
    
    printf("PASSED: FILE OPEN/CLOSE LIFECYCLE\n\n");

}

/*

    test_file_open_null_safety()
    ---
    tests file operations with null parameters

*/

void test_file_open_null_safety(void) {

    FILE* file;
    
    printf("TESTING: FILE OPEN NULL SAFETY...\n");
    
    /* test null path */

    file = commc_file_open(NULL, COMMC_FILE_READ);
    ASSERT_NULL(file);
    
    /* test closing null file (should handle gracefully) */

    commc_file_close(NULL);
    
    printf("PASSED: FILE OPEN NULL SAFETY\n\n");

}

/*

    test_file_open_nonexistent()
    ---
    tests opening files that don't exist

*/

void test_file_open_nonexistent(void) {

    FILE* file;
    
    printf("TESTING: FILE OPEN NONEXISTENT FILES...\n");
    
    /* ensure test file doesn't exist */

    commc_file_delete(TEST_NONEXISTENT_FILE);
    
    /* try to open nonexistent file for reading */

    file = commc_file_open(TEST_NONEXISTENT_FILE, COMMC_FILE_READ);
    ASSERT_NULL(file);
    
    /* try to open nonexistent file for read/write */

    file = commc_file_open(TEST_NONEXISTENT_FILE, COMMC_FILE_READ_WRITE);
    ASSERT_NULL(file);
    
    /* write mode should create file */

    file = commc_file_open(TEST_NONEXISTENT_FILE, COMMC_FILE_WRITE);
    ASSERT_NOT_NULL(file);
    commc_file_close(file);
    
    /* verify file was created */

    ASSERT_TRUE(commc_file_exists(TEST_NONEXISTENT_FILE));
    
    /* cleanup */

    commc_file_delete(TEST_NONEXISTENT_FILE);
    
    printf("PASSED: FILE OPEN NONEXISTENT FILES\n\n");

}

/*

    test_text_file_operations()
    ---
    tests reading and writing text files

*/

void test_text_file_operations(void) {

    char* content;
    const char* test_content = "Hello, World!\nThis is a test file.\nLine 3 with special chars: !@#$%^&*()";
    const char* empty_content = "";
    int result;
    
    printf("TESTING: TEXT FILE OPERATIONS...\n");
    
    /* test writing text */

    result = commc_file_write_all_text(TEST_TEXT_FILE, test_content);
    ASSERT_TRUE(result == 1);
    ASSERT_TRUE(commc_file_exists(TEST_TEXT_FILE));
    
    /* test reading text */

    content = commc_file_read_all_text(TEST_TEXT_FILE);
    ASSERT_NOT_NULL(content);
    ASSERT_TRUE(strcmp(content, test_content) == 0);
    free(content);
    
    /* test writing empty text */

    result = commc_file_write_all_text(TEST_TEXT_FILE, empty_content);
    ASSERT_TRUE(result == 1);
    
    content = commc_file_read_all_text(TEST_TEXT_FILE);
    ASSERT_NOT_NULL(content);
    ASSERT_TRUE(strcmp(content, empty_content) == 0);
    ASSERT_TRUE(strlen(content) == 0);
    free(content);
    
    /* test overwriting existing file */

    result = commc_file_write_all_text(TEST_TEXT_FILE, "overwritten content");
    ASSERT_TRUE(result == 1);
    
    content = commc_file_read_all_text(TEST_TEXT_FILE);
    ASSERT_NOT_NULL(content);
    ASSERT_TRUE(strcmp(content, "overwritten content") == 0);
    free(content);
    
    /* cleanup */

    commc_file_delete(TEST_TEXT_FILE);
    
    printf("PASSED: TEXT FILE OPERATIONS\n\n");

}

/*

    test_text_file_null_safety()
    ---
    tests text file operations with null parameters

*/

void test_text_file_null_safety(void) {

    char* content;
    int result;
    
    printf("TESTING: TEXT FILE NULL SAFETY...\n");
    
    /* test reading with null path */

    content = commc_file_read_all_text(NULL);
    ASSERT_NULL(content);
    
    /* test writing with null path */

    result = commc_file_write_all_text(NULL, "test content");
    ASSERT_TRUE(result == 0);
    
    /* test writing with null content */

    result = commc_file_write_all_text(TEST_TEXT_FILE, NULL);
    ASSERT_TRUE(result == 0);
    
    /* test reading nonexistent file */

    commc_file_delete(TEST_NONEXISTENT_FILE);
    content = commc_file_read_all_text(TEST_NONEXISTENT_FILE);
    ASSERT_NULL(content);
    
    printf("PASSED: TEXT FILE NULL SAFETY\n\n");

}

/*

    test_binary_file_operations()
    ---
    tests reading and writing binary files

*/

void test_binary_file_operations(void) {

    void* buffer;
    size_t size;
    unsigned char test_data[] = {0x00, 0x01, 0x02, 0xFF, 0xFE, 0xFD, 0x7F, 0x80, 0x81};
    size_t test_size = sizeof(test_data);
    int result;
    
    printf("TESTING: BINARY FILE OPERATIONS...\n");
    
    /* test writing binary data */

    result = commc_file_write_all_bytes(TEST_BINARY_FILE, test_data, test_size);
    ASSERT_TRUE(result == 1);
    ASSERT_TRUE(commc_file_exists(TEST_BINARY_FILE));
    
    /* test reading binary data */
    
    buffer = commc_file_read_all_bytes(TEST_BINARY_FILE, &size);
    ASSERT_NOT_NULL(buffer);
    ASSERT_TRUE(size == test_size);
    ASSERT_TRUE(memcmp(buffer, test_data, test_size) == 0);
    free(buffer);
    
    /* test writing empty binary data */

    result = commc_file_write_all_bytes(TEST_BINARY_FILE, "", 0);
    ASSERT_TRUE(result == 1);
    
    buffer = commc_file_read_all_bytes(TEST_BINARY_FILE, &size);
    ASSERT_NOT_NULL(buffer);
    ASSERT_TRUE(size == 0);
    free(buffer);
    
    /* test writing single byte */

    result = commc_file_write_all_bytes(TEST_BINARY_FILE, "\xAB", 1);
    ASSERT_TRUE(result == 1);
    
    buffer = commc_file_read_all_bytes(TEST_BINARY_FILE, &size);
    ASSERT_NOT_NULL(buffer);
    ASSERT_TRUE(size == 1);
    ASSERT_TRUE(((unsigned char*)buffer)[0] == 0xAB);
    free(buffer);
    
    /* cleanup */

    commc_file_delete(TEST_BINARY_FILE);
    
    printf("PASSED: BINARY FILE OPERATIONS\n\n");

}

/*

    test_binary_file_null_safety()
    ---
    tests binary file operations with null parameters

*/

void test_binary_file_null_safety(void) {

    void* buffer;
    size_t size;
    unsigned char test_data[] = {0x01, 0x02, 0x03};
    int result;
    
    printf("TESTING: BINARY FILE NULL SAFETY...\n");
    
    /* test reading with null path */

    buffer = commc_file_read_all_bytes(NULL, &size);
    ASSERT_NULL(buffer);
    
    /* test reading with null size pointer */

    buffer = commc_file_read_all_bytes(TEST_BINARY_FILE, NULL);
    ASSERT_NULL(buffer);
    
    /* test writing with null path */

    result = commc_file_write_all_bytes(NULL, test_data, sizeof(test_data));
    ASSERT_TRUE(result == 0);
    
    /* test writing with null buffer */

    result = commc_file_write_all_bytes(TEST_BINARY_FILE, NULL, sizeof(test_data));
    ASSERT_TRUE(result == 0);
    
    /* test reading nonexistent file */

    commc_file_delete(TEST_NONEXISTENT_FILE);
    buffer = commc_file_read_all_bytes(TEST_NONEXISTENT_FILE, &size);
    ASSERT_NULL(buffer);
    
    printf("PASSED: BINARY FILE NULL SAFETY\n\n");

}

/*

    test_file_exists_operations()
    ---
    tests file existence checking

*/

void test_file_exists_operations(void) {

    int result;
    
    printf("TESTING: FILE EXISTS OPERATIONS...\n");
    
    /* test nonexistent file */

    commc_file_delete(TEST_TEXT_FILE);
    result = commc_file_exists(TEST_TEXT_FILE);
    ASSERT_TRUE(result == 0);
    
    /* create file and test exists */

    commc_file_write_all_text(TEST_TEXT_FILE, "test");
    result = commc_file_exists(TEST_TEXT_FILE);
    ASSERT_TRUE(result == 1);
    
    /* delete file and test doesn't exist */

    commc_file_delete(TEST_TEXT_FILE);
    result = commc_file_exists(TEST_TEXT_FILE);
    ASSERT_TRUE(result == 0);
    
    /* test with null path */

    result = commc_file_exists(NULL);
    ASSERT_TRUE(result == 0);
    
    printf("PASSED: FILE EXISTS OPERATIONS\n\n");

}

/*

    test_file_delete_operations()
    ---
    tests file deletion operations

*/

void test_file_delete_operations(void) {

    int result;
    
    printf("TESTING: FILE DELETE OPERATIONS...\n");
    
    /* create test file */

    commc_file_write_all_text(TEST_TEXT_FILE, "delete me");
    ASSERT_TRUE(commc_file_exists(TEST_TEXT_FILE));
    
    /* test deleting existing file */

    result = commc_file_delete(TEST_TEXT_FILE);
    ASSERT_TRUE(result == 1);
    ASSERT_TRUE(commc_file_exists(TEST_TEXT_FILE) == 0);
    
    /* test deleting nonexistent file */

    result = commc_file_delete(TEST_TEXT_FILE);
    ASSERT_TRUE(result == 0);
    
    /* test deleting with null path */

    result = commc_file_delete(NULL);
    ASSERT_TRUE(result == 0);
    
    printf("PASSED: FILE DELETE OPERATIONS\n\n");

}

/*

    test_directory_operations()
    ---
    tests directory creation and deletion

*/

void test_directory_operations(void) {

    int result;
    
    printf("TESTING: DIRECTORY OPERATIONS...\n");
    
    /* ensure test directory doesn't exist */

    commc_dir_delete(TEST_DIR);
    
    /* test creating directory */

    result = commc_dir_create(TEST_DIR);
    ASSERT_TRUE(result == 1);
    
    /* test creating directory that already exists */

    result = commc_dir_create(TEST_DIR);
    ASSERT_TRUE(result == 0);  /* should fail gracefully */
    
    /* test deleting empty directory */

    result = commc_dir_delete(TEST_DIR);
    ASSERT_TRUE(result == 1);
    
    /* test deleting nonexistent directory */

    result = commc_dir_delete(TEST_DIR);
    ASSERT_TRUE(result == 0);
    
    printf("PASSED: DIRECTORY OPERATIONS\n\n");

}

/*

    test_directory_null_safety()
    ---
    tests directory operations with null parameters

*/

void test_directory_null_safety(void) {

    int result;
    
    printf("TESTING: DIRECTORY NULL SAFETY...\n");
    
    /* test creating directory with null path */

    result = commc_dir_create(NULL);
    ASSERT_TRUE(result == 0);
    
    /* test deleting directory with null path */

    result = commc_dir_delete(NULL);
    ASSERT_TRUE(result == 0);
    
    printf("PASSED: DIRECTORY NULL SAFETY\n\n");

}

/*

    test_large_file_operations()
    ---
    tests operations with larger files

*/

void test_large_file_operations(void) {

    char* large_text;
    void* large_buffer;
    char* read_text;
    void* read_buffer;
    size_t read_size;
    size_t large_size = 10000;  /* 10KB test */
    size_t i;
    int result;
    
    printf("TESTING: LARGE FILE OPERATIONS...\n");
    
    /* create large text content */

    large_text = (char*)malloc(large_size + 1);
    ASSERT_NOT_NULL(large_text);

    for (i = 0; i < large_size; i++) {

        large_text[i] = (char)('A' + (i % 26));  /* cycle through A-Z */

    }

    large_text[large_size] = '\0';
    
    /* test large text file */

    result = commc_file_write_all_text(TEST_TEXT_FILE, large_text);
    ASSERT_TRUE(result == 1);
    
    read_text = commc_file_read_all_text(TEST_TEXT_FILE);
    ASSERT_NOT_NULL(read_text);
    ASSERT_TRUE(strcmp(read_text, large_text) == 0);
    ASSERT_TRUE(strlen(read_text) == large_size);
    free(read_text);
    
    /* create large binary content */

    large_buffer = malloc(large_size);
    ASSERT_NOT_NULL(large_buffer);

    for (i = 0; i < large_size; i++) {

        ((unsigned char*)large_buffer)[i] = (unsigned char)(i % 256);

    }
    
    /* test large binary file */

    result = commc_file_write_all_bytes(TEST_BINARY_FILE, large_buffer, large_size);
    ASSERT_TRUE(result == 1);
    
    read_buffer = commc_file_read_all_bytes(TEST_BINARY_FILE, &read_size);
    ASSERT_NOT_NULL(read_buffer);
    ASSERT_TRUE(read_size == large_size);
    ASSERT_TRUE(memcmp(read_buffer, large_buffer, large_size) == 0);
    free(read_buffer);
    
    /* cleanup */

    free(large_text);
    free(large_buffer);
    commc_file_delete(TEST_TEXT_FILE);
    commc_file_delete(TEST_BINARY_FILE);
    
    printf("PASSED: LARGE FILE OPERATIONS\n\n");

}

/*

    test_complex_file_scenario()
    ---
    tests a complex scenario combining multiple file operations

*/

void test_complex_file_scenario(void) {

    FILE* file;
    char* content;
    void* buffer;
    size_t size;
    int result;
    const char* config_content = "[settings]\nvolume=75\nfullscreen=true\n";
    const unsigned char save_data[] = {0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x02, 0x03, 0x04};
    
    printf("TESTING: COMPLEX FILE SCENARIO...\n");
    
    /* create a temporary directory for project files */

    result = commc_dir_create(TEST_DIR);
    ASSERT_TRUE(result == 1);
    
    /* write configuration file */

    result = commc_file_write_all_text("test_temp_dir/config.ini", config_content);
    ASSERT_TRUE(result == 1);
    ASSERT_TRUE(commc_file_exists("test_temp_dir/config.ini"));
    
    /* write binary save file */

    result = commc_file_write_all_bytes("test_temp_dir/save.dat", save_data, sizeof(save_data));
    ASSERT_TRUE(result == 1);
    ASSERT_TRUE(commc_file_exists("test_temp_dir/save.dat"));
    
    /* create log file using low-level operations */

    file = commc_file_open("test_temp_dir/debug.log", COMMC_FILE_WRITE);
    ASSERT_NOT_NULL(file);
    fprintf(file, "APPLICATION STARTED\n");
    fprintf(file, "LOADING CONFIGURATION...\n");
    fprintf(file, "CONFIGURATION LOADED SUCCESSFULLY\n");
    commc_file_close(file);
    
    /* verify all files exist */

    ASSERT_TRUE(commc_file_exists("test_temp_dir/config.ini"));
    ASSERT_TRUE(commc_file_exists("test_temp_dir/save.dat"));
    ASSERT_TRUE(commc_file_exists("test_temp_dir/debug.log"));
    
    /* read and verify configuration */

    content = commc_file_read_all_text("test_temp_dir/config.ini");
    ASSERT_NOT_NULL(content);
    ASSERT_TRUE(strcmp(content, config_content) == 0);
    free(content);
    
    /* read and verify save data */

    buffer = commc_file_read_all_bytes("test_temp_dir/save.dat", &size);
    ASSERT_NOT_NULL(buffer);
    ASSERT_TRUE(size == sizeof(save_data));
    ASSERT_TRUE(memcmp(buffer, save_data, sizeof(save_data)) == 0);
    free(buffer);
    
    /* read and verify log file */

    content = commc_file_read_all_text("test_temp_dir/debug.log");
    ASSERT_NOT_NULL(content);
    ASSERT_TRUE(strlen(content) > 0);
    ASSERT_TRUE(strstr(content, "APPLICATION STARTED") != NULL);
    ASSERT_TRUE(strstr(content, "CONFIGURATION LOADED") != NULL);
    free(content);
    
    /* cleanup - delete all files first */

    ASSERT_TRUE(commc_file_delete("test_temp_dir/config.ini") == 1);
    ASSERT_TRUE(commc_file_delete("test_temp_dir/save.dat") == 1);
    ASSERT_TRUE(commc_file_delete("test_temp_dir/debug.log") == 1);
    
    /* then delete directory */

    ASSERT_TRUE(commc_dir_delete(TEST_DIR) == 1);
    
    printf("PASSED: COMPLEX FILE SCENARIO\n\n");

}

/*
    ==================================
             MAIN TEST RUNNER
    ==================================
*/

int main(void) {

    printf("\n");
    printf("========================================\n");
    printf("  COMMON-C FILE COMPREHENSIVE TESTS\n");
    printf("========================================\n\n");

    MEMORY_LEAK_CHECK_START();

    /* basic file operations */

    ADD_TEST(file_mode_enum);
    ADD_TEST(file_open_close_lifecycle);
    ADD_TEST(file_open_null_safety);
    ADD_TEST(file_open_nonexistent);

    /* text file operations */

    ADD_TEST(text_file_operations);
    ADD_TEST(text_file_null_safety);

    /* binary file operations */

    ADD_TEST(binary_file_operations);
    ADD_TEST(binary_file_null_safety);

    /* file system operations */

    ADD_TEST(file_exists_operations);
    ADD_TEST(file_delete_operations);

    /* directory operations */

    ADD_TEST(directory_operations);
    ADD_TEST(directory_null_safety);

    /* performance and edge cases */

    ADD_TEST(large_file_operations);

    /* integration tests */

    ADD_TEST(complex_file_scenario);

    RUN_ALL_TESTS();

    MEMORY_LEAK_CHECK_END();

    printf("========================================\n");
    printf("  ALL FILE TESTS COMPLETED\n");
    printf("========================================\n\n");

    return 0;

}

/*
    ==================================
             --- EOF ---
    ==================================
*/
