#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "support/benchmark_scenarios.hpp"
#include "support/benchmark_support.hpp"

int main(int argc, char* argv[]) {
    int iterations = 10;
    bool json_output = false;
    std::string output_path;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--json") {
            json_output = true;
        } else if (arg == "--iterations" && i + 1 < argc) {
            try {
                iterations = almo_test::parse_iterations(argv[++i]);
            } catch (const std::runtime_error& error) {
                std::cerr << error.what() << '\n';
                return 1;
            }
        } else if (arg == "--output" && i + 1 < argc) {
            output_path = argv[++i];
        } else {
            std::cerr << "Unknown argument: " << arg << '\n';
            return 1;
        }
    }

    const auto scenarios = almo_test::make_benchmark_scenarios();
    std::vector<almo_test::BenchmarkResult> results;
    results.reserve(scenarios.size());

    for (const auto& scenario : scenarios) {
        results.push_back(almo_test::run_benchmark(scenario, iterations));
    }

    const std::string serialized = almo_test::to_json(results);

    if (!output_path.empty()) {
        std::ofstream ofs(output_path);
        if (!ofs) {
            std::cerr << "Failed to open benchmark output file: "
                      << output_path << '\n';
            return 1;
        }
        ofs << serialized;
    }

    if (json_output) {
        std::cout << serialized;
    } else {
        almo_test::print_human_readable(results);
    }

    return 0;
}
