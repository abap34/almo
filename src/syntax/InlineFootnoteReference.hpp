#pragma once

#include "inline_match.hpp"

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"
#include "../utils.hpp"

namespace almo {
struct InlineFootnoteReference : public ASTNode {
   public:
    std::shared_ptr<ASTNode> clone() const override {
        return clone_as<InlineFootnoteReference>();
    }

   private:
    std::string symbol;

   public:
    InlineFootnoteReference(std::string _symbol) : symbol(_symbol) {
        set_uuid();
    }

    std::string to_html() const override {
        // This is correct.
        // `label_` is setted in `FootnoteDefinition` (which we have to jump to)
        std::string label = "ref_" + symbol;
        std::string jump_to = "label_" + symbol;

        return "<span class=\"footnote-ref\"><sup id=\"" + label +
               "\"><a href=\"#" + jump_to + "\">[" + symbol +
               "]</a></sup></span>";
    }

    std::map<std::string, std::string> get_properties() const override {
        return {{"symbol", symbol}};
    }
    std::string get_classname() const override {
        return "InlineFootnoteReference";
    }
};
struct InlineFootnoteReferenceSyntax : public InlineSyntax {
    static inline const std::regex rex{R"(\[\^(.*?)\])"};
    std::optional<InlineMatch> find(std::string_view source) const override {
        return detail::find_delimited_inline(source, "[^", "]");
    }
    void parse(const InlineMatch& matched, ASTNode& ast) const override {
        ast.pushback_child(std::make_shared<InlineFootnoteReference>(matched.captures[0]));
    }
};

}  // namespace almo
