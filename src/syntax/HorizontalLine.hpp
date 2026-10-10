#pragma once

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"
#include "../utils.hpp"

namespace almo {

struct HorizontalLine : public ASTNode {
   public:
    std::shared_ptr<ASTNode> clone() const override {
        return clone_as<HorizontalLine>();
    }

    HorizontalLine() { set_uuid(); }

    std::string to_html() const override { return "<hr>"; }

    std::map<std::string, std::string> get_properties() const override {
        return {};
    }
    std::string get_classname() const override { return "HorizontalLine"; }
};

struct HorizontalLineSyntax : BlockSyntax {
    std::optional<BlockMatch> match(const Reader &read) const override {
        if (!read.is_line_begin()) return block_match(false);
        if (rtrim(read.get_row()) == "---") return block_match(true);
        if (rtrim(read.get_row()) == "___") return block_match(true);
        if (rtrim(read.get_row()) == "***") return block_match(true);
        return block_match(false);
    }
    void parse(Reader &read, ASTNode &ast, const BlockMatch&) const override {
        HorizontalLine node;
        ast.pushback_child(std::make_shared<HorizontalLine>(node));
        read.move_next_line();
    }
};

}  // namespace almo