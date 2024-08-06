#pragma once

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"

namespace almo {

struct Header : public ASTNode {
   private:
    int level;

   public:
    Header(int _level) : level(_level) { set_uuid(); }
    std::string to_html() const override {
        std::string childs_html = concatenated_childs_html();
        std::string contents_push =
            ""
            "<script>"
            "page_contents.push({\n"
            "    \"type\":\"H" +
            std::to_string(level) +
            "\",\n"
            "    \"id\":\"" +
            get_uuid_str() +
            "\",\n"
            "    \"title\":\"" +
            childs_html +
            "\"\n"
            "});\n"
            "</script>\n";

        return contents_push + "<h" + std::to_string(level) + " id=\"" +
               get_uuid_str() + "\">" + childs_html + "</h" +
               std::to_string(level) + ">";
    }

    std::map<std::string, std::string> get_properties() const override {
        return {{"level", std::to_string(level)}};
    }
    std::string get_classname() const override { return "Header"; }
};

struct HeaderSyntax : BlockSyntax {
    bool operator()(Reader &read) const override {
        if (!read.is_line_begin()) return false;
        if (read.get_row().starts_with("# ")) return true;
        if (read.get_row().starts_with("## ")) return true;
        if (read.get_row().starts_with("### ")) return true;
        if (read.get_row().starts_with("#### ")) return true;
        if (read.get_row().starts_with("##### ")) return true;
        if (read.get_row().starts_with("###### ")) return true;
        return false;
    }
    void operator()(Reader &read, ASTNode &ast) const override {
        int level = 0;
        if (read.get_row().starts_with("# ")) level = 1;
        if (read.get_row().starts_with("## ")) level = 2;
        if (read.get_row().starts_with("### ")) level = 3;
        if (read.get_row().starts_with("#### ")) level = 4;
        if (read.get_row().starts_with("##### ")) level = 5;
        if (read.get_row().starts_with("###### ")) level = 6;
        assert(1 <= level && level <= 6);
        Header node(level);
        InlineParser::process(read.get_row().substr(level + 1), node);
        ast.pushback_child(std::make_shared<Header>(node));
        read.move_next_line();
    }
};

}  // namespace almo