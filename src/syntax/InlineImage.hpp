#pragma once

#include "inline_match.hpp"

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"
#include "../utils.hpp"

namespace almo {

struct InlineImage : public ASTNode {
   public:
    std::shared_ptr<ASTNode> clone() const override {
        return clone_as<InlineImage>();
    }

   private:
    std::string url;
    std::string caption;

   public:
    InlineImage(std::string url, std::string caption)
        : url(std::move(url)), caption(std::move(caption)) {
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
    static inline const std::regex rex{R"(\!\[(.*?)\]\((.*?)\))"};
    std::optional<InlineMatch> find(std::string_view source) const override {
        return detail::find_link_inline(source, "![");
    }
    void parse(const InlineMatch& matched, ASTNode& ast) const override {
        ast.pushback_child(std::make_shared<InlineImage>(matched.captures[1], matched.captures[0]));
    }
};

}  // namespace almo
