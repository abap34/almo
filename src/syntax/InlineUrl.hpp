#pragma once

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"
#include "../utils.hpp"

namespace almo {

struct InlineUrl : public ASTNode {
   private:
    std::string url;
    std::string alt;

   public:
    InlineUrl(std::string url, std::string alt) : url(url), alt(alt) {
        set_uuid();
    }

    std::string to_html() const override {
        return "<url><a href=\"" + url + "\">" + alt + "</a></url>";
    }

    std::map<std::string, std::string> get_properties() const override {
        return {{"url", url}, {"alt", alt}};
    }
    std::string get_classname() const override { return "InlineUrl"; }
};

struct InlineUrlSyntax : public InlineSyntax {
    static inline const std::regex rex{R"(\[(.*?)\]\((.*?)\))"};
    std::optional<InlineMatch> find(std::string_view source) const override {
        return find_inline(source, rex);
    }
    void parse(const InlineMatch& matched, ASTNode& ast) const override {
        ast.pushback_child(std::make_shared<InlineUrl>(matched.captures[1], matched.captures[0]));
    }
};

}  // namespace almo
