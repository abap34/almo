#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <glob.h>
#include "json.hpp"
#include "utils.hpp"

namespace almo {
    // to_html, to_config するためにグローバルな設定が保存されるグローバル変数.
    // 格納されるもの:
    // - editor_theme: Ace Editorのテ   ーマ
    // - syntax_theme: Highlight.jsのテーマ
    std::map<std::string, std::string> global_config;

    // 各ノードに対して一意なIDを生成する用の関数オブジェクト. 呼ぶと前に呼ばれた時の値 + 1 が返る.
    struct UUID_gen {
        std::string operator()() {
            static int uuid = 0;
            return std::to_string(uuid++);
        }
    }uuid;

    // 構文木の各要素を表す抽象クラス. 全ての構文ブロックを表す構造体はこれを継承する. 
    // `Inline` という prefix がついているクラスは、インラインでの記法が許されている構文.
    class ASTNode {
    public:
        // 構文木を json に変換するときに、そのノードの情報を json に追加する. それを頂点とする部分木を json に変換するわけではないので注意.
        virtual void add_json(nlohmann::json& json) const = 0;

        virtual ~ASTNode() = default;

        // このノードが構文木の葉ノードかどうかを返す. 
        virtual bool is_leaf() const = 0;
    };

    // 葉ノードでないノード。このノードは、プロパティを持たず、子から伝播してきた `child_html` に変換を施し親に渡す.
    class NonLeafNode : public ASTNode {
    public:
        bool is_leaf() const override {
            return false;
        }

        // このノードの子ノード.
        std::vector<std::shared_ptr<ASTNode>> childs = {};
        // 構文木をhtmlに変換する。子ノードが存在するので、それぞれを html に変換したものの vector が渡される.
        virtual std::string to_html(std::vector<std::string> childs_html) const = 0;
    };

    // 葉ノード。このノードは、htmlをゼロから生成する.
    class LeafNode : public ASTNode {
    public:
        bool is_leaf() const override {
            return true;
        }

        // 構文木をhtmlに変換する. 葉ノードなので子は持たないため、引数なしで呼ばれる.
        virtual std::string to_html() const = 0;
    };

    // H1~6 に対応する構文木のノード
    class Header : public NonLeafNode {
        std::string uuid;

        // ヘッダのレベル. H{level} に対応する.
        int level;
    public:
        Header(int level, std::string uuid) : level(level), uuid(uuid) {
            if (level < 1 || level > 6) {
                throw ParseError("Internal Error: Header level must be 1~6. But " + std::to_string(level) + " is given.");
            }
        }

        std::string to_html(std::vector<std::string> childs_html) const override {
            return "<h" + std::to_string(level) + ">" + join(childs_html) + "</h" + std::to_string(level) + ">";
        }

        void add_json(nlohmann::json& json) const override {
            json["class"] = "Header";
            json["level"] = level;
            json["uuid"] = uuid;
        }
    };


    // 複数のブロックをまとめるために使う構文木のノード. これ自体には意味はないことに注意。
    // 例えば一行分 parseしたとき、 `Block` を頂点する木としてパース結果が帰ってくる。 
    class Block : public NonLeafNode {
        std::string uuid;
    public:
        Block(std::string uuid) : uuid(uuid) { }

        // これ自体は意味を持たないので、子ノードが変換されたものをそのまま返す.
        std::string to_html(std::vector<std::string> childs_html) const override {
            return join(childs_html);
        }

        void add_json(nlohmann::json& json) const override {
            json["class"] = "Block";
            json["uuid"] = uuid;
        }
    };

    // 葉ノードとして文字情報を持つ構文木のノード. これ自体には意味はないことに注意。
    // 色々なクラスで末端として情報を持ち、また再帰の終了のために使う。　
    // 例えば **Hoge** という文字列をパースしたとき、
    // InlineStrong -> Raw(content="Hoge") という木として結果をもつ。
    class RawText : public LeafNode {
        std::string uuid;

        // このノードが持つ文字列
        std::string content;
    public:
        RawText(std::string content, std::string uuid) : content(content), uuid(uuid) { }
        std::string to_html() const override {
            return content;
        }

        void add_json(nlohmann::json& json) const override {
            json["class"] = "RawText";
            json["content"] = content;
            json["uuid"] = uuid;
        }
    };

    // 数式ブロック. 内容はMathJaxでレンダリングされる. 内容は `math-block` というクラスが付与されたdivタグで囲まれる
    class MathBlock : public LeafNode {
        std::string uuid;

        // markdownで渡された式をそのまま string で持つ。
        std::string expression;

    public:
        MathBlock(std::string expression, std::string uuid) : expression(expression), uuid(uuid) { }

        // mathjax の　複数行数式用に \[ \] で囲む
        std::string to_html() const override {
            return "<div class=\"math-block\"> \\[ \n" + expression + "\n \\] </div>";
        }

        void add_json(nlohmann::json& json) const override {
            json["class"] = "MathBlock";
            json["expression"] = expression;
            json["uuid"] = uuid;
        }
    };

    // インラインの数式ブロック. 内容はMathJaxでレンダリングされる. 内容は `math-inline` というクラスが付与されたspanタグで囲まれる
    class InlineMath : public LeafNode {
        std::string uuid;

        // markdownで渡された式をそのまま string で持つ。
        std::string expr;
    public:
        InlineMath(std::string expr, std::string uuid) : expr(expr), uuid(uuid) { }

        // mathjax の　インライン数式用に \( \) で囲む
        std::string to_html() const override {
            return "<span class=\"math-inline\"> \\( " + expr + " \\) </span>";
        }


        void add_json(nlohmann::json& json) const override {
            json["class"] = "InlineMath";
            json["expr"] = expr;
            json["uuid"] = uuid;
        }
    };

    // 打消し. <s>タグで囲まれる
    class InlineOverline : public NonLeafNode {
        std::string uuid;
    public:
        InlineOverline(std::string uuid) : uuid(uuid) { }
        std::string to_html(std::vector<std::string> childs_html) const override {
            return "<span class=\"overline\"> <s>" + join(childs_html) + "</s> </span>";
        }
        void add_json(nlohmann::json& json) const override {
            json["class"] = "InlineOverline";
            json["uuid"] = uuid;
        }
    };


    // 強調. <strong>タグで囲まれる
    class InlineStrong : public NonLeafNode {
        std::string uuid;
    public:
        InlineStrong(std::string uuid) : uuid(uuid) { }
        std::string to_html(std::vector<std::string> childs_html) const override {
            return "<span class=\"strong\"> <strong>" + join(childs_html) + "</strong> </span>";
        }
        void add_json(nlohmann::json& json) const override {
            json["class"] = "InlineStrong";
            json["uuid"] = uuid;
        }
    };

    // イタリック. <i>タグで囲まれる
    class InlineItalic : public NonLeafNode {
        std::string uuid;
    public:
        InlineItalic(std::string uuid) :uuid(uuid) { }
        std::string to_html(std::vector<std::string> childs_html) const override {
            return "<span class=\"italic\"> <i>" + join(childs_html) + "</i> </span>";
        }
        void add_json(nlohmann::json& json) const override {
            json["class"] = "InlineItalic";
            json["uuid"] = uuid;
        }
    };

    // プレインテキストを表すクラス.  <div class="plain-text"> タグで囲まれる
    class PlainText : public NonLeafNode {
        std::string uuid;
    public:
        PlainText(std::string uuid) : uuid(uuid) { }
        std::string to_html(std::vector<std::string> childs_html) const override {
            return "<div class=\"plain-text\">" + join(childs_html) + "</span>";
        }
        void add_json(nlohmann::json& json) const override {
            json["class"] = "PlainText";
            json["uuid"] = uuid;
        }
    };


    // コードブロックを表すクラス.  <div class="code-block"> タグで囲まれ、その中に <pre><code> タグが入る.
    class CodeBlock : public LeafNode {
        std::string uuid;

        // コードの中身。
        std::string code;

        // Highlight.js によって正確にハイライトするために、言語を指定する必要がある。
        // ```python -> python を持っておき、 `to_html` する際に <code class="python"> として出力する。
        std::string language;
    public:
        CodeBlock(std::string code, std::string language, std::string uuid) : code(code), language(language), uuid(uuid) { }

        // Highlight.js によって正確にハイライトするために、<code> にクラスを付与する。
        std::string to_html() const override {
            return "<div class=\"code-block\"> <pre><code class=\"" + language + "\">" + code + "</code></pre> </div>";
        }


        void add_json(nlohmann::json& json) const override {
            json["class"] = "CodeBlock";
            json["code"] = code;
            json["language"] = language;
            json["uuid"] = uuid;
        }
    };

    // インラインのコードブロックを表すクラス. <div class="inline-code"> タグで囲まれる.
    class InlineCodeBlock : public LeafNode {
        std::string uuid;
        std::string code;
    public:
        InlineCodeBlock(std::string code, std::string uuid) : code(code), uuid(uuid) { }
        std::string to_html() const override {
            return "<span class=\"inline-code\"> <code>" + code + "</code> </span>";
        }
        void add_json(nlohmann::json& json) const override {
            json["class"] = "InlineCodeBlock";
            json["code"] = code;
            json["uuid"] = uuid;
        }
    };



    // 独自記法のジャッジ付きプログラム実行環境を表すクラス。
    class Judge : public LeafNode {
        std::string uuid;

        // 問題のタイトル
        std::string title;

        // サンプル入力(単一ファイルのパス)
        std::string sample_in_path;

        // サンプル出力(単一ファイルのパス)
        std::string sample_out_path;

        // ジャッジの種類. 誤差ジャッジ (err_{rate}) または 完全一致ジャッジ (equal) が指定される.
        // C++側では特に処理は変わらず、 JS側に `judge_type[uuid]` を指定することで伝えれば切り替わる。
        std::string judge_type;

        // エディタにデフォルトで入力されてあるコード。
        std::string source;

        // 入力ファイルを表す glob パターン
        std::string in_files_glob;

        // 出力ファイルを表す glob パターン
        std::string out_files_glob;
    public:
        Judge(std::string title, std::string sample_in_path, std::string sample_out_path, std::string in_files_glob, std::string out_files_glob, std::string judge_type, std::string source, std::string uuid) : title(title), sample_in_path(sample_in_path), sample_out_path(sample_out_path), in_files_glob(in_files_glob), out_files_glob(out_files_glob),  judge_type(judge_type), source(source), uuid(uuid) {
            // ジャッジが`err_{rate}` または `equal` 以外の場合はエラーを出す.
            if (judge_type.substr(0, 4) != "err_" && judge_type != "equal") {
                throw ParseError("Invalid judge type. Expected `err_{rate}` or `equal`, but `" + judge_type + "` is given.");
            }
        }
        std::string to_html() const override {


            // タイトルを作る
            std::string title =
                "<h3 class=\"problem_title\"> <div class='badge' id='" + uuid + "_status'>WJ</div>   " + title + " </h2>\n";

            // ここから ace editor を作る.
            // まず editor を入れる用の div を作る.
            std::string editor_div = "<div class=\"editor\" id=\"" + uuid + "\" rows=\"3\" cols=\"80\"></div> \n";

            // ace editor の設定をする.
            // テーマは global_config から取得する.
            std::string ace_editor = ""
                "<script>"
                "editor = ace.edit(\"" + uuid + "\"); "
                "editor.setTheme(\"" + global_config["editor_theme"] + "\");"
                "editor.session.setMode(\"ace/mode/python\");"
                "editor.setShowPrintMargin(false);"
                "editor.setHighlightActiveLine(false);"
                "editor.setOptions({"
                "    enableBasicAutocompletion: true,"
                "    enableSnippets: true,"
                "    enableLiveAutocompletion: true,"
                "    minLines: 25, "
                "    maxLines: 25, "
                "    fontSize: \"14px\""
                "});"
                "editor.renderer.setScrollMargin(10, 10);"
                "editor.setValue(`" + source + "`, -1);"
                "</script>\n";

            // サンプル入力を読み込む.
            std::string sample_in = read_file(sample_in_path);
            std::string sample_out = read_file(sample_out_path);

            // サンプル入力と出力を表示する用の div を作る.
            std::string sample_in_area =
                "<div class=\"box-title\"> サンプルの入力 </div>"
                "<pre class=\"sample_in\" id=\"" + uuid + "_sample_in\">" + sample_in + "</pre>\n";

            std::string sample_out_area =
                "<div class=\"box-title\"> 出力 </div>"
                "<pre class=\"sample_out\" id=\"" + uuid + "_sample_out\"></pre>\n";

            std::string expect_out_area =
                "<div class=\"box-title\"> サンプルの答え </div>"
                "<pre class=\"expect_out\" id=\"" + uuid + "_expect_out\">" + sample_out + "</pre>\n";

            // データを定義する。 all_input, all_output, all_sample_input, all_sample_output, problem_status は JS側で使うために定義する.
            // また、目次生成のために page_contents にも追加する.
            std::string define_data =
                "<script>"
                "all_input[\"" + uuid + "\"] = [];\n"
                "all_output[\"" + uuid + "\"] = [];\n"
                "all_sample_input[\"" + uuid + "\"] = `" + sample_in + "`;\n"
                "all_sample_output[\"" + uuid + "\"] = `" + sample_out + "`;\n"
                "problem_status[\"" + uuid + "\"] = \"WJ\";\n"
                "page_contents.push({\n"
                "    \"type\":\"Problem\",\n"
                "    \"id\":\"" + uuid + "\",\n"
                "    \"title\":\"" + title + "\"\n"
                "});\n";

            // サンプル入力と出力のファイルの一覧を取得して、それぞれのデータを all_input, all_output に追加する.
            std::vector<std::string> in_files = glob(in_files_glob);
            std::vector<std::string> out_files = glob(out_files_glob);


            for (std::string in_file : in_files) {
                std::string input = read_file(in_file);
                define_data += "\n all_input[\"" + uuid + "\"].push(`" + input + "`)";
            }

            for (std::string in_file : out_files) {
                std::string output = read_file(in_file);
                define_data += "\n all_output[\"" + uuid + "\"].push(`" + output + "`)";
            }

            define_data += "</script> \n";

            // テスト実行ボタンと提出ボタンを作る.
            std::string test_run_button =
                "<button class=\"runbutton\" onclick=\"runCode('" + uuid + "', false)\"> Run Sample </button>\n";

            std::string submit_button =
                "<button class=\"submitbutton\" onclick=\"runCode('" + uuid + "', true)\"> Submit </button>\n";

            // ジャッジの種類を JS側に伝えるためのコードを作る.
            std::string judge_code =
                "<script>\n"
                "judge_types[\"" + uuid + "\"] = `" + judge_type + "`\n"
                "</script>\n";


            std::string output = title + editor_div + ace_editor + sample_in_area + sample_out_area + expect_out_area + define_data + test_run_button + submit_button + judge_code;

            return output;
        }

        void add_json(nlohmann::json& json) const override {
            json["class"] = "Judge";
            json["title"] = title;
            json["sample_in"] = sample_in_path;
            json["sample_out"] = sample_out_path;
            json["source"] = source;
            json["in_files_glob"] = in_files_glob;
            json["out_files_glob"] = out_files_glob;
            json["uuid"] = uuid;
        }
    };


    // 実行可能なコードブロックを表すクラス. 
    // `Judge` と異なり、こちらはジャッジを提供せず、単に実行のみを行う。
    class ExecutableCodeBlock : public LeafNode {
        std::string uuid;
        std::string code;
    public:
        ExecutableCodeBlock(std::string code, std::string uuid) : code(code), uuid(uuid) { }
        std::string to_html() const override {
            // コード全体を表示するために、何行のコードかを調べておく
            int n_line = std::count(code.begin(), code.end(), '\n');

            // `minLines` と `maxLines` を適切に設定してコード全体を表示するようにする。
            std::string ace_editor = ""
                "<script>"
                "editor = ace.edit(\"" + uuid + "\"); "
                "editor.setTheme(\"" + global_config["editor_theme"] + "\");"
                "editor.session.setMode(\"ace/mode/python\");"
                "editor.setShowPrintMargin(false);"
                "editor.setHighlightActiveLine(false);"
                "editor.setOptions({"
                "    enableBasicAutocompletion: true,"
                "    enableSnippets: true,"
                "    enableLiveAutocompletion: true,"
                "    minLines: " + std::to_string(n_line + 1) + ", "
                "    maxLines: " + std::to_string(n_line + 1) + ", "
                "    fontSize: \"14px\""
                "});"
                "editor.renderer.setScrollMargin(10, 10);"
                "editor.setValue(`" + code + "`, -1);"
                "</script>\n";

            // 通常の出力に加えて、matplotlib のプロットを表示するための div を作っておく。
            std::string out_area = "<pre class=\"exec_out\" id=\"" + uuid + "_output\"></pre>\n";
            std::string plot_area = "<div class=\"exec_plot\" id=\"" + uuid + "_plot\"></div>\n";

            std::string run_button =
                "<button class=\"runbutton\" onclick=\"runCode('" + uuid + "', false)\"> Run </button>\n";

            std::string output = ace_editor + out_area + plot_area + run_button;

            return output;
        }

        void add_json(nlohmann::json& json) const override {
            json["class"] = "ExecutableCodeBlock";
            json["code"] = code;
            json["uuid"] = uuid;
        }
    };

    // ライブラリの読み込みをする独自記法. 
    class LoadLib : public LeafNode {
        std::string uuid;

        // 読み込むライブラリの名前のリスト
        std::vector<std::string> libs;

    public:
        LoadLib(std::vector<std::string> libs, std::string uuid) : libs(libs), uuid(uuid) { }

        // use_libs に追加しておくと JS側で読み込み処理を行う。
        std::string to_html() const override {
            std::string output = "";
            for (std::string lib : libs) {
                std::string output = "<script> use_libs.push(\"" + lib + "\"); </script>";
            }
            return output;
        };

        void add_json(nlohmann::json& json) const override {
            json["class"] = "LoadLib";
            json["libs"] = libs;
            json["uuid"] = uuid;
        }
    };

    // リンク表記を表すクラス. <url> タグで囲まれる. 
    class InlineUrl : public LeafNode {
        std::string uuid;
        std::string url;
        std::string alt;
    public:
        InlineUrl(std::string url, std::string alt, std::string uuid) : url(url), alt(alt), uuid(uuid) { };

        std::string to_html() const override {
            return "<url> <a href=\"" + url + "\">" + url + "</a> </url>";
        }

        void add_json(nlohmann::json& json) const override {
            json["class"] = "InlineUrl";
            json["url"] = url;
        }
    };

    // 画像表記を表すクラス. 
    class InlineImage : public LeafNode {
        std::string uuid;
        std::string url;
        std::string caption;

    public:
        InlineImage(std::string url, std::string caption, std::string uuid) : url(url), caption(caption), uuid(uuid) { }
        //　<figure> タグを使うことで キャプションなどをつける。
        std::string to_html() const override {
            std::string output = "<img src=\" " + url + " \" >";
            std::string figcaption = "<figcaption>" + caption + "</figcaption>";
            return "<figure>" + output + figcaption + "</figure>";
        }

        void add_json(nlohmann::json& json) const override {
            json["class"] = "InlineImage";
            json["url"] = url;
            json["caption"] = caption;
            json["uuid"] = uuid;
        }

    };

    // 改行を表すクラス
    class NewLine : public LeafNode {
        std::string uuid;

    public:
        std::string to_html() const override {
            return "<br>";
        }

        void add_json(nlohmann::json& json) const override {
            json["class"] = "NewLine";
            json["uuid"] = uuid;
        }
    };

    // 箇条書き(番号なし) を表すクラス
    class ListBlock : public NonLeafNode {
        std::string uuid;
    public:
        ListBlock(std::string uuid) : uuid(uuid) { }
        std::string to_html(std::vector<std::string> childs_html) const override {
            return "<ul>" + join(childs_html) + "</ul>";
        }

        void add_json(nlohmann::json& json) const override {
            json["class"] = "ListBlock";
            json["uuid"] = uuid;
        }
    };

    // 番号付き箇条書きを表すクラス 
    class EnumerateBlock : public NonLeafNode {
        std::string uuid;
    public:
        std::string to_html(std::vector<std::string> childs_html) const override {
            return "<ol>" + join(childs_html) + "</ol>";
        }

        void add_json(nlohmann::json& json) const override {
            json["class"] = "EnumerateBlock";
            json["uuid"] = uuid;
        }
    };

    // 箇条書きの要素を表すクラス
    class Item : public NonLeafNode {
        std::string uuid;
    public:
        std::string to_html(std::vector<std::string> childs_html) const override {
            return "<li>" + join(childs_html) + "</li>";
        }

        void add_json(nlohmann::json& json) const override {
            json["class"] = "Item";
            json["uuid"] = uuid;
        }
    };


    // テーブルを表すクラス
    class Table : public NonLeafNode {
        std::string uuid;
        int n_row;
        int n_col;

        // 各列について、左寄せ(0), 中央寄せ(1), 右寄せ(2) のいずれかを指定する.
        std::vector<int> col_format;
        std::vector<std::string> col_names;
    public:
        std::string to_html(std::vector<std::string> childs_html) const override {
            std::string output = "<table>\n";
            output += "<thead>\n";
            output += "<tr>\n";
            for (std::string col_name : col_names) {
                output += "<th>" + col_name + "</th>\n";
            }
            output += "</tr>\n";
            output += "</thead>\n";
            output += "<tbody>\n";
            for (int i = 0; i < n_row; i++) {
                output += "<tr>\n";
                for (int j = 0; j < n_col; j++) {
                    std::string align = col_format[j] == 0 ? "left" : col_format[j] == 1 ? "center" : "right";
                    output += "<td align=\"" + align + "\">" + childs_html[i * n_col + j] + "</td>\n";
                }
                output += "</tr>\n";
            }
            output += "</tbody>\n";
            output += "</table>\n";
            return output;
        }

        void add_json(nlohmann::json& json) const override {
            json["class"] = "Table";
            json["uuid"] = uuid;
            json["n_row"] = n_row;
            json["n_col"] = n_col;
            json["col_format"] = col_format;
            json["col_names"] = col_names;
        }
    };


    // 水平線を表すクラス
    class HorizontalLine : public LeafNode {
        std::string uuid;
    public:
        std::string to_html() const override {
            return "<hr>";
        }

        void add_json(nlohmann::json& json) const override {
            json["class"] = "HorizontalLine";
            json["uuid"] = uuid;
        }
    };


    // 引用を表すクラス
    class Quote : public NonLeafNode {
        std::string uuid;
    public:
        std::string to_html(std::vector<std::string> childs_html) const override {
            return "<blockquote>" + join(childs_html) + "</blockquote>";
        }

        void add_json(nlohmann::json& json) const override {
            json["class"] = "Quote";
            json["uuid"] = uuid;
        }
    };
}