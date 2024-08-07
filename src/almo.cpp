#include "parse.hpp"
#include "render.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include "utils.hpp"

class Config {
public:
    std::string template_file = "__default__";
    std::string theme = "light";
    std::string css_setting = "__default__";
    bool debug = false;
    std::string editor_theme = "__default__";
    std::string syntax_theme = "__default__";
    bool plot_graph = false;
    std::string out_path = "__stdout__";

    void parse_arguments(int argc, char* argv[]) {
        if (argc < 2) {
            std::cerr << "コマンドライン引数が不足しています。" << std::endl;
            show_help(true);
            exit(1);
        }

        if (argc == 2 && std::string(argv[1]) == "-h") {
            show_help(false);
            exit(0);
        }

        for (int i = 2; i < argc; i++) {
            if (argv[i][0] == '-') {
                if (std::string(argv[i]).length() > 2) {
                    throw InvalidCommandLineArgumentsError("不正なコマンドライン引数です。 -h オプションでヘルプを確認してください。");
                    exit(1);
                }
                
                switch (argv[i][1]) {
                    case 'o':
                        out_path = argv[i + 1];
                        i++;
                        break;
                    case 'b':
                        template_file = argv[i + 1];
                        i++;
                        break;
                    case 't':
                        theme = argv[i + 1];
                        i++;
                        break;
                    case 'c':
                        css_setting = argv[i + 1];
                        i++;
                        break;
                    case 'd':
                        debug = true;
                        break;
                    case 'e':
                        editor_theme = argv[i + 1];
                        i++;
                        break;
                    case 's':
                        syntax_theme = argv[i + 1];
                        i++;
                        break;
                    case 'g':
                        plot_graph = true;
                        break;
                    case 'h':
                        throw InvalidCommandLineArgumentsError("不正なコマンドライン引数です。 -h オプションと他のオプションは同時に指定できません。");
                    default:
                        std::cerr << "不正なコマンドライン引数です。 -h オプションでヘルプを確認してください。" << std::endl;
                        exit(1);
                }
            }
        }

        apply_theme_defaults();
    }

    void apply_theme_defaults() {
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
        } else if (theme == "dark") {
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
    }

    void show_help(bool err) const {
        std::ostream& out = err ? std::cerr : std::cout;
        out << "使用法: almo <入力> [オプション]" << std::endl;
        out << "オプション:" << std::endl;
        out << "  -o <出力>     出力ファイル名を指定します。 指定しない場合標準出力に出力されます。" << std::endl;
        out << "  -b <テンプレート> テンプレートファイルを指定します。 " << std::endl;
        out << "  -t <テーマ>   テーマを指定します。デフォルトは light です。" << std::endl;
        out << "  -c <CSS>      CSSファイルを指定します。デフォルトは テーマに付属するものが使用されます。" << std::endl;
        out << "  -e <テーマ>   エディタのテーマを指定します。デフォルトは  です。" << std::endl;
        out << "  -d            デバッグモードで実行します。" << std::endl;
        out << "  -g            構文木をdot言語として出力します。" << std::endl;
        out << "  -h            ヘルプを表示します。" << std::endl;
    }
};


void debug_json(std::string ir_json, std::map<std::string, std::string> meta_data) {
    std::string meta_dump = "{\n";
    for (auto& [key, value] : meta_data) {
        meta_dump += "   \"" + key + "\": \"" + escape(value) + "\",";
    }
    meta_dump = meta_dump.substr(0, meta_dump.length() - 1);
    meta_dump += "} \n";
    std::string output = "{\n"
        "\"meta\": " + meta_dump + ",\n"
        "\"ir\": " + ir_json + "\n"
        "}\n";
    std::cout << output << std::endl;
}

void debug_graph(almo::Markdown ast){
    std::cout << "digraph G {\n graph [labelloc=\"t\"; \n ]\n" + ast.to_dot() + "}" << std::endl;
}

namespace almo_preprocess {


// md ファイルの中身 (frot YAML を含まない)
// を受け取って、前処理 ([コメントの削除, ])　を行う
std::vector<std::string> preprocess(std::vector<std::string> content) {
    std::string content_join = join(content, "\n");

    // コメントを削除
    auto remove_comment = [](std::string text) {
        return _remove_comment(text);
        };


    std::vector<std::function<std::string(std::string)>> hooks = {
        remove_comment,
    };

    for (auto hook : hooks) {
        content_join = hook(content_join);
    }

    content = split(content_join, "\n");

    return content;
}

// md ファイルの中身 (front YAML) 
// を受け取って、 front YAML をパースした結果と残りの md ファイルの開始位置を返す
// TODO: きちんとした YAML パーサを使うようにする。
std::pair<std::vector<std::pair<std::string, std::string>>, int> parse_front(std::vector<std::string> content) {
    std::vector<std::pair<std::string, std::string>> front_yaml;
    int front_yaml_end = 0;

    if (!content.empty() && content[0] == "---") {
        int index = 1;
        while (index < (int)content.size() && content[index] != "---") {
            std::string key = std::regex_replace(content[index], std::regex("(.*):\\s(.*)"), "$1");
            std::string data = std::regex_replace(content[index], std::regex("(.*):\\s(.*)"), "$2");
            front_yaml.emplace_back(key, data);
            index++;
        }
        front_yaml_end = index + 1;
    }

    return { front_yaml, front_yaml_end };
}

// md ファイルの内容から
// メタデータと md の本文の組を返す
std::pair<std::vector<std::string>, std::map<std::string, std::string>> split_markdown(std::vector<std::string> content){
    auto [meta_data, meta_data_end] = parse_front(content);

    // メタデータ以降の行を取り出し
    std::vector<std::string> md_lines(content.begin() + meta_data_end, content.end());

    // 前処理
    md_lines = preprocess(md_lines);

    // meta_data を std::map に変換する
    std::map<std::string, std::string> meta_data_map;

    // デフォルト値を設定
    meta_data_map["title"] = "";
    meta_data_map["date"] = "";
    meta_data_map["author"] = "";
    meta_data_map["twitter_id"] = "";
    meta_data_map["github_id"] = "";
    meta_data_map["mail"] = "";
    meta_data_map["ogp_url"] = "https://www.abap34.com/almo_logo.jpg";
    meta_data_map["tag"] = "";
    meta_data_map["url"] = "";
    meta_data_map["site_name"] = "";
    meta_data_map["twitter_site"] = "";

    for (auto [key, data] : meta_data) {
        meta_data_map[key] = data;
    }

    return {md_lines, meta_data_map};
}

// md ファイルのパスから
// md と メタデータの組を返す
std::pair<std::vector<std::string>, std::map<std::string, std::string>> split_markdown_from_path(std::string path){
    return split_markdown(read_file(path));
}


} // almo_preprocess

int main(int argc, char* argv[]) {
    Config config;
    config.parse_arguments(argc, argv);

    // パース
    auto [md_content, meta_data] = almo_preprocess::split_markdown_from_path(argv[1]);

    // コマンドライン引数を meta_data に追加
    meta_data["template_file"] = config.template_file;
    meta_data["theme"] = config.theme;
    meta_data["out_path"] = config.out_path;
    meta_data["css_setting"] = config.css_setting;
    meta_data["editor_theme"] = config.editor_theme;
    meta_data["syntax_theme"] = config.syntax_theme;

    almo::ParseSummary summary = almo::md_to_summary(md_content, meta_data);

    if (config.debug) {
        debug_json(summary.json, meta_data);
    }

    if (config.plot_graph) {
        debug_graph(summary.ast);
        return 0;
    }


    std::string result = summary.html;

    if (config.out_path == "__stdout__") {
        std::cout << result << std::endl;
    } else {
        std::ofstream ofs(config.out_path);
        ofs << result << std::endl;
    }

    return 0;
}

