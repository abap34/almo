#pragma once

#include "../new_ast.hpp"
#include "../new_reader.hpp"
#include "../new_syntax.hpp"

namespace almo::feature {

struct NewLine : public ASTNode {

    NewLine () {
        set_uuid();
    }

    std::string to_html() const override {
        return "<br>";
    }

    std::map<std::string, std::string> get_properties() const override {
        return {
        };
    }
    std::string get_classname() const override {
        return "NewLine";
    }
};

struct NewLine_syntax : public BlockSyntax {
    bool operator()(Reader &read) const override {
        return !read.is_line_begin() && read.whole_row() == "";
    }
    void operator()(Reader &read, ASTNode &ast) const override {
        NewLine node;
        ast.add_child(std::make_shared<NewLine>(node));
        read.move_next_line();
    }
};

} // namespace almo