#pragma once

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"
#include "../utils.hpp"
#include "Markdown.hpp"

namespace almo {

// 引用を表すクラス
struct Quote : public ASTNode {
   public:
    Quote() { set_uuid(); }

    std::string to_html() const override {
        return "<blockquote>" + concatenated_childs_html() + "</blockquote>";
    }

    std::map<std::string, std::string> get_properties() const override {
        return {};
    }
    std::string get_classname() const override { return "Quote"; }
};

struct QuoteSyntax : public BlockSyntax {
    bool operator()(Reader &read) const override {
        if (!read.is_line_begin()) return false;
        return read.get_row().starts_with("> ");
    }
    void operator()(Reader &read, ASTNode &ast) const override {
        // BEGIN : '> .*'
        // END   : '(?!> ).*'

        std::vector<std::string> quote_contents;
        while (!read.is_eof()) {
            if (read.get_row().starts_with("> ")) {
                quote_contents.emplace_back(read.get_row().substr(2));
                read.move_next_line();
                continue;
            }
            break;
        }

        // uuid order is node --> inner_md
        Quote node;
        Markdown inner_md;
        MarkdownParser parser(quote_contents);
        parser.process(inner_md);
        node.pushback_child(std::make_shared<Markdown>(inner_md));
        ast.pushback_child(std::make_shared<Quote>(node));
    }
};

}  // namespace almo