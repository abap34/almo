#pragma once

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"
#include "../utils.hpp"

namespace almo {

struct InlineImage : public ASTNode {
   private:
    std::string url;
    std::string caption;

   public:
    InlineImage(std::string url, std::string caption)
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
    static inline const std::regex rex =
        std::regex(R"((.*?)\!\[(.*?)\]\((.*?)\)(.*))");
    int operator()(const std::string &str) const override {
        std::smatch sm;
        if (std::regex_search(str, sm, rex)) {
            return sm.position(2) - 2;
        }
        return std::numeric_limits<int>::max();
    }
    void operator()(const std::string &str, ASTNode &ast) const override {
        std::smatch sm;
        std::regex_search(str, sm, rex);
        std::string prefix = sm.format("$1");
        std::string caption = sm.format("$2");
        std::string url = sm.format("$3");
        std::string suffix = sm.format("$4");
        InlineParser::process(prefix, ast);
        InlineImage node(url, caption);
        ast.pushback_child(std::make_shared<InlineImage>(node));
        InlineParser::process(suffix, ast);
    }
};

}  // namespace almo