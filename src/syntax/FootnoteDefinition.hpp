#pragma once

#include"../interfaces/ast.hpp"
#include"../interfaces/parse.hpp"
#include"../interfaces/syntax.hpp"
#include"../utils.hpp"

namespace almo{
struct FootnoteDefinition : public ASTNode {
  private:
    std::string tag;
  public:
    FootnoteDefinition (std::string tag_) : tag(tag_){
        set_uuid();
    }
    std::string to_html() const override {
        std::string childs_html = concatenated_childs_html();
        return "<span class=\"footnote_def\"> <li id=\"note_" + tag +"\"><a href=\"#ref_" + tag + "\">^" + tag + "</a>" + childs_html +"</li> </span>";
    }
    std::map<std::string, std::string> get_properties() const override {
        return {
            {"tag", tag}
        };
    }
    std::string get_classname() const override {
        return "FootnoteDefinition";
    }
};

struct FootnoteDefinitionSyntax : public BlockSyntax {
    /*
        "[^" + expr + "]: + hoge"
    */
    static inline const std::regex rex = std::regex(R"(\[\^(.*?)\]:(.*?))");
    bool operator()(Reader &read) const override {
        if (!read.is_line_begin()) return false;
        std::string row = read.get_row();
        std::smatch sm;
        return std::regex_match(row, sm, rex);
    }
    void operator()(Reader &read, ASTNode &ast) const override {
        std::string row = read.get_row();
        std::smatch sm;
        assert(std::regex_match(row, sm, rex));
        FootnoteDefinition node(sm.format("$1"));
        InlineParser::process(sm.format("$2"), node);
        ast.add_child(std::make_shared<FootnoteDefinition>(node));
        read.move_next_line();
    }
};
}