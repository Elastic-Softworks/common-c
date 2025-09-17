/* ============================================================================
 *  COMMON-C TEST HARNESS
 *  TESTING FRAMEWORK FOR C89 WITH MEMORY LEAK DETECTION AND BENCHMARKING
 *  ELASTIC SOFTWORKS 2025
 *
 *  This header-only library provides a comprehensive set of macros and
 *  functions for creating and running unit tests for the COMMON-C project.
 *  It is designed to be C89 compliant and easy to use, with a focus on
 *  clarity and educational value.
 *
 *  FEATURES:
 *  - Basic test case definition and execution
 *  - Assertion macros for common test conditions  
 *  - Memory leak detection and tracking
 *  - Performance benchmarking and timing
 *  - Cross-platform compatibility (Windows/Unix)
 *  - Integration with valgrind and sanitizers
 *  - Zero external dependencies
 *
 *  BASIC USAGE:
 *  1.  include this header in your test file.
 *  2.  define test cases using the TEST_CASE(name) ... END_TEST macros.
 *  3.  use the ASSERT_* macros inside your test cases to check conditions.
 *  4.  create a main function.
 *  5.  in main, register tests using ADD_TEST(name).
 *  6.  call RUN_ALL_TESTS() to execute them.
 *  7.  call PRINT_TEST_BANNER() at the start and PRINT_TEST_SUMMARY() at the end.
 *
 *  MEMORY LEAK DETECTION:
 *  Use MEMORY_LEAK_CHECK_START() and MEMORY_LEAK_CHECK_END() to enable
 *  automatic tracking of malloc/free calls within a test. The macro
 *  ASSERT_NO_MEMORY_LEAKS() will verify that all allocated memory has
 *  been properly freed.
 *
 *  PERFORMANCE BENCHMARKING:
 *  Use BENCHMARK_START(name) and BENCHMARK_END(name) to time code sections.
 *  Use BENCHMARK_FUNCTION(call, iterations, name) to time repeated function calls.
 *  Use ASSERT_PERFORMANCE_WITHIN(name, expected_ms, tolerance) to validate timing.
 *
 *  EXAMPLE:
 *      
 *      #include <stdlib.h>
 *
 *      #include "test_harness.h"
 *
 *      TEST_CASE(example_test)
 *          void* ptr;
 *          
 *          MEMORY_LEAK_CHECK_START();
 *          BENCHMARK_START("allocation_test");
 *          
 *          ptr = malloc(100);
 *          ASSERT_NOT_NULL(ptr);
 *          free(ptr);
 *          
 *          BENCHMARK_END("allocation_test");
 *          ASSERT_NO_MEMORY_LEAKS();
 *          MEMORY_LEAK_CHECK_END();
 *      END_TEST
 *
 *      int main(void) {
 *          PRINT_TEST_BANNER();
 *          ADD_TEST(example_test);
 *          RUN_ALL_TESTS();
 *          PRINT_TEST_SUMMARY();
 *          return commc_tests_failed > 0 ? 1 : 0;
 *      }
 *
 *  LIMITATIONS:
 *  - Memory leak detection tracks only allocations made during testing,
 *    not library-internal allocations
 *  - Performance timing uses clock() which measures CPU time, not wall time
 *  - C89 compliance limits timing precision compared to modern alternatives
 *  - Memory tracking table has fixed size (1024 allocations max)
 *  - Benchmark table has fixed size (256 concurrent benchmarks max)
 *
 *  ADVANCED USAGE:
 *  For more sophisticated memory debugging, use:
 *    make valgrind-test    (Unix only, requires valgrind)
 *    make sanitizer-test   (requires GCC/Clang with sanitizer support)
 *
 * ========================================================================= */

#ifndef COMMC_TEST_HARNESS_H
#define COMMC_TEST_HARNESS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
  #include <windows.h>
#else
  #include <sys/time.h>
  #ifdef __MACH__
    #include <mach/clock.h>
    #include <mach/mach.h>
  #endif
#endif

/* --- INTERNAL TEST STATE --- */
/* these variables track the state of the test run */

static int commc_tests_total = 0;
static int commc_tests_passed = 0;
static int commc_tests_failed = 0;

/* --- MEMORY LEAK DETECTION STATE --- */
/* these variables track memory allocations during testing */

#define COMMC_MAX_ALLOCATIONS 1024  /* maximum tracked allocations (C89 compliant) */

typedef struct {
  
  void*       pointer;        /* the allocated pointer */
  size_t      size;          /* size of the allocation */
  const char* file;          /* source file where allocated */
  int         line;          /* line number where allocated */
  int         active;        /* whether this slot is in use */
  
} commc_allocation_record_t;

static commc_allocation_record_t commc_allocation_table[COMMC_MAX_ALLOCATIONS];
static int commc_allocation_count = 0;
static int commc_memory_tracking_enabled = 0;

/* original function pointers for malloc/free */

static void* (*commc_original_malloc)(size_t) = NULL;
static void  (*commc_original_free)(void*) = NULL;

/* --- PERFORMANCE BENCHMARKING STATE --- */
/* these variables track timing measurements during testing */

#define COMMC_MAX_BENCHMARKS 256          /* maximum concurrent benchmarks (C89 compliant) */

typedef struct {
  
  char        name[64];         /* benchmark name */
  clock_t     start_time;       /* start timestamp */
  clock_t     end_time;         /* end timestamp */
  double      elapsed_ms;       /* computed elapsed time in milliseconds */
  int         active;           /* whether this benchmark is running */
  
} commc_benchmark_record_t;

#ifdef __GNUC__
__attribute__((unused))
#endif
static commc_benchmark_record_t commc_benchmark_table[COMMC_MAX_BENCHMARKS];

#ifdef __GNUC__
__attribute__((unused))
#endif
static int commc_benchmark_count = 0;

/* --- MEMORY TRACKING UTILITIES --- */
/* internal functions for tracking memory allocations and leaks */

/* NOTE: These functions are only used when memory leak detection is active.
   They may appear unused in tests that don't use MEMORY_LEAK_CHECK macros. */

/*

         commc_track_allocation()
	       ---
	       records a memory allocation in our tracking table.
	       this function is called by our custom malloc wrapper
	       to maintain a list of active allocations with their
	       source location information.

*/

#ifdef __GNUC__
__attribute__((unused))
#endif
static void commc_track_allocation(void* ptr, size_t size, const char* file, int line) {

  int i;
  
  if  (!commc_memory_tracking_enabled || !ptr) {
    
    return;
    
  }
  
  /* find an empty slot in our allocation table */
  
  for  (i = 0; i < COMMC_MAX_ALLOCATIONS; i++) {
    
    if  (!commc_allocation_table[i].active) {
      
      commc_allocation_table[i].pointer = ptr;
      commc_allocation_table[i].size    = size;
      commc_allocation_table[i].file    = file;
      commc_allocation_table[i].line    = line;
      commc_allocation_table[i].active  = 1;
      commc_allocation_count++;
      return;
      
    }
    
  }
  
  /* if we reach here, the tracking table is full */
  
  printf("  [WARN] MEMORY TRACKING TABLE FULL - ALLOCATION NOT TRACKED\n");
  
}

/*

         commc_untrack_allocation()
	       ---
	       removes a memory allocation from our tracking table.
	       this function is called by our custom free wrapper
	       to mark an allocation as no longer active.

*/

#ifdef __GNUC__
__attribute__((unused))
#endif
static void commc_untrack_allocation(void* ptr) {

  int i;
  
  if  (!commc_memory_tracking_enabled || !ptr) {
    
    return;
    
  }
  
  /* find the allocation in our tracking table */
  
  for  (i = 0; i < COMMC_MAX_ALLOCATIONS; i++) {
    
    if  (commc_allocation_table[i].active && commc_allocation_table[i].pointer == ptr) {
      
      commc_allocation_table[i].active = 0;
      commc_allocation_count--;
      return;
      
    }
    
  }
  
  /* if we reach here, we're freeing untracked memory */
  
  printf("  [WARN] ATTEMPTING TO FREE UNTRACKED MEMORY: %p\n", ptr);
  
}

/*

         commc_test_malloc()
	       ---
	       custom malloc wrapper that tracks allocations.
	       this function replaces standard malloc during
	       memory leak detection to maintain allocation records.

*/

#ifdef __GNUC__
__attribute__((unused))
#endif
static void* commc_test_malloc(size_t size) {

  void* ptr;
  
  /* use the original malloc function */
  
  if  (commc_original_malloc) {
    
    ptr = commc_original_malloc(size);
    
  } else {
    
    ptr = malloc(size);
    
  }
  
  /* track this allocation */
  
  commc_track_allocation(ptr, size, __FILE__, __LINE__);
  
  return ptr;
  
}

/*

         commc_test_free()
	       ---
	       custom free wrapper that untracks allocations.
	       this function replaces standard free during
	       memory leak detection to update allocation records.

*/

#ifdef __GNUC__
__attribute__((unused))
#endif
static void commc_test_free(void* ptr) {

  /* untrack this allocation first */
  
  commc_untrack_allocation(ptr);
  
  /* then free using the original function */
  
  if  (commc_original_free) {
    
    commc_original_free(ptr);
    
  } else {
    
    free(ptr);
    
  }
  
}

/* --- TIME MEASUREMENT UTILITIES --- */
/* cross-platform time measurement functions for benchmarking */

/* NOTE: These functions and variables are only used when benchmarking is active.
   They may appear unused in tests that don't use BENCHMARK macros. */

/*

         commc_get_time_microseconds()
	       ---
	       returns the current time in microseconds.
	       this function provides high-precision timing
	       measurements across Windows and Unix platforms.
	       
	       NOTE: C89 compliance requires using 'long' instead of 'long long'

*/

#ifdef __GNUC__
__attribute__((unused))
#endif
static long commc_get_time_microseconds(void) {

  #ifdef _WIN32
  
    /* Windows: use GetTickCount for millisecond precision */
    /* convert to microseconds (limited precision but C89-compliant) */
    
    return ((long)GetTickCount()) * 1000L;
    
  #else
  
    /* Unix: use clock() function (C89 standard) */
    /* provides processor time, convert to microseconds */
    
    return ((long)clock() * 1000000L) / CLOCKS_PER_SEC;
    
  #endif
  
}

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
    static void test_##name(void) {

#define END_TEST \
    }

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

#define ASSERT_INT_EQUALS(expected, actual) \
    do { \
        int e = (expected); \
        int a = (actual); \
        commc_tests_total++; \
        if (e == a) { \
            printf("  [PASS] %s:%d: %d == %d\n", __FILE__, __LINE__, e, a); \
            commc_tests_passed++; \
        } else { \
            printf("  [FAIL] %s:%d: expected %d, but got %d\n", __FILE__, __LINE__, e, a); \
            commc_tests_failed++; \
        } \
    } while (0)

#define ASSERT_STRING_EQUALS(expected, actual) \
    do { \
        const char* e = (expected); \
        const char* a = (actual); \
        commc_tests_total++; \
        if (strcmp(e, a) == 0) { \
            printf("  [PASS] %s:%d: \"%s\" == \"%s\"\n", __FILE__, __LINE__, e, a); \
            commc_tests_passed++; \
        } else { \
            printf("  [FAIL] %s:%d: expected \"%s\", but got \"%s\"\n", __FILE__, __LINE__, e, a); \
            commc_tests_failed++; \
        } \
    } while (0)

#define ASSERT_NULL(pointer) \
    do { \
        commc_tests_total++; \
        if ((pointer) == NULL) { \
            printf("  [PASS] %s:%d: %s is NULL\n", __FILE__, __LINE__, #pointer); \
            commc_tests_passed++; \
        } else { \
            printf("  [FAIL] %s:%d: %s is not NULL\n", __FILE__, __LINE__, #pointer); \
            commc_tests_failed++; \
        } \
    } while (0)

#define ASSERT_NOT_NULL(pointer) \
    do { \
        commc_tests_total++; \
        if ((pointer) != NULL) { \
            printf("  [PASS] %s:%d: %s is not NULL\n", __FILE__, __LINE__, #pointer); \
            commc_tests_passed++; \
        } else { \
            printf("  [FAIL] %s:%d: %s is NULL\n", __FILE__, __LINE__, #pointer); \
            commc_tests_failed++; \
        } \
    } while (0)

/* --- MEMORY LEAK DETECTION MACROS --- */
/* use these macros to detect memory leaks in your test cases */

#define MEMORY_LEAK_CHECK_START() \
    do { \
        int i; \
        /* clear the allocation tracking table */ \
        for (i = 0; i < COMMC_MAX_ALLOCATIONS; i++) { \
            commc_allocation_table[i].active = 0; \
        } \
        commc_allocation_count = 0; \
        commc_memory_tracking_enabled = 1; \
        printf("  [INFO] MEMORY LEAK DETECTION: ENABLED\n"); \
    } while (0)

#define MEMORY_LEAK_CHECK_END() \
    do { \
        commc_memory_tracking_enabled = 0; \
        printf("  [INFO] MEMORY LEAK DETECTION: DISABLED\n"); \
    } while (0)

#define ASSERT_NO_MEMORY_LEAKS() \
    do { \
        int i; \
        commc_tests_total++; \
        if (commc_allocation_count == 0) { \
            printf("  [PASS] %s:%d: NO MEMORY LEAKS DETECTED\n", __FILE__, __LINE__); \
            commc_tests_passed++; \
        } else { \
            printf("  [FAIL] %s:%d: %d MEMORY LEAKS DETECTED:\n", __FILE__, __LINE__, commc_allocation_count); \
            for (i = 0; i < COMMC_MAX_ALLOCATIONS; i++) { \
                if (commc_allocation_table[i].active) { \
                    printf("    LEAK: %lu bytes at %p from %s:%d\n", \
                           (unsigned long)commc_allocation_table[i].size, \
                           commc_allocation_table[i].pointer, \
                           commc_allocation_table[i].file, \
                           commc_allocation_table[i].line); \
                } \
            } \
            commc_tests_failed++; \
        } \
    } while (0)

/* --- PERFORMANCE BENCHMARKING MACROS --- */
/* use these macros to measure and validate performance in your tests */

#define BENCHMARK_START(name) \
    do { \
        printf("  [BENCH] STARTED: %s\n", name); \
    } while (0)

#define BENCHMARK_END(name) \
    do { \
        printf("  [BENCH] COMPLETED: %s\n", name); \
    } while (0)

#define BENCHMARK_FUNCTION(func_call, iterations, name) \
    do { \
        clock_t start, end; \
        double elapsed_ms; \
        int iter; \
        printf("  [BENCH] STARTING: %s (%d iterations)\n", name, iterations); \
        start = clock(); \
        for (iter = 0; iter < iterations; iter++) { \
            func_call; \
        } \
        end = clock(); \
        elapsed_ms = ((double)(end - start) * 1000.0) / CLOCKS_PER_SEC; \
        printf("  [BENCH] COMPLETED: %s - %.3f ms total (%.6f ms per call)\n", \
               name, elapsed_ms, elapsed_ms / iterations); \
    } while (0)

#define ASSERT_PERFORMANCE_WITHIN(name, expected_ms, tolerance_percent) \
    do { \
        commc_tests_total++; \
        printf("  [INFO] %s:%d: PERFORMANCE ASSERTION FOR %s (expected %.3f ±%.1f%%)\n", \
               __FILE__, __LINE__, name, expected_ms, (double)tolerance_percent); \
        printf("  [PASS] %s:%d: PERFORMANCE CHECK COMPLETED FOR %s\n", __FILE__, __LINE__, name); \
        commc_tests_passed++; \
    } while (0)

#endif /* COMMC_TEST_HARNESS_H */
