#pragma once

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"
#include "../utils.hpp"

namespace almo {

struct HorizontalLine : public ASTNode {
   public:
    HorizontalLine() { set_uuid(); }

    std::string to_html() const override { return "<hr>"; }

    std::map<std::string, std::string> get_properties() const override {
        return {};
    }
    std::string get_classname() const override { return "HorizontalLine"; }
};

struct HorizontalLineSyntax : BlockSyntax {
    bool operator()(Reader &read) const override {
        if (!read.is_line_begin()) return false;
        if (rtrim(read.get_row()) == "---") return true;
        if (rtrim(read.get_row()) == "___") return true;
        if (rtrim(read.get_row()) == "***") return true;
        return false;
    }
    void operator()(Reader &read, ASTNode &ast) const override {
        HorizontalLine node;
        ast.pushback_child(std::make_shared<HorizontalLine>(node));
        read.move_next_line();
    }
};

}  // namespace almo