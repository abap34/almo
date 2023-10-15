#include "parse.hpp"
#include "render.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include "utils.hpp"

int main(int argc, char* argv[]) {
    // コマンドライン引数のデフォルト値を設定
    std::string theme = "light";
    std::string css_setting = "light";
    bool debug = false;
    std::string editor_theme = "ace/theme/xcode";
    std::string syntax_theme = "github.min";
    bool plot_graph = false;

    // 出力ファイルのデフォルト値を設定。 .md -> .html に置き換える
    std::string out_path = argv[1];
    if (out_path.ends_with(".md")) {
        out_path = out_path.substr(0, out_path.length() - 3) + ".html";
    }
    else {
        throw InvalidCommandLineArgumentsError("不正な入力ファイルです。 .md 拡張子を持つファイルを入力してください。");
        exit(1);
    }


    for (int i = 2; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (strlen(argv[i]) > 3) {
                throw InvalidCommandLineArgumentsError("不正なコマンドライン引数です。 -h オプションでヘルプを確認してください。");
                exit(1);
            }
            if (argv[i][1] == 'o') {
                out_path = argv[i + 1];
            }
            else if (argv[i][1] == 't') {
                theme = argv[i + 1];
            }
            else if (argv[i][1] == 'c') {
                css_setting = argv[i + 1];
            }
            else if (argv[i][1] == 'd') {
                debug = true;
            }
            else if (argv[i][1] == 'e') {
                editor_theme = argv[i + 1];
            }
            else if (argv[i][1] == 's') {
                syntax_theme = argv[i + 1];
            }
            else if (argv[i][1] == 'g') {
                plot_graph = true;
            }
            else if (argv[i][1] == 'h') {
                if (argc > 3) {
                    throw InvalidCommandLineArgumentsError("不正なコマンドライン引数です。 -h オプションと他のオプションは同時に指定できません。");
                    exit(1);
                }
                std::cout << "使用法: almo <入力> [オプション]" << std::endl;
                std::cout << "オプション:" << std::endl;
                std::cout << "  -o <出力>     出力ファイル名を指定します。デフォルトは入力ファイル名の拡張子を .html に変更したものです。" << std::endl;
                std::cout << "  -t <テーマ>   テーマを指定します。デフォルトは light です。" << std::endl;
                std::cout << "  -c <CSS>      CSSファイルを指定します。デフォルトは テーマに付属するものが使用されます。" << std::endl;
                std::cout << "  -e <テーマ>   エディタのテーマを指定します。デフォルトは  です。" << std::endl;
                std::cout << "  -d            デバッグモードで実行します。" << std::endl;
                std::cout << "  -g            構文木をdot言語として出力します。" << std::endl;
                std::cout << "  -h            ヘルプを表示します。" << std::endl;
            }
            else {
                std::cerr << "Unkown command line arguments is passed." << std::endl;
                std::cerr << "Please check the option by `almo -h`" << std::endl;
                exit(1);
            }
        }
    }


    // もし theme が dark であれば、 editor_theme と syntax_theme も対応したものにしておく
    if (theme == "dark") {
        editor_theme = "ace/theme/monokai";
        syntax_theme = "monokai-sublime.min";
        css_setting = "dark";
    }


    // パース
    auto [meta_data, ast] = almo::parse_md_file(argv[1]);

    // コマンドライン引数を meta_data に追加
    meta_data["theme"] = theme;
    meta_data["out_path"] = out_path;
    meta_data["css_setting"] = css_setting;
    meta_data["editor_theme"] = editor_theme;
    meta_data["syntax_theme"] = syntax_theme;


    if (debug) {
        std::string ir = ast.to_json();
        std::string meta_dump = "{\n";
        for (auto [key, value] : meta_data) {
            meta_dump += "   \"" + key + "\": \"" + escape(value) + "\",";
        }
        // 最後のカンマを削除
        meta_dump = meta_dump.substr(0, meta_dump.length() - 1);
        meta_dump += "} \n";
        std::string output = "{\n"
            "\"meta\": " + meta_dump + ",\n"
            "\"ir\": " + ir + "\n"
            "}\n";

        
        std::cout << output << std::endl;
    }

    if (plot_graph) {
        std::string graph = ast.to_dot();
        std::cout << "digraph g {" << std::endl;
        std::cout << "    graph [" << std::endl;
        std::cout << "        labelloc=\"t\";" << std::endl;
        std::cout << "        ];" << std::endl;
        std::cout << graph << std::endl;
        std::cout << "}" << std::endl;
    }



    almo::meta_data = meta_data;

    almo::render(ast, meta_data, out_path);
    return 0;
}