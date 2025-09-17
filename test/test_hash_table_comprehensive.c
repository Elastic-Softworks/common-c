/*

    ===================================================

        COMMON-C TEST MODULE: HASH TABLE COMPREHENSIVE TESTS
           ---
           comprehensive test coverage for hash_table.c
           following established methodology template

        NOTE: This module tests hash table data structure
        with separate chaining collision resolution using
        linked lists and DJB2 hashing algorithm.

    ===================================================
    
    Public Functions Tested:
    
    - commc_hash_table_create()
    - commc_hash_table_destroy()
    - commc_hash_table_insert()
    - commc_hash_table_get()
    - commc_hash_table_remove()
    - commc_hash_table_size()
    
*/

#include    <stdio.h>
#include    <string.h>
#include    <stdlib.h>

#include    "test_harness.h"
#include    "../include/commc/hash_table.h"
#include    "../include/commc/error.h"

/*
    ==================================
             TEST FUNCTIONS
    ==================================
*/

/*

    test_hash_table_lifecycle()
    ---
    tests hash table creation and destruction

*/

void test_hash_table_lifecycle(void) {

    commc_hash_table_t* table;
    
    printf("\nOUTPUT: TESTING HASH TABLE LIFECYCLE...\n\n");
    
    /* test creating hash tables with various capacities */

    table = commc_hash_table_create(16);
    ASSERT_NOT_NULL(table);
    ASSERT_TRUE(commc_hash_table_size(table) == 0);
    commc_hash_table_destroy(table);
    
    table = commc_hash_table_create(1);
    ASSERT_NOT_NULL(table);
    ASSERT_TRUE(commc_hash_table_size(table) == 0);
    commc_hash_table_destroy(table);
    
    table = commc_hash_table_create(101); /* prime number capacity */
    ASSERT_NOT_NULL(table);
    ASSERT_TRUE(commc_hash_table_size(table) == 0);
    commc_hash_table_destroy(table);
    
    table = commc_hash_table_create(1000); /* large capacity */
    ASSERT_NOT_NULL(table);
    ASSERT_TRUE(commc_hash_table_size(table) == 0);
    commc_hash_table_destroy(table);
    
    printf("\nOUTPUT: HASH TABLE LIFECYCLE SUCCESSFUL...!\n\n");

}

/*

    test_hash_table_null_safety()
    ---
    tests hash table operations with null parameters

*/

void test_hash_table_null_safety(void) {

    const char* test_key = "test_key";
    void* test_value = (void*)0x12345678;
    
    printf("\nTESTING: HASH TABLE NULL SAFETY...\n\n");
    
    /* test creating hash table with zero capacity */

    ASSERT_NULL(commc_hash_table_create(0));
    
    /* test operations with null table */

    commc_hash_table_destroy(NULL);    /* should handle gracefully */
    ASSERT_TRUE(commc_hash_table_insert(NULL, test_key, test_value) == 0);
    ASSERT_NULL(commc_hash_table_get(NULL, test_key));
    commc_hash_table_remove(NULL, test_key);
    ASSERT_TRUE(commc_hash_table_size(NULL) == 0);
    
    /* test operations with null keys (using valid table) */

    {
        commc_hash_table_t* table = commc_hash_table_create(16);
        ASSERT_NOT_NULL(table);
        
        ASSERT_TRUE(commc_hash_table_insert(table, NULL, test_value) == 0);
        ASSERT_NULL(commc_hash_table_get(table, NULL));
        commc_hash_table_remove(table, NULL);
        
        commc_hash_table_destroy(table);
    }
    
    printf("\nPASSED: HASH TABLE NULL SAFETY\n\n");

}

/*

    test_hash_table_basic_operations()
    ---
    tests basic insert, get, and size operations

*/

void test_hash_table_basic_operations(void) {

    commc_hash_table_t* table;
    int test_values[] = {100, 200, 300, 400, 500};
    const char* test_keys[] = {"key1", "key2", "key3", "key4", "key5"};
    void* retrieved_value;
    int i;
    
    printf("\nTESTING: HASH TABLE BASIC OPERATIONS...\n\n");
    
    table = commc_hash_table_create(8);
    ASSERT_NOT_NULL(table);
    
    /* test inserting key-value pairs */

    for (i = 0; i < 5; i++) {

        ASSERT_TRUE(commc_hash_table_insert(table, test_keys[i], &test_values[i]) == 1);
        ASSERT_TRUE(commc_hash_table_size(table) == (size_t)(i + 1));

    }
    
    /* test retrieving values */

    for (i = 0; i < 5; i++) {

        retrieved_value = commc_hash_table_get(table, test_keys[i]);
        ASSERT_NOT_NULL(retrieved_value);
        ASSERT_TRUE(*(int*)retrieved_value == test_values[i]);

    }
    
    /* test retrieving non-existent keys */

    ASSERT_NULL(commc_hash_table_get(table, "nonexistent"));
    ASSERT_NULL(commc_hash_table_get(table, "missing"));
    ASSERT_NULL(commc_hash_table_get(table, ""));
    
    commc_hash_table_destroy(table);
    
    printf("\nPASSED: HASH TABLE BASIC OPERATIONS\n\n");

}

/*

    test_hash_table_key_update()
    ---
    tests updating existing keys with new values

*/

void test_hash_table_key_update(void) {

    commc_hash_table_t* table;
    int original_value = 100;
    int updated_value = 999;
    const char* key = "update_test";
    void* retrieved_value;
    
    printf("\nTESTING: HASH TABLE KEY UPDATE...\n\n");
    
    table = commc_hash_table_create(16);
    ASSERT_NOT_NULL(table);
    
    /* insert initial key-value pair */

    ASSERT_TRUE(commc_hash_table_insert(table, key, &original_value) == 1);
    ASSERT_TRUE(commc_hash_table_size(table) == 1);
    
    retrieved_value = commc_hash_table_get(table, key);
    ASSERT_NOT_NULL(retrieved_value);
    ASSERT_TRUE(*(int*)retrieved_value == original_value);
    
    /* update the same key with new value */

    ASSERT_TRUE(commc_hash_table_insert(table, key, &updated_value) == 1);
    ASSERT_TRUE(commc_hash_table_size(table) == 1); /* size should remain the same */
    
    retrieved_value = commc_hash_table_get(table, key);
    ASSERT_NOT_NULL(retrieved_value);
    ASSERT_TRUE(*(int*)retrieved_value == updated_value);
    
    commc_hash_table_destroy(table);
    
    printf("\nPASSED: HASH TABLE KEY UPDATE\n\n");

}

/*

    test_hash_table_remove_operations()
    ---
    tests removing key-value pairs

*/

void test_hash_table_remove_operations(void) {

    commc_hash_table_t* table;
    int test_values[] = {10, 20, 30, 40, 50};
    const char* test_keys[] = {"remove1", "remove2", "remove3", "remove4", "remove5"};
    int i;
    
    printf("\nTESTING: HASH TABLE REMOVE OPERATIONS...\n\n");
    
    table = commc_hash_table_create(16);
    ASSERT_NOT_NULL(table);
    
    /* insert test data */

    for (i = 0; i < 5; i++) {

        ASSERT_TRUE(commc_hash_table_insert(table, test_keys[i], &test_values[i]) == 1);

    }

    ASSERT_TRUE(commc_hash_table_size(table) == 5);
    
    /* test removing existing keys */

    commc_hash_table_remove(table, test_keys[0]);
    ASSERT_TRUE(commc_hash_table_size(table) == 4);
    ASSERT_NULL(commc_hash_table_get(table, test_keys[0]));
    
    commc_hash_table_remove(table, test_keys[2]);
    ASSERT_TRUE(commc_hash_table_size(table) == 3);
    ASSERT_NULL(commc_hash_table_get(table, test_keys[2]));
    
    /* verify remaining keys still exist */

    ASSERT_NOT_NULL(commc_hash_table_get(table, test_keys[1]));
    ASSERT_NOT_NULL(commc_hash_table_get(table, test_keys[3]));
    ASSERT_NOT_NULL(commc_hash_table_get(table, test_keys[4]));
    
    /* test removing non-existent keys (should be safe) */

    commc_hash_table_remove(table, "nonexistent");
    commc_hash_table_remove(table, test_keys[0]); /* already removed */
    ASSERT_TRUE(commc_hash_table_size(table) == 3); /* size unchanged */
    
    /* remove all remaining keys */

    commc_hash_table_remove(table, test_keys[1]);
    commc_hash_table_remove(table, test_keys[3]);
    commc_hash_table_remove(table, test_keys[4]);
    ASSERT_TRUE(commc_hash_table_size(table) == 0);
    
    commc_hash_table_destroy(table);
    
    printf("\nPASSED: HASH TABLE REMOVE OPERATIONS\n\n");

}

/*

    test_hash_table_collision_handling()
    ---
    tests hash table behavior with hash collisions

*/

void test_hash_table_collision_handling(void) {

    commc_hash_table_t* table;
    int test_values[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    const char* collision_keys[] = {
        "key_a", "key_b", "key_c", "key_d", "key_e", 
        "key_f", "key_g", "key_h", "key_i", "key_j"
    };
    void* retrieved_value;
    int i;
    
    printf("\nTESTING: HASH TABLE COLLISION HANDLING...\n\n");
    
    /* create small table to force collisions */

    table = commc_hash_table_create(3);
    ASSERT_NOT_NULL(table);
    
    /* insert many keys into small table */

    for (i = 0; i < 10; i++) {

        ASSERT_TRUE(commc_hash_table_insert(table, collision_keys[i], &test_values[i]) == 1);
        ASSERT_TRUE(commc_hash_table_size(table) == (size_t)(i + 1));

    }
    
    /* verify all keys can still be retrieved despite collisions */

    for (i = 0; i < 10; i++) {

        retrieved_value = commc_hash_table_get(table, collision_keys[i]);
        ASSERT_NOT_NULL(retrieved_value);
        ASSERT_TRUE(*(int*)retrieved_value == test_values[i]);

    }
    
    /* test removing keys in collision chains */

    commc_hash_table_remove(table, collision_keys[5]);
    ASSERT_TRUE(commc_hash_table_size(table) == 9);
    ASSERT_NULL(commc_hash_table_get(table, collision_keys[5]));
    
    /* verify other keys in same chain still work */

    for (i = 0; i < 10; i++) {

        if  (i != 5) {

            retrieved_value = commc_hash_table_get(table, collision_keys[i]);
            ASSERT_NOT_NULL(retrieved_value);
            ASSERT_TRUE(*(int*)retrieved_value == test_values[i]);

        }

    }
    
    commc_hash_table_destroy(table);
    
    printf("\nPASSED: HASH TABLE COLLISION HANDLING\n\n");

}

/*

    test_hash_table_string_keys()
    ---
    tests hash table with various string key formats

*/

void test_hash_table_string_keys(void) {

    commc_hash_table_t* table;

    const char* string_keys[] = {

        "",                          /* empty string */
        "a",                         /* single character */
        "short",                     /* short string */
        "this_is_a_longer_key_name", /* long string */
        "key with spaces",           /* spaces */
        "key_with_numbers_123",      /* alphanumeric */
        "UPPERCASE_KEY",             /* uppercase */
        "MiXeD_CaSe_KeY",           /* mixed case */
        "key!@#$%^&*()",            /* special characters */
        "very_long_key_name_that_goes_on_and_on_and_should_test_longer_string_handling"

    };

    int string_values[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    void* retrieved_value;
    int i;
    
    printf("\nTESTING: HASH TABLE STRING KEYS...\n\n");
    
    table = commc_hash_table_create(16);
    ASSERT_NOT_NULL(table);
    
    /* test inserting various string key formats */

    for (i = 0; i < 10; i++) {

        ASSERT_TRUE(commc_hash_table_insert(table, string_keys[i], &string_values[i]) == 1);
        ASSERT_TRUE(commc_hash_table_size(table) == (size_t)(i + 1));

    }
    
    /* verify all string keys can be retrieved */

    for (i = 0; i < 10; i++) {

        retrieved_value = commc_hash_table_get(table, string_keys[i]);
        ASSERT_NOT_NULL(retrieved_value);
        ASSERT_TRUE(*(int*)retrieved_value == string_values[i]);

    }
    
    /* test case sensitivity */

    ASSERT_NULL(commc_hash_table_get(table, "SHORT")); /* should not match "short" */
    ASSERT_NULL(commc_hash_table_get(table, "uppercase_key")); /* should not match "UPPERCASE_KEY" */
    
    /* test similar but different keys */

    ASSERT_NULL(commc_hash_table_get(table, "short "));  /* trailing space */
    ASSERT_NULL(commc_hash_table_get(table, " short"));  /* leading space */
    ASSERT_NULL(commc_hash_table_get(table, "shorts"));  /* extra character */
    
    commc_hash_table_destroy(table);
    
    printf("\nPASSED: HASH TABLE STRING KEYS\n\n");

}

/*

    test_hash_table_value_types()
    ---
    tests hash table with different value data types

*/

void test_hash_table_value_types(void) {

    commc_hash_table_t* table;
    int int_value = 42;
    float float_value = 3.14159f;
    char char_value = 'X';
    const char* string_value = "Hello, World!";
    void* null_value = NULL;
    void* retrieved_value;
    
    printf("\nTESTING: HASH TABLE VALUE TYPES...\n\n");
    
    table = commc_hash_table_create(16);
    ASSERT_NOT_NULL(table);
    
    /* test storing different data types */

    ASSERT_TRUE(commc_hash_table_insert(table, "int_key", &int_value) == 1);
    ASSERT_TRUE(commc_hash_table_insert(table, "float_key", &float_value) == 1);
    ASSERT_TRUE(commc_hash_table_insert(table, "char_key", &char_value) == 1);
    ASSERT_TRUE(commc_hash_table_insert(table, "string_key", (void*)string_value) == 1);
    ASSERT_TRUE(commc_hash_table_insert(table, "null_key", null_value) == 1);
    
    ASSERT_TRUE(commc_hash_table_size(table) == 5);
    
    /* test retrieving different data types */

    retrieved_value = commc_hash_table_get(table, "int_key");
    ASSERT_NOT_NULL(retrieved_value);
    ASSERT_TRUE(*(int*)retrieved_value == int_value);
    
    retrieved_value = commc_hash_table_get(table, "float_key");
    ASSERT_NOT_NULL(retrieved_value);
    ASSERT_TRUE(*(float*)retrieved_value == float_value);
    
    retrieved_value = commc_hash_table_get(table, "char_key");
    ASSERT_NOT_NULL(retrieved_value);
    ASSERT_TRUE(*(char*)retrieved_value == char_value);
    
    retrieved_value = commc_hash_table_get(table, "string_key");
    ASSERT_NOT_NULL(retrieved_value);
    ASSERT_TRUE(strcmp((const char*)retrieved_value, string_value) == 0);
    
    /* null values should be retrievable but test carefully */

    retrieved_value = commc_hash_table_get(table, "null_key");

    /* NOTE: this tests that the key exists, even though value is NULL */
    /* we expect get() to return the stored NULL value, not "key not found" NULL */

    commc_hash_table_destroy(table);
    
    printf("\nPASSED: HASH TABLE VALUE TYPES\n\n");

}

/*

    test_hash_table_large_dataset()
    ---
    tests hash table performance with large number of entries

*/

void test_hash_table_large_dataset(void) {

    commc_hash_table_t* table;
    char key_buffer[32];
    int* values;
    void* retrieved_value;
    int num_entries = 1000;
    int i;
    
    printf("\nTESTING: HASH TABLE LARGE DATASET...\n\n");
    
    /* allocate array for test values */

    values = (int*)malloc(num_entries * sizeof(int));
    ASSERT_NOT_NULL(values);
    
    table = commc_hash_table_create(101); /* prime number for better distribution */
    ASSERT_NOT_NULL(table);
    
    /* insert large number of key-value pairs */

    for (i = 0; i < num_entries; i++) {

        values[i] = i * 2; /* some computed value */
        sprintf(key_buffer, "key_%d", i);
        ASSERT_TRUE(commc_hash_table_insert(table, key_buffer, &values[i]) == 1);

    }

    ASSERT_TRUE(commc_hash_table_size(table) == (size_t)num_entries);
    
    /* verify all entries can be retrieved */

    for (i = 0; i < num_entries; i++) {

        sprintf(key_buffer, "key_%d", i);
        retrieved_value = commc_hash_table_get(table, key_buffer);
        ASSERT_NOT_NULL(retrieved_value);
        ASSERT_TRUE(*(int*)retrieved_value == values[i]);

    }
    
    /* remove every 10th entry */

    for (i = 0; i < num_entries; i += 10) {

        sprintf(key_buffer, "key_%d", i);
        commc_hash_table_remove(table, key_buffer);

    }

    ASSERT_TRUE(commc_hash_table_size(table) == (size_t)(num_entries - num_entries / 10));
    
    /* verify removed entries are gone and others remain */

    for (i = 0; i < num_entries; i++) {

        sprintf(key_buffer, "key_%d", i);
        retrieved_value = commc_hash_table_get(table, key_buffer);

        if  (i % 10 == 0) {

            ASSERT_NULL(retrieved_value); /* should be removed */

        } else {

            ASSERT_NOT_NULL(retrieved_value); /* should still exist */
            ASSERT_TRUE(*(int*)retrieved_value == values[i]);

        }

    }
    
    commc_hash_table_destroy(table);
    free(values);
    
    printf("\nPASSED: HASH TABLE LARGE DATASET\n\n");

}

/*

    test_hash_table_complex_scenario()
    ---
    tests a complex scenario combining multiple hash table operations

*/

void test_hash_table_complex_scenario(void) {

    commc_hash_table_t* config_table;
    commc_hash_table_t* user_table;
    
    /* configuration data */

    int max_connections = 100;
    float timeout_seconds = 30.5f;
    const char* server_name = "GameServer";
    int debug_mode = 1;
    
    /* user data structures */

    typedef struct {
        int user_id;
        const char* username;
        int score;
    } user_data_t;
    
    user_data_t users[] = {
        {1, "alice", 1500},
        {2, "bob", 2300},
        {3, "charlie", 890},
        {4, "diana", 3200}
    };
    
    void* retrieved_value;
    user_data_t* user_ptr;
    int i;
    
    printf("\nTESTING: HASH TABLE COMPLEX SCENARIO...\n\n");
    
    /* create configuration hash table */

    config_table = commc_hash_table_create(16);
    ASSERT_NOT_NULL(config_table);
    
    /* store configuration settings */

    ASSERT_TRUE(commc_hash_table_insert(config_table, "max_connections", &max_connections) == 1);
    ASSERT_TRUE(commc_hash_table_insert(config_table, "timeout_seconds", &timeout_seconds) == 1);
    ASSERT_TRUE(commc_hash_table_insert(config_table, "server_name", (void*)server_name) == 1);
    ASSERT_TRUE(commc_hash_table_insert(config_table, "debug_mode", &debug_mode) == 1);
    
    /* create user lookup table */

    user_table = commc_hash_table_create(32);
    ASSERT_NOT_NULL(user_table);
    
    /* store user data by username */

    for (i = 0; i < 4; i++) {

        ASSERT_TRUE(commc_hash_table_insert(user_table, users[i].username, &users[i]) == 1);

    }
    
    /* simulate server operations */

    /* 1. load configuration */

    retrieved_value = commc_hash_table_get(config_table, "max_connections");
    ASSERT_NOT_NULL(retrieved_value);
    ASSERT_TRUE(*(int*)retrieved_value == 100);
    
    retrieved_value = commc_hash_table_get(config_table, "server_name");
    ASSERT_NOT_NULL(retrieved_value);
    ASSERT_TRUE(strcmp((const char*)retrieved_value, "GameServer") == 0);
    
    /* 2. user login */

    user_ptr = (user_data_t*)commc_hash_table_get(user_table, "alice");
    ASSERT_NOT_NULL(user_ptr);
    ASSERT_TRUE(user_ptr->user_id == 1);
    ASSERT_TRUE(user_ptr->score == 1500);
    
    /* 3. update user score */

    user_ptr->score += 250;
    user_ptr = (user_data_t*)commc_hash_table_get(user_table, "alice");
    ASSERT_TRUE(user_ptr->score == 1750);
    
    /* 4. remove inactive user */

    commc_hash_table_remove(user_table, "charlie");
    ASSERT_TRUE(commc_hash_table_size(user_table) == 3);
    ASSERT_NULL(commc_hash_table_get(user_table, "charlie"));
    
    /* 5. update configuration */

    debug_mode = 0;
    ASSERT_TRUE(commc_hash_table_insert(config_table, "debug_mode", &debug_mode) == 1);
    ASSERT_TRUE(commc_hash_table_size(config_table) == 4); /* size unchanged - update */
    
    retrieved_value = commc_hash_table_get(config_table, "debug_mode");
    ASSERT_NOT_NULL(retrieved_value);
    ASSERT_TRUE(*(int*)retrieved_value == 0);
    
    /* cleanup */

    commc_hash_table_destroy(config_table);
    commc_hash_table_destroy(user_table);
    
    printf("\nPASSED: HASH TABLE COMPLEX SCENARIO\n\n");

}

/*
    ==================================
             MAIN TEST RUNNER
    ==================================
*/

int main(void) {

    printf("\n");
    printf("========================================\n");
    printf("  COMMON-C HASH TABLE COMPREHENSIVE TESTS\n");
    printf("========================================\n\n");

    MEMORY_LEAK_CHECK_START();

    /* basic hash table operations */

    ADD_TEST(hash_table_lifecycle);
    ADD_TEST(hash_table_null_safety);

    /* core functionality tests */

    ADD_TEST(hash_table_basic_operations);
    ADD_TEST(hash_table_key_update);
    ADD_TEST(hash_table_remove_operations);

    /* advanced functionality tests */

    ADD_TEST(hash_table_collision_handling);
    ADD_TEST(hash_table_string_keys);
    ADD_TEST(hash_table_value_types);

    /* performance and integration tests */
    
    ADD_TEST(hash_table_large_dataset);
    ADD_TEST(hash_table_complex_scenario);

    RUN_ALL_TESTS();

    MEMORY_LEAK_CHECK_END();

    printf("========================================\n");
    printf("  ALL HASH TABLE TESTS COMPLETED\n");
    printf("========================================\n\n");

    return 0;

}

/*
    ==================================
             --- EOF ---
    ==================================
*/