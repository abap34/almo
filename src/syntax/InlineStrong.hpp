#pragma once

#include "inline_match.hpp"

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"

namespace almo {

struct InlineStrong : public ASTNode {
   public:
    std::shared_ptr<ASTNode> clone() const override {
        return clone_as<InlineStrong>();
    }

    InlineStrong() { set_uuid(); }

    std::string to_html() const override {
        return "<span class=\"strong\"><strong>" + concatenated_childs_html() +
               "</strong></span>";
    }

    std::map<std::string, std::string> get_properties() const override {
        return {};
    }
    std::string get_classname() const override { return "InlineStrong"; }
};

struct InlineStrongSyntax : public InlineSyntax {
    static inline const std::regex rex{R"(\*\*(.*?)\*\*)"};
    std::optional<InlineMatch> find(std::string_view source) const override {
        return detail::find_delimited_inline(source, "**", "**");
    }
    void parse(const InlineMatch& matched, ASTNode& ast) const override {
        auto node = std::make_shared<InlineStrong>();
        InlineParser::process(matched.captures[0], *node);
        ast.pushback_child(std::move(node));
    }
};

}  // namespace almo
