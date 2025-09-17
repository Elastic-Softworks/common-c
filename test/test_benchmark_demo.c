/* ============================================================================
 *  COMMON-C PERFORMANCE BENCHMARKING DEMO
 *  DEMONSTRATION OF TIMING AND PERFORMANCE FEATURES
 *  ELASTIC SOFTWORKS 2025
 *
 *  this file demonstrates how to use the performance benchmarking features
 *  built into the COMMON-C test harness. it shows how to measure execution
 *  time, compare performance, and validate that functions meet performance
 *  requirements.
 *
 *  performance benchmarking is crucial for systems programming where
 *  efficiency matters. these tools help identify bottlenecks and ensure
 *  code meets performance specifications across different platforms.
 * ========================================================================= */

#include <stdlib.h>             /* for malloc, free, rand */
#include <string.h>             /* for memset, memcpy */

#include "test_harness.h"

/* --- HELPER FUNCTIONS FOR BENCHMARKING --- */

/*

         expensive_computation()
	       ---
	       simulates an expensive computational task.
	       this function performs meaningless work to consume CPU time
	       in a predictable way for benchmarking purposes.

*/

static void expensive_computation(int iterations) {

  volatile int result = 0;          /* volatile prevents optimization */
  int          i;
  int          j;

  for  (i = 0; i < iterations; i++) {

    for  (j = 0; j < 100; j++) {

      result += (i * j) % 7;

    }

  }

}

/*

         memory_intensive_task()
	       ---
	       simulates memory-intensive operations.
	       allocates, fills, and copies memory blocks
	       to test memory bandwidth and allocation overhead.

*/

static void memory_intensive_task(size_t block_size, int block_count) {

  void** blocks;
  int    i;

  blocks = (void**)malloc(block_count * sizeof(void*));

  if  (!blocks) {

    return;

  }

  /* allocate blocks */

  for  (i = 0; i < block_count; i++) {

    blocks[i] = malloc(block_size);

    if  (blocks[i]) {

      memset(blocks[i], i % 256, block_size);

    }

  }

  /* copy between blocks (if we have at least 2) */

  if  (block_count >= 2 && blocks[0] && blocks[1]) {

    memcpy(blocks[1], blocks[0], block_size);

  }

  /* free all blocks */

  for  (i = 0; i < block_count; i++) {

    if  (blocks[i]) {

      free(blocks[i]);

    }

  }

  free(blocks);

}

/* --- DEMONSTRATION TEST CASES --- */

TEST_CASE(benchmark_basic_timing)

    /* this test demonstrates basic start/stop timing */

    int i;

    BENCHMARK_START("basic_loop");

    /* do some work that should take measurable time */

    for  (i = 0; i < 100000; i++) {

      /* volatile prevents optimization, but we don't need the result */

      (void)(i * i);

    }

    BENCHMARK_END("basic_loop");

    /* the timing result is automatically printed */

END_TEST

TEST_CASE(benchmark_function_calls)

    /* this test demonstrates function call benchmarking */

    BENCHMARK_FUNCTION(expensive_computation(500), 10, "expensive_computation");

    /* this will call expensive_computation(500) ten times 
       and report total time plus per-call average */

END_TEST

TEST_CASE(benchmark_memory_operations)

    /* this test compares different memory allocation strategies */

    BENCHMARK_START("small_allocations");
    memory_intensive_task(64, 100);
    BENCHMARK_END("small_allocations");

    BENCHMARK_START("large_allocations");
    memory_intensive_task(4096, 10);
    BENCHMARK_END("large_allocations");

    /* compare which approach is faster */

END_TEST

TEST_CASE(benchmark_with_assertions)

    /* this test demonstrates performance validation */

    BENCHMARK_START("string_operations");

    /* perform some string operations */

    {
        char* buffer = malloc(1000);
        int   i;

        if  (buffer) {

          for  (i = 0; i < 100; i++) {

            memset(buffer, 'A' + (i % 26), 1000);

          }

          free(buffer);

        }

    }

    BENCHMARK_END("string_operations");

    /* assert that the operation completed within reasonable time */
    /* NOTE: these values are examples - adjust based on your system */

    ASSERT_PERFORMANCE_WITHIN("string_operations", 10.0, 50);
    
    /* this asserts the "string_operations" benchmark completed within
       10ms ±50% (i.e., between 5ms and 15ms) */

END_TEST

TEST_CASE(benchmark_comparison_demo)

    /* this test demonstrates comparing different algorithms */

    int*  data;
    int   i;
    int   size = 1000;

    /* prepare test data */

    data = (int*)malloc(size * sizeof(int));

    if  (!data) {

      return;

    }

    for  (i = 0; i < size; i++) {

      data[i] = rand() % 1000;

    }

    /* benchmark linear search */

    BENCHMARK_START("linear_search");

    {
        int target = data[size / 2];        /* search for middle element */
        int found  = 0;

        for  (i = 0; i < size && !found; i++) {

          if  (data[i] == target) {

            found = 1;

          }

        }

    }

    BENCHMARK_END("linear_search");

    /* benchmark binary search would go here if we had sorted data */
    /* for now, demonstrate different access patterns */

    BENCHMARK_START("random_access");

    {
        volatile int sum = 0;

        for  (i = 0; i < 100; i++) {

          sum += data[rand() % size];

        }

    }

    BENCHMARK_END("random_access");

    BENCHMARK_START("sequential_access");

    {
        volatile int sum = 0;

        for  (i = 0; i < size; i++) {

          sum += data[i];

        }

    }

    BENCHMARK_END("sequential_access");

    free(data);

    /* sequential access should generally be faster than random access */

END_TEST

/* --- TEST RUNNER --- */

int main(void) {

    PRINT_TEST_BANNER();

    /* add all performance benchmark demo tests */

    ADD_TEST(benchmark_basic_timing);
    ADD_TEST(benchmark_function_calls);
    ADD_TEST(benchmark_memory_operations);
    ADD_TEST(benchmark_with_assertions);
    ADD_TEST(benchmark_comparison_demo);

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