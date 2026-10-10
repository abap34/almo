#pragma once

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"
#include "../utils.hpp"

namespace almo {
struct FootnoteDefinition : public ASTNode {
   public:
    std::shared_ptr<ASTNode> clone() const override {
        return clone_as<FootnoteDefinition>();
    }

   private:
    std::string symbol;

   public:
    FootnoteDefinition(std::string symbol_) : symbol(std::move(symbol_)) { set_uuid(); }
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
    std::optional<BlockMatch> match(const Reader& read) const override {
        if (!read.is_line_begin()) return std::nullopt;
        const auto row = read.line().text;
        std::match_results<std::string_view::const_iterator> found;
        if (!std::regex_match(row.begin(), row.end(), found, rex)) return std::nullopt;
        return BlockMatch{{found[1].str(), found[2].str()}};
    }
    void parse(Reader& read, ASTNode& ast, const BlockMatch& matched) const override {
        const auto& symbol = matched.captures[0];
        const auto& suffix = matched.captures[1];
        if (symbol.empty())
            read.warn("Footnote symbol is empty. This may cause unexpected behavior.");

        auto node = std::make_shared<FootnoteDefinition>(symbol);
        InlineParser::process(suffix, *node);

        ast.pushback_child(std::move(node));
        read.move_next_line();
    }
};
}  // namespace almo
