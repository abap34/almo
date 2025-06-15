#include <benchmark/benchmark.h>
#include <random>
#include <sstream>
#include "src/parse.hpp"
#include "src/ast.hpp"
#include "src/utils.hpp"

// =====================================
// Helper Functions for Benchmark Data
// =====================================

std::string generate_markdown_content(size_t size) {
    std::ostringstream oss;
    std::mt19937 gen(42); // Fixed seed for reproducible benchmarks
    std::uniform_int_distribution<> dist(1, 6);
    
    for (size_t i = 0; i < size; ++i) {
        int type = dist(gen);
        switch (type) {
            case 1:
                oss << "# Header " << i << "\n\n";
                break;
            case 2:
                oss << "This is paragraph " << i << " with some *italic* and **bold** text.\n\n";
                break;
            case 3:
                oss << "- List item " << i << "\n";
                break;
            case 4:
                oss << "> Quote " << i << "\n\n";
                break;
            case 5:
                oss << "```cpp\nint func" << i << "() { return " << i << "; }\n```\n\n";
                break;
            case 6:
                oss << "[Link " << i << "](https://example.com/" << i << ")\n\n";
                break;
        }
    }
    return oss.str();
}

std::string generate_inline_content(size_t complexity) {
    std::ostringstream oss;
    for (size_t i = 0; i < complexity; ++i) {
        oss << "Normal text " << i << " with *italic* and **bold** ";
        oss << "and `inline code` and [link](http://example.com) ";
        if (i % 5 == 0) oss << "~~strikethrough~~ ";
        if (i % 7 == 0) oss << "***bold italic*** ";
    }
    return oss.str();
}

// =====================================
// Core Parsing Benchmarks
// =====================================

static void BM_BasicParsing(benchmark::State& state) {
    std::string content = " *hello* **world** with `code` ";
    for (auto _ : state) {
        almo::MarkdownParser parser({content});
        almo::Markdown ast;
        parser.process(ast);
        benchmark::DoNotOptimize(ast);
    }
}
BENCHMARK(BM_BasicParsing);

static void BM_HeaderParsing(benchmark::State& state) {
    std::string content = "# H1\n## H2\n### H3\n#### H4\n##### H5\n###### H6";
    for (auto _ : state) {
        almo::MarkdownParser parser({content});
        almo::Markdown ast;
        parser.process(ast);
        benchmark::DoNotOptimize(ast);
    }
}
BENCHMARK(BM_HeaderParsing);

static void BM_ListParsing(benchmark::State& state) {
    std::string content = "- Item 1\n- Item 2\n  - Nested 1\n  - Nested 2\n- Item 3";
    for (auto _ : state) {
        almo::MarkdownParser parser({content});
        almo::Markdown ast;
        parser.process(ast);
        benchmark::DoNotOptimize(ast);
    }
}
BENCHMARK(BM_ListParsing);

static void BM_CodeBlockParsing(benchmark::State& state) {
    std::string content = "```cpp\nint main() {\n    std::cout << \"Hello\" << std::endl;\n    return 0;\n}\n```";
    for (auto _ : state) {
        almo::MarkdownParser parser({content});
        almo::Markdown ast;
        parser.process(ast);
        benchmark::DoNotOptimize(ast);
    }
}
BENCHMARK(BM_CodeBlockParsing);

// =====================================
// Inline Parsing Benchmarks
// =====================================

static void BM_InlineParsingSimple(benchmark::State& state) {
    std::string content = "*italic* **bold** `code`";
    for (auto _ : state) {
        almo::InlineParser parser(content);
        almo::RawText ast;
        parser.process(ast);
        benchmark::DoNotOptimize(ast);
    }
}
BENCHMARK(BM_InlineParsingSimple);

static void BM_InlineParsingComplex(benchmark::State& state) {
    std::string content = generate_inline_content(state.range(0));
    for (auto _ : state) {
        almo::InlineParser parser(content);
        almo::RawText ast;
        parser.process(ast);
        benchmark::DoNotOptimize(ast);
    }
}
BENCHMARK(BM_InlineParsingComplex)->Range(1, 100);

// =====================================
// Scaling Benchmarks
// =====================================

static void BM_DocumentSize(benchmark::State& state) {
    std::string content = generate_markdown_content(state.range(0));
    for (auto _ : state) {
        almo::MarkdownParser parser({content});
        almo::Markdown ast;
        parser.process(ast);
        benchmark::DoNotOptimize(ast);
    }
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_DocumentSize)->Range(10, 1000)->Complexity();

// =====================================
// Output Generation Benchmarks
// =====================================

static void BM_JSONSerialization(benchmark::State& state) {
    std::string content = generate_markdown_content(50);
    almo::MarkdownParser parser({content});
    almo::Markdown ast;
    parser.process(ast);
    
    for (auto _ : state) {
        auto json_output = ast.to_json();
        benchmark::DoNotOptimize(json_output);
    }
}
BENCHMARK(BM_JSONSerialization);

static void BM_HTMLGeneration(benchmark::State& state) {
    std::string content = generate_markdown_content(50);
    almo::MarkdownParser parser({content});
    almo::Markdown ast;
    parser.process(ast);
    
    for (auto _ : state) {
        std::string html_output = ast.to_html();
        benchmark::DoNotOptimize(html_output);
    }
}
BENCHMARK(BM_HTMLGeneration);

static void BM_DOTGeneration(benchmark::State& state) {
    std::string content = generate_markdown_content(20); // Smaller for DOT
    almo::MarkdownParser parser({content});
    almo::Markdown ast;
    parser.process(ast);
    
    for (auto _ : state) {
        std::string dot_output = ast.to_dot();
        benchmark::DoNotOptimize(dot_output);
    }
}
BENCHMARK(BM_DOTGeneration);

// =====================================
// Utility Function Benchmarks
// =====================================

static void BM_StringTrimming(benchmark::State& state) {
    std::string test_string = "   lots of whitespace around this text   ";
    for (auto _ : state) {
        std::string result = almo::trim(test_string);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_StringTrimming);

static void BM_StringSplitting(benchmark::State& state) {
    std::string test_string = "a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z";
    for (auto _ : state) {
        auto result = almo::split(test_string, ',');
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_StringSplitting);

static void BM_HTMLEscaping(benchmark::State& state) {
    std::string test_string = "<script>alert('This is a test & example');</script>";
    for (auto _ : state) {
        std::string result = almo::escape_for_html(test_string);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_HTMLEscaping);

static void BM_JSONEscaping(benchmark::State& state) {
    std::string test_string = "This is a \"test\" string with \n newlines \t and tabs";
    for (auto _ : state) {
        std::string result = almo::escape(test_string);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_JSONEscaping);

// =====================================
// AST Operations Benchmarks
// =====================================

static void BM_ASTNodeCreation(benchmark::State& state) {
    for (auto _ : state) {
        auto header = std::make_shared<almo::Header>(1, "Test Header");
        benchmark::DoNotOptimize(header);
    }
}
BENCHMARK(BM_ASTNodeCreation);

static void BM_ASTChildManagement(benchmark::State& state) {
    almo::Markdown root;
    for (auto _ : state) {
        auto header = std::make_shared<almo::Header>(1, "Test");
        root.pushback_child(header);
        root.remove_child(header);
        benchmark::DoNotOptimize(root);
    }
}
BENCHMARK(BM_ASTChildManagement);

// =====================================
// Memory and Performance Stress Tests
// =====================================

static void BM_DeepNesting(benchmark::State& state) {
    std::ostringstream oss;
    int depth = state.range(0);
    
    // Create deeply nested list structure
    for (int i = 0; i < depth; ++i) {
        for (int j = 0; j < i; ++j) {
            oss << "  ";
        }
        oss << "- Item at depth " << i << "\n";
    }
    
    std::string content = oss.str();
    for (auto _ : state) {
        almo::MarkdownParser parser({content});
        almo::Markdown ast;
        parser.process(ast);
        benchmark::DoNotOptimize(ast);
    }
}
BENCHMARK(BM_DeepNesting)->Range(1, 50);

static void BM_WideStructure(benchmark::State& state) {
    std::ostringstream oss;
    int width = state.range(0);
    
    // Create structure with many siblings
    for (int i = 0; i < width; ++i) {
        oss << "# Header " << i << "\n\n";
        oss << "Paragraph " << i << "\n\n";
    }
    
    std::string content = oss.str();
    for (auto _ : state) {
        almo::MarkdownParser parser({content});
        almo::Markdown ast;
        parser.process(ast);
        benchmark::DoNotOptimize(ast);
    }
}
BENCHMARK(BM_WideStructure)->Range(10, 500);

// =====================================
// Regression Tests (Performance)
// =====================================

static void BM_RealWorldDocument(benchmark::State& state) {
    // Simulate a real-world document structure
    std::ostringstream oss;
    oss << "# Main Title\n\n";
    oss << "This is an introduction paragraph with *emphasis* and **strong** text.\n\n";
    oss << "## Section 1: Code Examples\n\n";
    oss << "Here's some code:\n\n";
    oss << "```cpp\n#include <iostream>\nint main() {\n    std::cout << \"Hello!\" << std::endl;\n    return 0;\n}\n```\n\n";
    oss << "### Subsection 1.1\n\n";
    oss << "- Feature A\n- Feature B\n  - Sub-feature B.1\n  - Sub-feature B.2\n- Feature C\n\n";
    oss << "## Section 2: Tables and Links\n\n";
    oss << "| Column 1 | Column 2 | Column 3 |\n";
    oss << "|----------|----------|----------|\n";
    oss << "| Data 1   | Data 2   | Data 3   |\n";
    oss << "| More 1   | More 2   | More 3   |\n\n";
    oss << "Check out [our website](https://example.com) for more info.\n\n";
    oss << "> This is a blockquote with some important information.\n\n";
    
    std::string content = oss.str();
    for (auto _ : state) {
        almo::MarkdownParser parser({content});
        almo::Markdown ast;
        parser.process(ast);
        std::string html = ast.to_html();
        nlohmann::json json = ast.to_json();
        benchmark::DoNotOptimize(html);
        benchmark::DoNotOptimize(json);
    }
}
BENCHMARK(BM_RealWorldDocument);

BENCHMARK_MAIN();