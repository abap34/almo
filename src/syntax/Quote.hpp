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
    std::shared_ptr<ASTNode> clone() const override {
        return clone_as<Quote>();
    }

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
    std::optional<BlockMatch> match(const Reader &read) const override {
        if (!read.is_line_begin()) return block_match(false);
        return block_match(read.line().text.starts_with("> "));
    }
    void parse(Reader &read, ASTNode &ast, const BlockMatch&) const override {
        // BEGIN : '> .*'
        // END   : '(?!> ).*'

        std::vector<SourceLine> quote_contents;
        while (!read.is_eof()) {
            if (read.get_row().starts_with("> ")) {
                const auto line = read.line();
                quote_contents.push_back({line.text.substr(2), line.begin + 2, line.end});
                read.move_next_line();
                continue;
            }
            break;
        }

        // uuid order is node --> inner_md
        Quote node;
        Markdown inner_md;
        auto nested = read.nested(std::move(quote_contents));
        detail::parse_into(nested, inner_md);
        node.pushback_child(std::make_shared<Markdown>(inner_md));
        ast.pushback_child(std::make_shared<Quote>(node));
    }
};

}  // namespace almo
