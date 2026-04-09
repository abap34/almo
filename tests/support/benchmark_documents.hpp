#pragma once

#include <cstddef>
#include <sstream>
#include <string>
#include <vector>

#include "almo_test_support.hpp"

namespace almo_test {

inline std::vector<std::string> build_basic_document(std::size_t sections) {
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

inline std::vector<std::string> build_inline_heavy_document(
    std::size_t paragraphs) {
    std::ostringstream oss;

    for (std::size_t i = 0; i < paragraphs; ++i) {
        oss << "Paragraph " << i << " with **bold " << i << "**, *italic " << i
            << "*, ~~drop " << i << "~~, `code_" << i << "`, $x_" << i
            << "^2$, [link](https://example.com/" << i
            << "), ![plot](https://example.com/img/" << i
            << ".png), and footnote[^" << i << "].\n\n";
        oss << "[^" << i << "]: footnote " << i << "\n\n";
    }

    return markdown_lines(oss.str());
}

inline std::vector<std::string> build_structured_document(std::size_t sections) {
    std::ostringstream oss;

    for (std::size_t i = 0; i < sections; ++i) {
        oss << ":::callout-" << i << "\n";
        oss << "Lead paragraph " << i << " with **highlight**.\n";
        oss << ":::inner-" << i << "\n";
        oss << "> quoted line " << i << "\n";
        oss << "> second line " << i << "\n";
        oss << ":::\n";
        oss << ":::\n\n";
        oss << "| Name | Score | Ref |\n";
        oss << "| :--- | :-: | ---: |\n";
        oss << "| **Item " << i << "** | $x_" << i
            << "$ | [ref](https://example.com/" << i << ") |\n\n";
        oss << "- bullet " << i << "\n";
        oss << "  - nested bullet " << i << "\n\n";
        oss << "1. step " << i << "\n";
        oss << "   1. nested step " << i << "\n\n";
        oss << "$$\n";
        oss << "x_" << i << " + y_" << i << " = z_" << i << "\n";
        oss << "$$\n\n";
    }

    return markdown_lines(oss.str());
}

inline std::vector<std::string> build_interactive_document(std::size_t blocks) {
    std::ostringstream oss;

    oss << ":::loadlib\n";
    oss << "numpy\n";
    oss << "matplotlib\n";
    oss << ":::\n\n";

    for (std::size_t i = 0; i < blocks; ++i) {
        oss << "## Playground " << i << "\n\n";
        oss << ":::code\n";
        oss << "print('block " << i << "')\n";
        oss << "for j in range(3):\n";
        oss << "    print(j)\n";
        oss << ":::\n\n";
    }

    return markdown_lines(oss.str());
}

inline std::vector<std::string> build_judge_document(std::size_t blocks) {
    std::ostringstream oss;

    for (std::size_t i = 0; i < blocks; ++i) {
        oss << ":::judge\n";
        oss << "title=Benchmark Judge " << i << '\n';
        oss << "sample_in=" << fixtures::kJudgeSampleInputPath << '\n';
        oss << "sample_out=" << fixtures::kJudgeSampleOutputPath << '\n';
        oss << "in=" << fixtures::kJudgeInputGlob << '\n';
        oss << "out=" << fixtures::kJudgeOutputGlob << '\n';
        oss << "source=" << fixtures::kExampleInputPath << '\n';
        oss << ":::\n\n";
    }

    return markdown_lines(oss.str());
}

}  // namespace almo_test
