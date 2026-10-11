#pragma once

#include "inline_match.hpp"

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"
#include "../utils.hpp"

namespace almo {

struct InlineMath : public ASTNode {
   public:
    std::shared_ptr<ASTNode> clone() const override {
        return clone_as<InlineMath>();
    }

   private:
    std::string expr;

   public:
    InlineMath(std::string _expr) : expr(std::move(_expr)) { set_uuid(); }

    // mathjax の　インライン数式用に \( \) で囲む
    std::string to_html() const override {
        return "<span class=\"math-inline\"> \\( " + expr + " \\) </span>";
    }

    std::map<std::string, std::string> get_properties() const override {
        return {{"expr", expr}};
    }
    std::string get_classname() const override { return "InlineMath"; }
};

struct InlineMathSyntax : public InlineSyntax {
    static inline const std::regex rex{R"(\$(.*?)\$)"};
    std::optional<InlineMatch> find(std::string_view source) const override {
        auto begin = source.find('$');
        while (begin != source.npos) {
            if (detail::is_escaped_delimiter(source, begin)) {
                begin = source.find('$', begin + 1);
                continue;
            }
            auto end = source.find_first_of("$\r\n", begin + 1);
            while (end != source.npos && source[end] == '$' &&
                   detail::is_escaped_delimiter(source, end))
                end = source.find_first_of("$\r\n", end + 1);
            if (end == source.npos) break;
            if (source[end] == '$')
                return InlineMatch{{begin, end + 1},
                                   {std::string(source.substr(begin + 1, end - begin - 1))}};
            // Like the other inline syntaxes, math cannot cross CR or LF.
            begin = source.find('$', end + 1);
        }
        return std::nullopt;
    }
    void parse(const InlineMatch& matched, ASTNode& ast) const override {
        ast.pushback_child(std::make_shared<InlineMath>(matched.captures[0]));
    }
};

}  // namespace almo
