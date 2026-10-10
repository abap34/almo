#pragma once

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"
#include "../utils.hpp"

namespace almo {

struct NewLine : public ASTNode {
   public:
    std::shared_ptr<ASTNode> clone() const override {
        return clone_as<NewLine>();
    }

    NewLine() { set_uuid(); }

    std::string to_html() const override { return "<br>"; }

    std::map<std::string, std::string> get_properties() const override {
        return {};
    }
    std::string get_classname() const override { return "NewLine"; }
};

struct NewLineSyntax : public BlockSyntax {
    std::optional<BlockMatch> match(const Reader &read) const override {
        if (!read.is_line_begin()) return block_match(false);
        if (detail::rtrim_view(read.line().text).empty()) return block_match(true);
        return block_match(false);
    }
    void parse(Reader &read, ASTNode &ast, const BlockMatch&) const override {
        NewLine node;
        ast.pushback_child(std::make_shared<NewLine>(node));
        read.move_next_line();
    }
};

}  // namespace almo
