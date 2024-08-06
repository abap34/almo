#pragma once

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"
#include "../utils.hpp"

namespace almo {

// 数式ブロック. 内容はMathJaxでレンダリングされる. 内容は `math-block`
// というクラスが付与されたdivタグで囲まれる
struct MathBlock : public ASTNode {
   private:
    // markdownで渡された式をそのまま string で持つ。
    std::string expression;

   public:
    MathBlock(std::string expression) : expression(expression) { set_uuid(); }

    // mathjax の　複数行数式用に \[ \] で囲む
    std::string to_html() const override {
        return "<div class=\"math-block\"> \\[ \n" + expression +
               "\n \\] </div>";
    }

    std::map<std::string, std::string> get_properties() const override {
        return {{"expression", expression}};
    }
    std::string get_classname() const override { return "MathBlock"; }
};

struct MathBlockSyntax : public BlockSyntax {
    bool operator()(Reader &read) const override {
        if (!read.is_line_begin()) return false;
        return read.get_row().starts_with("$$");
    }
    void operator()(Reader &read, ASTNode &ast) const override {
        // BEGIN : '$$.*'
        // END   : '.*$$\s*'
        // note  : The mathematical rendering library is responsible for
        //         the treatment of newlines and whitespace.

        std::string expression = "";

        read.move_next_char(2);

        while (!read.is_eof()) {
            if (rtrim(read.get_rest_row()).ends_with("$$")) {
                std::string tail = rtrim(read.get_rest_row());
                expression += tail.substr(0, tail.size() - 2u);
                read.move_next_line();
                break;
            }
            expression += read.get_rest_row() + "\n";
            read.move_next_line();
        }

        MathBlock node(expression);
        ast.pushback_child(std::make_shared<MathBlock>(node));
    }
};

}  // namespace almo