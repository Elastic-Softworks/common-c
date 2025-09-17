/* ============================================================================
 *  COMMON-C MEMORY LEAK DETECTION DEMO
 *  MEMORY TRACKING DEMO
 *  ELASTIC SOFTWORKS 2025
 *
 *  this file demonstrates how to use the memory leak detection features
 *  built into the COMMON-C test harness. it shows both correct memory
 *  management and intentional leaks for educational purposes.
 *
 *  the memory leak detection system tracks malloc/free calls and reports
 *  any memory that is allocated but never freed. this is essential for
 *  systems programming where memory leaks can degrade performance over time.
 * ========================================================================= */

#include "test_harness.h"
#include <stdlib.h>  /* for malloc, free */

/* --- DEMONSTRATION TEST CASES --- */

TEST_CASE(memory_correct_allocation)

    /* this test demonstrates correct memory allocation and cleanup */
    
    void* ptr1;
    void* ptr2;
    
    MEMORY_LEAK_CHECK_START();
    
    /* allocate some memory */
    
    ptr1 = malloc(100);
    ASSERT_NOT_NULL(ptr1);
    
    ptr2 = malloc(256);  
    ASSERT_NOT_NULL(ptr2);
    
    /* use the memory (simulate actual work) */
    
    memset(ptr1, 0xAA, 100);
    memset(ptr2, 0xBB, 256);
    
    /* properly free all allocations */
    
    free(ptr1);
    free(ptr2);
    
    /* check that no leaks occurred */
    
    ASSERT_NO_MEMORY_LEAKS();
    
    MEMORY_LEAK_CHECK_END();

END_TEST

TEST_CASE(memory_intentional_leak_demo)

    /* this test demonstrates what happens when memory is leaked.
       this is intentionally failing to show leak detection in action. */
    
    void* leaked_ptr;
    
    MEMORY_LEAK_CHECK_START();
    
    /* allocate memory but "forget" to free it */
    
    leaked_ptr = malloc(50);
    ASSERT_NOT_NULL(leaked_ptr);
    
    /* simulate some work with the memory */
    
    memset(leaked_ptr, 0xCC, 50);
    
    /* NOTE: we intentionally do NOT call free(leaked_ptr) here
       to demonstrate the leak detection capability */
    
    /* this assertion should fail and report the leak */
    
    ASSERT_NO_MEMORY_LEAKS();
    
    MEMORY_LEAK_CHECK_END();
    
    /* clean up the leak to prevent issues in subsequent tests */
    
    free(leaked_ptr);

END_TEST

TEST_CASE(memory_multiple_allocations)

    /* this test demonstrates tracking multiple allocations
       and shows cleanup of a subset (mixed scenario) */
       
    void* ptrs[5];
    int   i;
    
    MEMORY_LEAK_CHECK_START();
    
    /* allocate several blocks of different sizes */
    
    for  (i = 0; i < 5; i++) {
    
        ptrs[i] = malloc((i + 1) * 32);
        ASSERT_NOT_NULL(ptrs[i]);
        
    }
    
    /* free only some of them (intentional partial leak) */
    
    free(ptrs[0]);
    free(ptrs[2]); 
    free(ptrs[4]);
    
    /* this should report leaks for ptrs[1] and ptrs[3] */
    
    ASSERT_NO_MEMORY_LEAKS();
    
    MEMORY_LEAK_CHECK_END();
    
    /* clean up remaining leaks */
    
    free(ptrs[1]);
    free(ptrs[3]);

END_TEST

TEST_CASE(memory_zero_allocation)

    /* this test demonstrates behavior with edge cases
       like zero-byte allocations */
       
    void* zero_ptr;
    void* normal_ptr;
    
    MEMORY_LEAK_CHECK_START();
    
    /* allocate zero bytes (implementation-defined behavior) */
    
    zero_ptr = malloc(0);
    
    /* zero_ptr might be NULL or a unique pointer - both are valid */
    
    if  (zero_ptr) {
    
        free(zero_ptr);
        
    }
    
    /* allocate and free normally */
    
    normal_ptr = malloc(64);
    ASSERT_NOT_NULL(normal_ptr);
    free(normal_ptr);
    
    /* should have no leaks */
    
    ASSERT_NO_MEMORY_LEAKS();
    
    MEMORY_LEAK_CHECK_END();

END_TEST

/* --- TEST RUNNER --- */

int main(void) {

    PRINT_TEST_BANNER();

    /* add all memory detection demo tests */

    ADD_TEST(memory_correct_allocation);
    ADD_TEST(memory_intentional_leak_demo);
    ADD_TEST(memory_multiple_allocations);
    ADD_TEST(memory_zero_allocation);

    /* run all tests */

    RUN_ALL_TESTS();

    /* print summary */

    PRINT_TEST_SUMMARY();

    return commc_tests_failed > 0 ? 1 : 0;

}

/*
	==================================
             --- EOF ---
	==================================
*/