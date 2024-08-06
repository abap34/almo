#pragma once

#include <functional>
#include "interfaces/parse.hpp"
#include "ast.hpp"
#include "syntax_all.hpp"

namespace almo {

template<class Syntax>
void InlineParser::process_inline(const std::string &str, ASTNode &ast, Syntax&& syn, int pos){
    std::invoke(syn, str, ast);
}

// syn has matched at position pos
template<class Syntax, class HeadSyntax, class... TailSyntax>
void InlineParser::process_inline(const std::string &str, ASTNode &ast, Syntax&& syn, int pos, HeadSyntax&& hsyn, TailSyntax&&... tsyn){
    int newpos = std::invoke(hsyn, str);
    if (newpos < pos){
        // syntax with nearer match detected
        return process_inline(str, ast, hsyn, newpos, tsyn...);
    }
    else {
        return process_inline(str, ast, syn, pos, tsyn...);
    }
}

void InlineParser::process(const std::string &str, ASTNode &ast){
    if (str == "") return ;
    process_inline(
        str, ast,
        RawTextSyntax{}, std::numeric_limits<int>::max(),
        InlineMathSyntax{},
        InlineOverlineSyntax{},
        InlineStrongSyntax{},
        InlineItalicSyntax{},
        InlineCodeBlockSyntax{},
        InlineImageSyntax{},
        InlineUrlSyntax{},
        InlineFootnoteReferenceSyntax{}
    );
}

MarkdownParser::MarkdownParser(const std::vector<std::string> &lines) : reader(lines) {}

void MarkdownParser::process_block(std::string &str_inline, ASTNode &ast){
    if (str_inline != ""){
        str_inline += ' ';
    }
    str_inline += reader.get_row();
    reader.move_next_line();
}

template<class HeadSyntax, class... TailSyntax>
void MarkdownParser::process_block(std::string &str_inline, ASTNode &ast, HeadSyntax&& hsyn, TailSyntax&&... tsyn){
    if (std::invoke(hsyn, reader)){
        if (str_inline != ""){
            InlineParser::process(str_inline, ast);
            str_inline = "";
        }
        std::invoke(hsyn, reader, ast);
        return ;
    }
    return process_block(str_inline, ast, tsyn...);
}

void MarkdownParser::process(ASTNode &ast){
    std::string str_inline = "";
    while (!reader.eof_read_flg){
        process_block(
            str_inline, ast,
            EOFSyntax{},
            NewLineSyntax{},
            JudgeSyntax{},
            ExecutableCodeBlockSyntax{},
            LoadLibSyntax{},
            DivBlockSyntax{},
            QuoteSyntax{},
            ListBlockSyntax{},
            EnumerateBlockSyntax{},
            HeaderSyntax{},
            CodeBlockSyntax{},
            MathBlockSyntax{},
            HorizontalLineSyntax{},
            TableSyntax{},
            FootnoteDefinitionSyntax{}
        );
    }
    if (str_inline != ""){
        std::cerr << "Not empty inline string : '" << str_inline << "'" << std::endl;
        throw ParseError("Internal Error");
    }
}

} // namespace almo