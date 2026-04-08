#include <algorithm>
#include <chrono>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../build/ast.hpp"
#include "../build/parse.hpp"
#include "../build/render.hpp"

namespace {

struct BenchmarkResult {
    std::string name;
    int iterations;
    std::size_t work_units;
    double min_ms;
    double mean_ms;
    double max_ms;
};

std::map<std::string, std::string> default_meta() {
    return {
        {"template_file", "__default__"},
        {"theme", "light"},
        {"css_setting", "light"},
        {"editor_theme", "ace/theme/chrome"},
        {"syntax_theme", "github.min"},
        {"title", "ALMO Benchmark"},
        {"description", "Benchmark document"},
        {"author", "benchmark"},
        {"twitter_id", ""},
        {"github_id", ""},
        {"mail", ""},
        {"ogp_url", "https://example.com/ogp.png"},
        {"tag", "benchmark"},
        {"url", "https://example.com"},
        {"site_name", "ALMO"},
        {"twitter_site", ""},
    };
}

std::vector<std::string> markdown_lines(const std::string& text) {
    return split(text, "\n");
}

std::vector<std::string> build_document(std::size_t sections) {
    std::ostringstream oss;

    for (std::size_t i = 0; i < sections; ++i) {
        oss << "# Section " << i << "\n\n";
        oss << "Paragraph " << i
            << " with **bold** text, `inline code`, and a "
            << "[link](https://example.com/" << i << ").\n\n";
        oss << "- item " << i << '\n';
        oss << "- item " << i + 1 << "\n\n";
        oss << "```cpp\n";
        oss << "int value_" << i << "() { return " << i << "; }\n";
        oss << "```\n\n";
    }

    return markdown_lines(oss.str());
}

BenchmarkResult run_benchmark(const std::string& name,
                              const std::function<void()>& body,
                              int iterations, std::size_t work_units) {
    std::vector<double> samples;
    samples.reserve(iterations);

    body();

    for (int i = 0; i < iterations; ++i) {
        const auto start = std::chrono::steady_clock::now();
        body();
        const auto end = std::chrono::steady_clock::now();
        const std::chrono::duration<double, std::milli> elapsed = end - start;
        samples.push_back(elapsed.count());
    }

    const auto [min_it, max_it] =
        std::minmax_element(samples.begin(), samples.end());
    const double total =
        std::accumulate(samples.begin(), samples.end(), 0.0);

    return {
        name,
        iterations,
        work_units,
        *min_it,
        total / static_cast<double>(samples.size()),
        *max_it,
    };
}

std::string to_json(const std::vector<BenchmarkResult>& results) {
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

void print_human_readable(const std::vector<BenchmarkResult>& results) {
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Benchmark results (ms)\n";

    for (const auto& result : results) {
        std::cout << "- " << result.name << ": mean=" << result.mean_ms
                  << ", min=" << result.min_ms << ", max=" << result.max_ms
                  << ", iterations=" << result.iterations
                  << ", work_units=" << result.work_units << '\n';
    }
}

int parse_iterations(const std::string& value) {
    try {
        const int iterations = std::stoi(value);
        if (iterations < 1) {
            throw std::out_of_range("iterations must be positive");
        }
        return iterations;
    } catch (const std::exception&) {
        throw std::runtime_error(
            "Invalid value for --iterations: expected an integer >= 1"
        );
    }
}

}  // namespace

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
                iterations = parse_iterations(argv[++i]);
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

    const auto small_document = build_document(10);
    const auto large_document = build_document(200);

    std::vector<BenchmarkResult> results;
    results.push_back(run_benchmark(
        "parse_small_document",
        [&]() {
            almo::Markdown ast;
            almo::MarkdownParser parser(small_document);
            parser.process(ast);
        },
        iterations, small_document.size()));

    results.push_back(run_benchmark(
        "parse_large_document",
        [&]() {
            almo::Markdown ast;
            almo::MarkdownParser parser(large_document);
            parser.process(ast);
        },
        iterations, large_document.size()));

    results.push_back(run_benchmark(
        "render_large_document",
        [&]() {
            auto meta = default_meta();
            auto summary = almo::md_to_summary(large_document, meta);
            if (summary.html.empty()) {
                throw std::runtime_error("rendered HTML must not be empty");
            }
        },
        iterations, large_document.size()));

    const std::string serialized = to_json(results);

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
        print_human_readable(results);
    }

    return 0;
}
