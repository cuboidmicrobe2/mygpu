#ifndef MYGPU_TEST_H
#define MYGPU_TEST_H

#include <setjmp.h>
#include <stdio.h>

/*
 * Minimal test helpers. Passing checks print nothing; each failure prints its location, and
 * test_finish() prints a single summary line and returns the process exit code.
 *
 *   check(cond, name)  record a failure and keep going
 *   require(cond)      record a failure and skip the rest of the current test; only valid inside run_test()
 *   run_test(fn)       run a test function so require() can abort it without stopping the others
 */

static int test_checks_run;
static int test_checks_failed;
static const char *test_current;
static jmp_buf test_abort;

static inline void test_fail(const char *file, int line, const char *what)
{
    test_checks_failed++;

    if (test_current != NULL) {
        printf("FAIL %s:%d: %s: %s\n", file, line, test_current, what);
    } else {
        printf("FAIL %s:%d: %s\n", file, line, what);
    }
}

static inline void test_check(int condition, const char *name, const char *file, int line)
{
    test_checks_run++;

    if (!condition) {
        test_fail(file, line, name);
    }
}

#define check(condition, name) test_check((condition), (name), __FILE__, __LINE__)

#define require(condition)                                                                                             \
    do {                                                                                                               \
        test_checks_run++;                                                                                             \
        if (!(condition)) {                                                                                            \
            test_fail(__FILE__, __LINE__, #condition);                                                                 \
            longjmp(test_abort, 1);                                                                                    \
        }                                                                                                              \
    } while (0)

static inline void test_run(void (*fn)(void), const char *name)
{
    test_current = name;

    if (setjmp(test_abort) == 0) {
        fn();
    }

    test_current = NULL;
}

#define run_test(fn) test_run((fn), #fn)

static inline int test_summary(const char *file)
{
    if (test_checks_failed == 0) {
        printf("%s: all %d passed\n", file, test_checks_run);
        return 0;
    }

    printf("%s: %d of %d failed\n", file, test_checks_failed, test_checks_run);
    return 1;
}

#define test_finish() test_summary(__FILE__)

#endif
