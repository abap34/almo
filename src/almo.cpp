#include "parse.hpp"
#include "render.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include "utils.hpp"

int main(int argc, char* argv[]) {
    // コマンドライン引数のデフォルト値を設定
    std::string theme = "light";
    std::string css_setting = "__default__";
    bool debug = false;
    std::string editor_theme = "__default__";
    std::string syntax_theme = "__default__";
    bool plot_graph = false;
    std::string out_path = "__stdout__";


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
                std::cout << "  -o <出力>     出力ファイル名を指定します。 指定しない場合標準出力に出力されます。" << std::endl;
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


    // もし theme が dark であれば、 editor_theme と syntax_theme がデフォルトの場合対応したものにしておく
    if (theme == "light") {
        if (editor_theme == "__default__") {
            editor_theme = "ace/theme/chrome";
        }

        if (syntax_theme == "__default__") {
            syntax_theme = "github.min";
        }

        if (css_setting == "__default__") {
            css_setting = "light";
        }
    }
    else if (theme == "dark") {
        if (editor_theme == "__default__") {
            editor_theme = "ace/theme/monokai";
        }

        if (syntax_theme == "__default__") {
            syntax_theme = "monokai.min";
        }

        if (css_setting == "__default__") {
            css_setting = "dark";
        }
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

    std::string result = almo::render(ast, meta_data);

    if (out_path == "__stdout__") {
        std::cout << result << std::endl;
    }
    else {
        std::ofstream ofs(out_path);
        ofs << result << std::endl;
    }

    return 0;
}