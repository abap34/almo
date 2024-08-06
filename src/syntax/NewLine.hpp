#pragma once

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"
#include "../utils.hpp"

namespace almo {

struct NewLine : public ASTNode {
   public:
    NewLine() { set_uuid(); }

    std::string to_html() const override { return "<br>"; }

    std::map<std::string, std::string> get_properties() const override {
        return {};
    }
    std::string get_classname() const override { return "NewLine"; }
};

struct NewLineSyntax : public BlockSyntax {
    bool operator()(Reader &read) const override {
        if (!read.is_line_begin()) return false;
        if (rtrim(read.get_row()) == "") return true;
        return false;
    }
    void operator()(Reader &read, ASTNode &ast) const override {
        NewLine node;
        ast.pushback_child(std::make_shared<NewLine>(node));
        read.move_next_line();
    }
};

}  // namespace almo