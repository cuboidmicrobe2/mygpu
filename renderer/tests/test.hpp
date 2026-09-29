#pragma once

#include <cstdio>
#include <exception>

// C++ version of tests/test.h. Passing checks print nothing; each failure prints its location,
// and TEST_FINISH() prints one summary line and returns the process exit code.
//
//   REQUIRE(cond)  record a failure and skip the rest of the current test
//   RUN_TEST(fn)   run a test so a failed REQUIRE doesn't stop the others

namespace test {

inline int checksRun = 0;
inline int checksFailed = 0;
inline const char *currentTest = nullptr;

// Thrown by REQUIRE; unwinding runs the destructors of everything the test created.
struct Abort {};

inline void Fail(const char *file, int line, const char *what) {
    checksFailed++;
    std::printf("FAIL %s:%d: %s: %s\n", file, line, currentTest, what);
}

inline void Run(void (*fn)(), const char *name) {
    currentTest = name;

    try {
        fn();
    } catch (const Abort &) {
        // Already reported by REQUIRE.
    } catch (const std::exception &e) {
        Fail(__FILE__, __LINE__, e.what());
    }

    currentTest = nullptr;
}

inline int Finish(const char *file) {
    if (checksFailed == 0) {
        std::printf("%s: all %d passed\n", file, checksRun);
        return 0;
    }

    std::printf("%s: %d of %d failed\n", file, checksFailed, checksRun);
    return 1;
}

} // namespace test

#define REQUIRE(condition)                                                                                             \
    do {                                                                                                               \
        test::checksRun++;                                                                                             \
        if (!(condition)) {                                                                                            \
            test::Fail(__FILE__, __LINE__, #condition);                                                                \
            throw test::Abort{};                                                                                       \
        }                                                                                                              \
    } while (0)

#define RUN_TEST(fn) test::Run((fn), #fn)
#define TEST_FINISH() test::Finish(__FILE__)
