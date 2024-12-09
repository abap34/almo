#include <benchmark/benchmark.h>
#include "src/parse.hpp"

static void BM_Sum(benchmark::State& state) {
    std::string content = " *hello* ";
    for (auto _ : state) {
        almo::MarkdownParser parser({content});
        almo::Markdown ast;
        parser.process(ast);
    }
    
}

BENCHMARK(BM_Sum);

BENCHMARK_MAIN();