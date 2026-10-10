#pragma once

#include "inline_match.hpp"

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"
#include "../utils.hpp"

namespace almo {

struct InlineCodeBlock : public ASTNode {
   public:
    std::shared_ptr<ASTNode> clone() const override {
        return clone_as<InlineCodeBlock>();
    }

   private:
    std::string code;

   public:
    InlineCodeBlock(std::string code) : code(code) { set_uuid(); }

    std::string to_html() const override {
        return "<span class=\"inline-code\"><code>" + escape(code, EscapeFormat::HTML) +
               "</code></span>";
    }

    std::map<std::string, std::string> get_properties() const override {
        return {{"code", code}};
    }
    std::string get_classname() const override { return "InlineCodeBlock"; }
};

struct InlineCodeBlockSyntax : public InlineSyntax {
    static inline const std::regex rex{R"(\`(.*?)\`)"};
    std::optional<InlineMatch> find(std::string_view source) const override {
        return detail::find_delimited_inline(source, "`", "`");
    }
    void parse(const InlineMatch& matched, ASTNode& ast) const override {
        ast.pushback_child(std::make_shared<InlineCodeBlock>(matched.captures[0]));
    }
};

}  // namespace almo
