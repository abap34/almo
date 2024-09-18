#pragma once

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"
#include "../utils.hpp"

namespace almo {
struct FootnoteDefinition : public ASTNode {
   private:
    std::string symbol;

   public:
    FootnoteDefinition(std::string symbol_) : symbol(symbol_) { set_uuid(); }
    std::string to_html() const override {
        std::string childs_html = concatenated_childs_html();
        std::string label = "label_" + symbol;
        std::string jump_to = "ref_" + symbol;

        return "<span class=\"footnote-def\" id=\"" + label + "\"><a href=\"#" +
               jump_to + "\">[" + symbol + "]</a>" + childs_html + "</span>";
    }
    std::map<std::string, std::string> get_properties() const override {
        return {{"symbol", symbol}};
    }
    std::string get_classname() const override { return "FootnoteDefinition"; }
};

struct FootnoteDefinitionSyntax : public BlockSyntax {
    /*
        "[^" + symbol + "]: + hoge"
    */
    static inline const std::regex rex = std::regex(R"(\[\^(.*)\]:(.*))");
    bool operator()(Reader &read) const override {
        if (!read.is_line_begin()) return false;
        std::string row = read.get_row();
        std::smatch sm;
        return std::regex_match(row, sm, rex);
    }
    void operator()(Reader &read, ASTNode &ast) const override {
        std::string row = read.get_row();
        std::smatch sm;
        std::regex_search(row, sm, rex);
        std::string symbol = sm.format("$1");
        std::string suffix = sm.format("$2");
        if (symbol.empty()) {
            std::cerr << "Warning: Footnote symbol is empty. This may cause "
                         "unexpected behavior. \n... \n"
                      << read.near() << "\n^^^ parsing line" << std::endl;
        }

        FootnoteDefinition node(symbol);
        InlineParser::process(suffix, node);

        ast.pushback_child(std::make_shared<FootnoteDefinition>(node));
        read.move_next_line();
    }
};
}  // namespace almo