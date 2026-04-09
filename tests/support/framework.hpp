#pragma once

#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace almo_test {

struct TestCase {
    std::string name;
    std::function<void()> body;
};

inline void expect_true(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

template <class T>
inline void expect_eq(const T& actual, const T& expected,
                      const std::string& message) {
    if (!(actual == expected)) {
        throw std::runtime_error(message);
    }
}

inline void expect_contains(const std::string& text, const std::string& needle,
                            const std::string& message) {
    expect_true(text.find(needle) != std::string::npos, message);
}

inline void expect_not_contains(const std::string& text,
                                const std::string& needle,
                                const std::string& message) {
    expect_true(text.find(needle) == std::string::npos, message);
}

template <class Exception>
inline void expect_throws(const std::function<void()>& body,
                          const std::string& message,
                          const std::string& needle = "") {
    try {
        body();
    } catch (const Exception& error) {
        if (!needle.empty()) {
            expect_contains(error.what(), needle,
                            message + " should include the expected message");
        }
        return;
    } catch (const std::exception& error) {
        throw std::runtime_error(message + " threw unexpected exception: " +
                                 std::string(error.what()));
    }

    throw std::runtime_error(message + " did not throw");
}

inline int run_tests(const std::vector<TestCase>& tests) {
    int failures = 0;

    for (const auto& test : tests) {
        try {
            test.body();
            std::cout << "[PASS] " << test.name << '\n';
        } catch (const std::exception& error) {
            ++failures;
            std::cerr << "[FAIL] " << test.name << ": " << error.what()
                      << '\n';
        }
    }

    if (failures != 0) {
        std::cerr << failures << " test(s) failed" << '\n';
        return 1;
    }

    std::cout << "All tests passed" << '\n';
    return 0;
}

}  // namespace almo_test
