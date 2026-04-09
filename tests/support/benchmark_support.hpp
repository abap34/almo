#pragma once

#include <algorithm>
#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "almo_test_support.hpp"

namespace almo_test {

struct BenchmarkResult {
    std::string name;
    int iterations;
    std::size_t work_units;
    double min_ms;
    double mean_ms;
    double max_ms;
};

struct BenchmarkScenario {
    std::string name;
    std::size_t work_units;
    std::function<void()> body;
};

inline BenchmarkScenario make_parse_scenario(
    const std::string& name, const std::vector<std::string>& document) {
    return {
        name,
        document.size(),
        [document]() {
            almo::Markdown ast;
            almo::MarkdownParser parser(document);
            parser.process(ast);
        },
    };
}

inline BenchmarkScenario make_render_scenario(
    const std::string& name, const std::vector<std::string>& document,
    const std::function<void(const almo::ParseSummary&)>& validate = nullptr) {
    return {
        name,
        document.size(),
        [document, validate]() {
            auto meta = default_meta();
            const auto summary = almo::md_to_summary(document, meta);
            if (validate) {
                validate(summary);
                return;
            }
            if (summary.html.empty()) {
                throw std::runtime_error("rendered HTML must not be empty");
            }
        },
    };
}

inline BenchmarkScenario make_summary_scenario(
    const std::string& name, const std::vector<std::string>& document) {
    return {
        name,
        document.size(),
        [document]() {
            auto meta = default_meta();
            const auto summary = almo::md_to_summary(document, meta);
            if (summary.html.empty() || summary.json.empty() ||
                summary.dot.empty()) {
                throw std::runtime_error("summary outputs must not be empty");
            }
        },
    };
}

inline BenchmarkResult run_benchmark(const BenchmarkScenario& scenario,
                                     int iterations) {
    std::vector<double> samples;
    samples.reserve(iterations);

    scenario.body();

    for (int i = 0; i < iterations; ++i) {
        const auto start = std::chrono::steady_clock::now();
        scenario.body();
        const auto end = std::chrono::steady_clock::now();
        const std::chrono::duration<double, std::milli> elapsed = end - start;
        samples.push_back(elapsed.count());
    }

    const auto [min_it, max_it] =
        std::minmax_element(samples.begin(), samples.end());
    const double total =
        std::accumulate(samples.begin(), samples.end(), 0.0);

    return {
        scenario.name,
        iterations,
        scenario.work_units,
        *min_it,
        total / static_cast<double>(samples.size()),
        *max_it,
    };
}

inline std::string to_json(const std::vector<BenchmarkResult>& results) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3);
    oss << "{\n  \"benchmarks\": [\n";

    for (std::size_t i = 0; i < results.size(); ++i) {
        const auto& result = results[i];
        oss << "    {\n";
        oss << "      \"name\": \"" << result.name << "\",\n";
        oss << "      \"iterations\": " << result.iterations << ",\n";
        oss << "      \"work_units\": " << result.work_units << ",\n";
        oss << "      \"min_ms\": " << result.min_ms << ",\n";
        oss << "      \"mean_ms\": " << result.mean_ms << ",\n";
        oss << "      \"max_ms\": " << result.max_ms << '\n';
        oss << "    }";
        if (i + 1 != results.size()) {
            oss << ",";
        }
        oss << '\n';
    }

    oss << "  ]\n}\n";
    return oss.str();
}

inline void print_human_readable(const std::vector<BenchmarkResult>& results) {
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Benchmark results (ms)\n";

    for (const auto& result : results) {
        std::cout << "- " << result.name << ": mean=" << result.mean_ms
                  << ", min=" << result.min_ms << ", max=" << result.max_ms
                  << ", iterations=" << result.iterations
                  << ", work_units=" << result.work_units << '\n';
    }
}

inline int parse_iterations(const std::string& value) {
    try {
        const int iterations = std::stoi(value);
        if (iterations < 1) {
            throw std::out_of_range("iterations must be positive");
        }
        return iterations;
    } catch (const std::exception&) {
        throw std::runtime_error(
            "Invalid value for --iterations: expected an integer >= 1");
    }
}

}  // namespace almo_test
