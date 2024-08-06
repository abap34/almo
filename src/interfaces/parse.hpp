#pragma once

#include <string>
#include "../reader.hpp"
#include "ast.hpp"

namespace almo {

// parse a line contain only inline syntax
struct InlineParser {
    template<class Syntax>
    static void process_inline(const std::string &str, ASTNode &ast, Syntax&& syn, int pos);

    template<class Syntax, class HeadSyntax, class... TailSyntax>
    static void process_inline(const std::string &str, ASTNode &ast, Syntax&& syn, int pos, HeadSyntax&& hsyn, TailSyntax&&... tsyn);

    static void process(const std::string &str, ASTNode &ast);
};

// parse an entire markdown and detect block syntax
struct MarkdownParser {
    Reader reader;
    MarkdownParser(const std::vector<std::string> &lines);

    void process_block(std::string &str_inline, ASTNode &ast);

    template<class HeadSyntax, class... TailSyntax>
    void process_block(std::string &str_inline, ASTNode &ast, HeadSyntax&& hsyn, TailSyntax&&... tsyn);

    void process(ASTNode &ast);
};

} // namespace almo