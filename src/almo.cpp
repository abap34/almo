#include <fstream>
#include <iostream>
#include <string>

#include "parse.hpp"
#include "utils.hpp"

class Config {
   public:
    bool debug = false;
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

        if (argc == 2 && std::string(argv[1]) == "-v") {
            std::cout << "almo " << almo::VERSION << " (" << almo::COMMITHASH
                      << ")" << std::endl;
            exit(0);
        }

        for (int i = 2; i < argc; i++) {
            if (argv[i][0] == '-') {
                if (std::string(argv[i]).length() > 2) {
                    throw InvalidCommandLineArgumentsError(
                        "不正なコマンドライン引数です。 -h "
                        "オプションでヘルプを確認してください。");
                    exit(1);
                }

                switch (argv[i][1]) {
                    case 'o':
                        out_path = argv[i + 1];
                        i++;
                        break;
                    case 'd':
                        debug = true;
                        break;
                    case 'g':
                        plot_graph = true;
                        break;
                    case 'h':
                        throw InvalidCommandLineArgumentsError(
                            "不正なコマンドライン引数です。 -h "
                            "オプションと他のオプションは同時に指定できません"
                            "。");
                    default:
                        std::cerr << "不正なコマンドライン引数です。 -h "
                                     "オプションでヘルプを確認してください。"
                                  << std::endl;
                        exit(1);
                }
            }
        }
    }

    void show_help(bool err) const {
        std::ostream& out = err ? std::cerr : std::cout;
        out << "使用法: almo <入力> [オプション]" << std::endl;
        out << "オプション:" << std::endl;
        out << "  -o <出力>     出力ファイル名を指定します。 "
               "指定しない場合標準出力に出力されます。"
            << std::endl;
        out << "  -d            デバッグモードで実行します。" << std::endl;
        out << "  -g            構文木を dot 言語として出力します。"
            << std::endl;
        out << "  -h            ヘルプを表示します。" << std::endl;
    }
};

void debug_graph(almo::Markdown ast) {
    std::cout << "digraph G {\n graph [labelloc=\"t\"; \n ]\n" + ast.to_dot() +
                     "}"
              << std::endl;
}

int main(int argc, char* argv[]) {
    Config config;
    config.parse_arguments(argc, argv);

    std::vector<std::string> md_content = read_file(argv[1]);

    almo::MarkdownParser parser(md_content);
    almo::Markdown ast;

    parser.process(ast);

    if (config.debug) {
        std::cerr << ast.to_json() << std::endl;
    }

    if (config.plot_graph) {
        std::cerr << ast.to_dot() << std::endl;
        return 0;
    }

    if (config.out_path == "__stdout__") {
        std::cout << ast.to_html();
    } else {
        std::ofstream ofs(config.out_path);
        ofs << ast.to_html();
    }

    return 0;
}
