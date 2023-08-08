#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace almo {

    enum Type {
        H1,
        H2,
        H3,
        H4,
        H5,
        H6,
        Block,
        CodeRunner,
        CodeBlock,
        MathBlock,
        InlineMath,
        InlineOverline,
        InlineStrong,
        InlineItalic,
        PlainText,
        InfoBlock,
        NewLine,
    };

    struct AST {
        using node_ptr = std::shared_ptr<AST>;
        AST() = default;
        AST(Type type, std::string content = "") : type(type), content(content) { }
        Type type;
        std::string content;
        std::vector<node_ptr> childs;

        std::vector<std::pair<std::string, std::string>> code_runner;
    };
}