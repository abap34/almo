#include <benchmark/benchmark.h>
#include <random>
#include <sstream>

// =====================================
// Simple Benchmark Tests
// =====================================

static void BM_StringCreation(benchmark::State& state) {
    for (auto _ : state) {
        std::string s = "Hello World";
        benchmark::DoNotOptimize(s);
    }
}
BENCHMARK(BM_StringCreation);

static void BM_StringConcatenation(benchmark::State& state) {
    std::string base = "Hello";
    for (auto _ : state) {
        std::string result = base + " World";
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_StringConcatenation);

static void BM_VectorPushBack(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<int> v;
        for (int i = 0; i < state.range(0); ++i) {
            v.push_back(i);
        }
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(BM_VectorPushBack)->Range(8, 8<<10);

static void BM_StringSplit(benchmark::State& state) {
    std::string test_string = "a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z";
    for (auto _ : state) {
        std::vector<std::string> result;
        std::stringstream ss(test_string);
        std::string item;
        while (std::getline(ss, item, ',')) {
            result.push_back(item);
        }
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_StringSplit);

BENCHMARK_MAIN();