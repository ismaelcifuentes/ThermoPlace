// Minimal test framework (no external dependencies, works with MinGW).
// You do not need to modify this file.
#pragma once

#include <cmath>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#ifndef TP_BENCH_DIR
#define TP_BENCH_DIR "benchmarks"  // run the tests from the repository root
#endif
#ifndef TP_OUT_DIR
#define TP_OUT_DIR "results"
#endif

namespace tt {

struct TestCase {
    std::string group;
    std::string name;
    std::function<void()> fn;
};

inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> r;
    return r;
}

struct Registrar {
    Registrar(const char* g, const char* n, std::function<void()> f) {
        registry().push_back({g, n, std::move(f)});
    }
};

inline int& checksFailedInTest() {
    static int n = 0;
    return n;
}

struct AbortTest {};  // thrown by REQUIRE to stop the current test

inline void reportFailure(const char* file, int line, const std::string& what) {
    ++checksFailedInTest();
    std::cout << "      FAIL " << file << ":" << line << "  " << what << "\n";
}

inline std::string bench(const std::string& rel) { return std::string(TP_BENCH_DIR) + "/" + rel; }
inline std::string out(const std::string& file) { return std::string(TP_OUT_DIR) + "/" + file; }

}  // namespace tt

#define TT_CAT2(a, b) a##b
#define TT_CAT(a, b) TT_CAT2(a, b)

#define TEST(group, name)                                                              \
    static void TT_CAT(test_, TT_CAT(group, TT_CAT(_, name)))();                       \
    static tt::Registrar TT_CAT(reg_, TT_CAT(group, TT_CAT(_, name)))(                 \
        #group, #name, TT_CAT(test_, TT_CAT(group, TT_CAT(_, name))));                  \
    static void TT_CAT(test_, TT_CAT(group, TT_CAT(_, name)))()

#define CHECK(cond)                                                                    \
    do {                                                                               \
        if (!(cond)) tt::reportFailure(__FILE__, __LINE__, "CHECK(" #cond ")");         \
    } while (0)

#define REQUIRE(cond)                                                                  \
    do {                                                                               \
        if (!(cond)) {                                                                 \
            tt::reportFailure(__FILE__, __LINE__, "REQUIRE(" #cond ")");               \
            throw tt::AbortTest{};                                                     \
        }                                                                              \
    } while (0)

#define CHECK_EQ(a, b)                                                                 \
    do {                                                                               \
        auto va_ = (a);                                                                \
        auto vb_ = (b);                                                                \
        if (!(va_ == vb_)) {                                                           \
            std::ostringstream os_;                                                    \
            os_ << "CHECK_EQ(" #a ", " #b ")  got " << va_ << " vs " << vb_;           \
            tt::reportFailure(__FILE__, __LINE__, os_.str());                          \
        }                                                                              \
    } while (0)

#define CHECK_NEAR(a, b, tol)                                                          \
    do {                                                                               \
        double va_ = (a);                                                              \
        double vb_ = (b);                                                              \
        if (!(std::fabs(va_ - vb_) <= (tol))) {                                        \
            std::ostringstream os_;                                                    \
            os_ << "CHECK_NEAR(" #a ", " #b ")  got " << va_ << " vs " << vb_;         \
            tt::reportFailure(__FILE__, __LINE__, os_.str());                          \
        }                                                                              \
    } while (0)
