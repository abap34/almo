#pragma once

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"
#include "../utils.hpp"

namespace almo {

// 実行可能なコードブロックを表すクラス.
// `Judge` と異なり、こちらはジャッジを提供せず、単に実行のみを行う。
struct ExecutableCodeBlock : public ASTNode {
   private:
    std::string code;
    std::string editor_theme;

   public:
    ExecutableCodeBlock(std::string code, std::string editor_theme)
        : code(code), editor_theme(editor_theme) {
        set_uuid();
    }

    std::string to_html() const override {
        // コード全体を表示するために、何行のコードかを調べておく
        int n_line = std::count(code.begin(), code.end(), '\n');

        std::string uuid_str = get_uuid_str();

        // `minLines` と `maxLines`
        // を適切に設定してコード全体を表示するようにする。
        std::string ace_editor =
            ""
            "<script>"
            "editor = ace.edit(\"" +
            uuid_str +
            "\"); "
            "editor.setTheme(\"" +
            editor_theme +
            "\");"
            "editor.session.setMode(\"ace/mode/python\");"
            "editor.setShowPrintMargin(false);"
            "editor.setHighlightActiveLine(false);"
            "editor.setOptions({"
            "    enableBasicAutocompletion: true,"
            "    enableSnippets: true,"
            "    enableLiveAutocompletion: true,"
            "    minLines: " +
            std::to_string(n_line + 1) +
            ", "
            "    maxLines: " +
            std::to_string(n_line + 1) +
            ", "
            "    fontSize: \"14px\""
            "});"
            "editor.renderer.setScrollMargin(10, 10);"
            "editor.setValue(`" +
            code +
            "`, -1);"
            "</script>\n";

        // 通常の出力に加えて、matplotlib のプロットを表示するための div
        // を作っておく。
        std::string editor_div = "<br> \n <div class=\"editor\" id=\"" +
                                 uuid_str +
                                 "\" rows=\"3\" cols=\"80\"></div> \n";
        std::string out_area =
            "<pre class=\"exec_out\" id=\"" + uuid_str + "_out\"></pre>\n";
        std::string plot_area =
            "<div class=\"exec_plot\" id=\"" + uuid_str + "_plot\"></div>\n";

        std::string run_button =
            "<button class=\"runbutton\" onclick=\"runBlock('" + uuid_str +
            "')\"> Run </button>\n";

        std::string output =
            editor_div + ace_editor + out_area + plot_area + run_button;

        return output;
    }

    std::map<std::string, std::string> get_properties() const override {
        return {{"code", code}};
    }
    std::string get_classname() const override { return "ExecutableCodeBlock"; }
};

struct ExecutableCodeBlockSyntax : public BlockSyntax {
    bool operator()(Reader &read) const override {
        if (!read.is_line_begin()) return false;
        if (rtrim(read.get_row()) == ":::code") return true;
        return false;
    }
    void operator()(Reader &read, ASTNode &ast) const override {
        std::string code = "";

        // skip :::code
        read.move_next_line();

        while (!read.is_eof()) {
            if (rtrim(read.get_row()) == ":::") {
                read.move_next_line();
                break;
            }
            code += read.get_row() + "\n";
            read.move_next_line();
        }

        ExecutableCodeBlock node(code, "{{ editor_theme }}");
        ast.pushback_child(std::make_shared<ExecutableCodeBlock>(node));
    }
};

}  // namespace almo