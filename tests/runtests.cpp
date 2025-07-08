#include <gtest/gtest.h>
#include <fstream>
#include <sstream>
#include "src/parse.hpp"
#include "src/ast.hpp"
#include "src/utils.hpp"
#include "src/reader.hpp"

class AlmoTestSuite : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary test files
        std::ofstream test_file("test_input.md");
        test_file << "# Test Header\n\nThis is a test paragraph.\n\n```cpp\nint main() { return 0; }\n```";
        test_file.close();
    }
    
    void TearDown() override {
        // Clean up test files
        std::remove("test_input.md");
    }
};

// =====================================
// Reader Class Tests
// =====================================

TEST_F(AlmoTestSuite, ReaderBasicFunctionality) {
    std::vector<std::string> lines = {"Hello", "World", "Test"};
    almo::Reader reader(lines);
    
    EXPECT_EQ(reader.get_row(), "Hello");
    EXPECT_EQ(reader.row, 0);
    EXPECT_EQ(reader.col, 0);
    EXPECT_TRUE(reader.is_line_begin());
    
    reader.move_next_char(1);
    EXPECT_EQ(reader.col, 1);
    EXPECT_FALSE(reader.is_line_begin());
    
    reader.move_next_line();
    EXPECT_EQ(reader.row, 1);
    EXPECT_EQ(reader.get_row(), "World");
}

TEST_F(AlmoTestSuite, ReaderEOFHandling) {
    std::vector<std::string> lines = {"A"};
    almo::Reader reader(lines);
    
    EXPECT_FALSE(reader.is_eof());
    reader.move_next_line();
    EXPECT_TRUE(reader.is_eof());
}

// =====================================
// Utility Functions Tests
// =====================================

TEST_F(AlmoTestSuite, UtilityFunctions) {
    // Test string trimming
    EXPECT_EQ(almo::ltrim("  hello  "), "hello  ");
    EXPECT_EQ(almo::rtrim("  hello  "), "  hello");
    EXPECT_EQ(almo::trim("  hello  "), "hello");
    
    // Test string splitting
    auto parts = almo::split("a,b,c", ',');
    EXPECT_EQ(parts.size(), 3);
    EXPECT_EQ(parts[0], "a");
    EXPECT_EQ(parts[1], "b");
    EXPECT_EQ(parts[2], "c");
    
    // Test string joining
    std::vector<std::string> vec = {"a", "b", "c"};
    EXPECT_EQ(almo::join(vec, ","), "a,b,c");
    
    // Test HTML escaping
    EXPECT_EQ(almo::escape_for_html("<script>alert('xss')</script>"), "&lt;script&gt;alert('xss')&lt;/script&gt;");
    
    // Test JSON escaping
    EXPECT_EQ(almo::escape("Hello\"World"), "Hello\\\"World");
}

TEST_F(AlmoTestSuite, FileOperations) {
    // Test file reading
    std::string content = almo::read_file("test_input.md");
    EXPECT_FALSE(content.empty());
    EXPECT_TRUE(content.find("Test Header") != std::string::npos);
    
    // Test file not found error
    EXPECT_THROW(almo::read_file("nonexistent.md"), almo::NotFoundFileError);
}

// =====================================
// AST Node Tests
// =====================================

TEST_F(AlmoTestSuite, ASTNodeBasics) {
    almo::Markdown root;
    
    EXPECT_FALSE(root.get_uuid().empty());
    EXPECT_EQ(root.get_class_name(), "Markdown");
    EXPECT_EQ(root.get_children().size(), 0);
    
    // Test child management
    auto header = std::make_shared<almo::Header>(1, "Test");
    root.pushback_child(header);
    
    EXPECT_EQ(root.get_children().size(), 1);
    EXPECT_EQ(root.get_children()[0]->get_class_name(), "Header");
}

TEST_F(AlmoTestSuite, ASTNodeSerialization) {
    almo::Header header(1, "Test Header");
    
    // Test JSON serialization
    nlohmann::json json_output = header.to_json();
    EXPECT_EQ(json_output["class_name"], "Header");
    EXPECT_EQ(json_output["level"], 1);
    EXPECT_EQ(json_output["content"], "Test Header");
    
    // Test HTML generation
    std::string html_output = header.to_html();
    EXPECT_TRUE(html_output.find("<h1") != std::string::npos);
    EXPECT_TRUE(html_output.find("Test Header") != std::string::npos);
}

// =====================================
// Parser Tests
// =====================================

TEST_F(AlmoTestSuite, BasicMarkdownParsing) {
    std::string content = "# Header 1\n\nThis is a paragraph.\n\n## Header 2";
    almo::MarkdownParser parser({content});
    almo::Markdown ast;
    
    parser.process(ast);
    
    auto children = ast.get_children();
    EXPECT_GE(children.size(), 2); // At least header and paragraph
    
    // Check if first child is a header
    bool found_header = false;
    for (const auto& child : children) {
        if (child->get_class_name() == "Header") {
            found_header = true;
            break;
        }
    }
    EXPECT_TRUE(found_header);
}

TEST_F(AlmoTestSuite, InlineParserBasics) {
    std::string content = "*italic* and **bold** text";
    almo::InlineParser parser(content);
    almo::RawText ast;
    
    parser.process(ast);
    
    auto children = ast.get_children();
    EXPECT_GT(children.size(), 1); // Should have multiple inline elements
    
    // Check HTML output contains emphasis tags
    std::string html = ast.to_html();
    EXPECT_TRUE(html.find("<em>") != std::string::npos || html.find("<i>") != std::string::npos);
    EXPECT_TRUE(html.find("<strong>") != std::string::npos || html.find("<b>") != std::string::npos);
}

TEST_F(AlmoTestSuite, CodeBlockParsing) {
    std::string content = "```cpp\nint main() {\n    return 0;\n}\n```";
    almo::MarkdownParser parser({content});
    almo::Markdown ast;
    
    parser.process(ast);
    
    auto children = ast.get_children();
    bool found_codeblock = false;
    for (const auto& child : children) {
        if (child->get_class_name() == "CodeBlock") {
            found_codeblock = true;
            break;
        }
    }
    EXPECT_TRUE(found_codeblock);
}

// =====================================
// Error Handling Tests
// =====================================

TEST_F(AlmoTestSuite, ErrorHandling) {
    // Test ParseError
    try {
        throw almo::ParseError("Test parse error", 1, 5);
    } catch (const almo::ParseError& e) {
        EXPECT_EQ(e.get_line(), 1);
        EXPECT_EQ(e.get_pos(), 5);
        std::string msg = e.what();
        EXPECT_TRUE(msg.find("Test parse error") != std::string::npos);
    }
    
    // Test SyntaxError
    EXPECT_THROW({
        throw almo::SyntaxError("Test syntax error", 2, 10);
    }, almo::SyntaxError);
}

// =====================================
// Integration Tests
// =====================================

TEST_F(AlmoTestSuite, EndToEndParsing) {
    std::string complex_content = R"(
# Main Title

This is a paragraph with *italic* and **bold** text.

## Subsection

- List item 1
- List item 2
  - Nested item

```python
def hello():
    print("Hello, World!")
```

> This is a quote

| Column 1 | Column 2 |
|----------|----------|
| Data 1   | Data 2   |
)";
    
    almo::MarkdownParser parser({complex_content});
    almo::Markdown ast;
    
    EXPECT_NO_THROW(parser.process(ast));
    
    // Verify AST structure
    auto children = ast.get_children();
    EXPECT_GT(children.size(), 3);
    
    // Test JSON serialization
    nlohmann::json json_output;
    EXPECT_NO_THROW(json_output = ast.to_json());
    EXPECT_EQ(json_output["class_name"], "Markdown");
    EXPECT_GT(json_output["children"].size(), 0);
    
    // Test HTML generation
    std::string html_output;
    EXPECT_NO_THROW(html_output = ast.to_html());
    EXPECT_TRUE(html_output.find("<h1>") != std::string::npos);
    EXPECT_TRUE(html_output.find("<ul>") != std::string::npos);
    EXPECT_TRUE(html_output.find("<pre>") != std::string::npos);
    EXPECT_TRUE(html_output.find("<blockquote>") != std::string::npos);
    EXPECT_TRUE(html_output.find("<table>") != std::string::npos);
}

TEST_F(AlmoTestSuite, PerformanceBaseline) {
    std::string large_content;
    for (int i = 0; i < 1000; ++i) {
        large_content += "# Header " + std::to_string(i) + "\n\n";
        large_content += "This is paragraph " + std::to_string(i) + " with some *italic* text.\n\n";
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    almo::MarkdownParser parser({large_content});
    almo::Markdown ast;
    parser.process(ast);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should process 1000 elements in reasonable time (adjust threshold as needed)
    EXPECT_LT(duration.count(), 5000); // Less than 5 seconds
    
    // Verify structure
    EXPECT_GT(ast.get_children().size(), 1000);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
