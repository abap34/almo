#pragma once

#include"../interfaces/ast.hpp"
#include"../interfaces/parser.hpp"
#include"../interfaces/syntax.hpp"

namespace almo {

struct Header : public ASTNode {

    int level;
    Header (int _level) : level(_level) {
        set_uuid();
    }
    std::string to_html() const override {
        std::string childs_html = concatenated_childs_html();
        std::string contents_push = ""
            "<script>"
            "page_contents.push({\n"
            "    \"type\":\"H" + std::to_string(level) + "\",\n"
            "    \"id\":\"" + get_uuid_str() + "\",\n"
            "    \"title\":\"" + childs_html + "\"\n"
            "});\n"
            "</script>\n";

        return contents_push + "<h" + std::to_string(level) + " id=\"" + get_uuid_str() + "\">" + childs_html + "</h" + std::to_string(level) + ">";
    }

    std::map<std::string, std::string> get_properties() const override {
        return {
            {"level", std::to_string(level)}
        };
    }
    std::string get_classname() const override {
        return "Header";
    }
};

struct Header_syntax : BlockSyntax {
    bool operator()(Reader &read) const override {
        if (!read.is_line_begin()) return false;
        if (read.get_row().starts_with("# ")) return true;
        // todo #2~6
        return false;
    }
    void operator()(Reader &read, ASTNode &ast) const override {
        if (read.get_row().starts_with("# ")){
            Header node(1);
            InlineParser::process(read.get_row().substr(2), node);
            ast.add_child(std::make_shared<Header>(node));
            read.move_next_line();
        }
        // todo #2~6
    }
};

} // namespace almo