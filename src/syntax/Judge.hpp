#pragma once

#include "../interfaces/ast.hpp"
#include "../interfaces/parse.hpp"
#include "../interfaces/syntax.hpp"
#include "../utils.hpp"

namespace almo {

struct Judge : public ASTNode {
   private:
    // 問題のタイトル
    std::string title;

    // サンプル入力(単一ファイルのパス)
    std::string sample_in_path;

    // サンプル出力(単一ファイルのパス)
    std::string sample_out_path;

    // ジャッジの種類. 誤差ジャッジ (err_{rate}) または 完全一致ジャッジ (equal)
    // が指定される. C++側では特に処理は変わらず、 JS側に `judge_type[uuid]`
    // を指定することで伝えれば切り替わる。
    std::string judge_type;

    // エディタにデフォルトで入力されてあるコード。
    std::string source;

    // 入力ファイルを表す glob パターン
    std::string in_files_glob;

    // 出力ファイルを表す glob パターン
    std::string out_files_glob;

    // エディタのテーマ
    std::string editor_theme;

   public:
    Judge(std::string title, std::string sample_in_path,
          std::string sample_out_path, std::string in_files_glob,
          std::string out_files_glob, std::string judge_type,
          std::string source, std::string editor_theme)
        : title(title),
          sample_in_path(sample_in_path),
          sample_out_path(sample_out_path),
          in_files_glob(in_files_glob),
          out_files_glob(out_files_glob),
          judge_type(judge_type),
          source(source),
          editor_theme(editor_theme) {
        // ジャッジが`err_{rate}` または `equal` 以外の場合はエラーを出す.
        if (judge_type.substr(0, 4) != "err_" && judge_type != "equal") {
            throw ParseError(
                "Invalid judge type. Expected `err_{rate}` or `equal`, but `" +
                judge_type + "` is given.");
        }
        set_uuid();
    }

    std::string to_html() const override {
        std::string uuid_str = get_uuid_str();

        // タイトルを作る
        std::string title_h3 =
            "<h3 class=\"problem_title\"> <div class='badge' id='" + uuid_str +
            "_status'>WJ</div>   " + title + " </h2>\n";

        // ここから ace editor を作る.
        // まず editor を入れる用の div を作る.
        std::string editor_div = "<div class=\"editor\" id=\"" + uuid_str +
                                 "\" rows=\"3\" cols=\"80\"></div> \n";

        // ace editor の設定をする.

        std::string source_code = join(read_file(source), "\n");

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
            "    minLines: 25, "
            "    maxLines: 25, "
            "    fontSize: \"14px\""
            "});"
            "editor.renderer.setScrollMargin(10, 10);"
            "editor.setValue(`" +
            source_code +
            "`, -1);"
            "</script>\n";

        // サンプル入力を読み込む.
        std::string sample_in = join(read_file(sample_in_path));
        std::string sample_out = join(read_file(sample_out_path));

        // サンプル入力と出力を表示する用の div を作る.
        std::string sample_in_area =
            "<div class=\"box-title\"> サンプルの入力 </div>"
            "<pre class=\"sample_in\" id=\"" +
            uuid_str + "_sample_in\">" + sample_in + "</pre>\n";

        std::string sample_out_area =
            "<div class=\"box-title\"> 出力 </div>"
            "<pre class=\"sample_out\" id=\"" +
            uuid_str + "_out\"></pre>\n";

        std::string expect_out_area =
            "<div class=\"box-title\"> サンプルの答え </div>"
            "<pre class=\"expect_out\" id=\"" +
            uuid_str + "_expect_out\">" + sample_out + "</pre>\n";

        // データを定義する。 all_input, all_output, all_sample_input,
        // all_sample_output, problem_status は JS側で使うために定義する.
        // また、目次生成のために page_contents にも追加する.
        std::string define_data =
            "<script>"
            "all_input[\"" +
            uuid_str +
            "\"] = [];\n"
            "all_output[\"" +
            uuid_str +
            "\"] = [];\n"
            "all_sample_input[\"" +
            uuid_str + "\"] = `" + sample_in +
            "`;\n"
            "all_sample_output[\"" +
            uuid_str + "\"] = `" + sample_out +
            "`;\n"
            "problem_status[\"" +
            uuid_str +
            "\"] = \"WJ\";\n"
            "page_contents.push({\n"
            "    \"type\":\"Problem\",\n"
            "    \"id\":\"" +
            uuid_str +
            "\",\n"
            "    \"title\":\"" +
            title +
            "\"\n"
            "});\n";

        // サンプル入力と出力のファイルの一覧を取得して、それぞれのデータを
        // all_input, all_output に追加する.
        std::vector<std::string> in_files = glob(in_files_glob);
        std::vector<std::string> out_files = glob(out_files_glob);

        for (std::string in_file : in_files) {
            std::string input = join(read_file(in_file));
            define_data +=
                "\n all_input[\"" + uuid_str + "\"].push(`" + input + "`)";
        }

        for (std::string out_file : out_files) {
            std::string output = join(read_file(out_file));
            define_data +=
                "\n all_output[\"" + uuid_str + "\"].push(`" + output + "`)";
        }

        define_data += "</script> \n";

        // テスト実行ボタンと提出ボタンを作る.
        std::string test_run_button =
            "<button class=\"runbutton\" onclick=\"runCode('" + uuid_str +
            "', false)\"> Run Sample </button>\n";

        std::string submit_button =
            "<button class=\"submitbutton\" onclick=\"runCode('" + uuid_str +
            "', true)\"> Submit </button>\n";

        // ジャッジの種類を JS側に伝えるためのコードを作る.
        std::string judge_code =
            "<script>\n"
            "judge_types[\"" +
            uuid_str + "\"] = `" + judge_type +
            "`\n"
            "</script>\n";

        std::string output = title_h3 + editor_div + ace_editor +
                             sample_in_area + sample_out_area +
                             expect_out_area + define_data + test_run_button +
                             submit_button + judge_code;

        return output;
    }

    std::map<std::string, std::string> get_properties() const override {
        return {{"title", title},
                {"sample_in_path", sample_in_path},
                {"sample_out_path", sample_out_path},
                {"in_files_glob", in_files_glob},
                {"out_files_glob", out_files_glob},
                {"judge_type", judge_type},
                {"source", source}};
    }
    std::string get_classname() const override { return "Judge"; }
};

struct JudgeSyntax : BlockSyntax {
    bool operator()(Reader &read) const override {
        if (!read.is_line_begin()) return false;
        if (rtrim(read.get_row()) == ":::judge") return true;
        return false;
    }
    void operator()(Reader &read, ASTNode &ast) const override {
        // BEGIN : ':::judge\s*'
        // END   : ':::\s*'
        std::map<std::string, std::string> judge_info;

        const std::vector<std::string> required_args = {
            "title", "sample_in", "sample_out", "in", "out",
        };

        std::map<std::string, std::string> optional_args = {
            {"judge", "equal"},
            {"source", ""},
        };

        // skip :::judge
        read.move_next_line();

        while (!read.is_eof()) {
            if (rtrim(read.get_row()) == ":::") {
                read.move_next_line();
                break;
            }
            std::size_t mid = read.get_row().find("=");
            if (mid == std::string::npos) {
                throw SyntaxError("judge option must be separated with '='");
            }
            std::string key = rtrim(read.get_row().substr(0u, mid));
            std::string value = rtrim(ltrim(read.get_row().substr(mid + 1)));
            if (judge_info.contains(key)) {
                throw SyntaxError("Duplicate judge options : " + key);
            }
            judge_info[key] = value;
            read.move_next_line();
        }

        // required args check
        for (std::string arg : required_args) {
            if (judge_info.contains(arg)) continue;
            // lost args
            // set title
            if (judge_info.contains("title")) {
                throw SyntaxError("問題" + judge_info["title"] + "の引数 " +
                                  arg +
                                  " がありません. 引数を追加してください.");
            } else {
                throw SyntaxError(
                    "問題タイトルがありません. 引数を追加してください.");
            }
        }

        // optional args check
        for (auto [arg, default_value] : optional_args) {
            if (judge_info.contains(arg)) continue;
            judge_info[arg] = default_value;
        }

        Judge node(judge_info["title"], judge_info["sample_in"],
                   judge_info["sample_out"], judge_info["in"],
                   judge_info["out"], judge_info["judge"], judge_info["source"],
                   "{{ editor_theme }}");

        ast.pushback_child(std::make_shared<Judge>(node));
    }
};

}  // namespace almo