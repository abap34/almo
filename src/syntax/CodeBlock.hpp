#pragma once

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"
#include "../utils.hpp"

namespace almo {

// コードブロックを表すクラス.  <div class="code-block"> タグで囲まれ、その中に
// <pre><code> タグが入る.
struct CodeBlock : public ASTNode {
   private:
    // コードの中身。
    std::string code;

    // Highlight.js
    // によって正確にハイライトするために、言語を指定する必要がある。
    // ```python -> python を持っておき、 `to_html` する際に <code
    // class="language-python"> として出力する。
    std::string language;

   public:
    CodeBlock(std::string code, std::string language)
        : code(code), language(language) {
        set_uuid();
    }

    std::string to_html() const override {
        std::string code_class;

        if (language == "") {
            code_class = "language-plaintext";
        } else {
            code_class = "language-" + language;
        }

        return "<div class=\"code-block\"> <pre><code class=\"" + code_class +
               "\">" + escape_for_html(code) + "</code></pre> </div>";
    }

    std::map<std::string, std::string> get_properties() const override {
        return {{"code", code}, {"language", language}};
    }

    std::string get_classname() const override { return "CodeBlock"; }
};

struct CodeBlockSyntax : public BlockSyntax {
    bool operator()(Reader &read) const override {
        if (!read.is_line_begin()) return false;
        return read.get_row().starts_with("```");
    }
    void operator()(Reader &read, ASTNode &ast) const override {
        // BEGIN : '```(.*)'
        // END   : '```\s*'

        std::string language = rtrim(read.get_row().substr(3));
        read.move_next_line();
        std::string code;
        while (!read.is_eof()) {
            if (rtrim(read.get_row()) == "```") {
                read.move_next_line();
                break;
            }
            code += read.get_row() + "\n";
            read.move_next_line();
        }

        CodeBlock node(code, language);
        ast.pushback_child(std::make_shared<CodeBlock>(node));
    }
};

}  // namespace almo