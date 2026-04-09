#pragma once

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"
#include "../utils.hpp"
#include "inline_match_utils.hpp"

namespace almo {

struct InlineImage : public ASTNode {
   private:
    std::string url;
    std::string caption;

   public:
    InlineImage(std::string_view url, std::string_view caption)
        : url(url), caption(caption) {
        set_uuid();
    }

    // 　<figure> タグを使うことで キャプションなどをつける。
    std::string to_html() const override {
        std::string output = "<img src=\"" + url + "\" >";
        std::string figcaption = "<figcaption>" + caption + "</figcaption>";
        return "<figure>" + output + figcaption + "</figure>";
    }

    std::map<std::string, std::string> get_properties() const override {
        return {{"url", url}, {"caption", caption}};
    }
    std::string get_classname() const override { return "InlineImage"; }
};

struct InlineImageSyntax : public InlineSyntax {
    int operator()(std::string_view str) const override {
        inline_match::LinkMatch match;
        if (inline_match::find_link(str, "![", match)) {
            return static_cast<int>(match.start);
        }
        return std::numeric_limits<int>::max();
    }
    void operator()(std::string_view str, ASTNode &ast) const override {
        inline_match::LinkMatch match;
        inline_match::find_link(str, "![", match);
        InlineParser::process(match.prefix, ast);
        InlineImage node(match.target, match.label);
        ast.pushback_child(std::make_shared<InlineImage>(node));
        InlineParser::process(match.suffix, ast);
    }
};

}  // namespace almo
