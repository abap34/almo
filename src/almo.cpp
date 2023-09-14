#include "makejson.hpp"
#include "parse.hpp"
#include "render.hpp"
#include <iostream>
#include <fstream>

int main(int argc, char *argv[]){
    auto [meta_data, asts] = almo::parse_md_file(argv[1]);
    nlohmann::json json_meta_data = almo::make_meta_data_json(meta_data);
    nlohmann::json json_ir = almo::make_json(asts);

    std::vector<std::string> args = {"-o", "-t", "-d", "-h"};

    // default
    std::string theme = "light";
    std::string out_path = "index.html";

    for (int i = 2; i < argc; i++){
        if (argv[i][0] == '-'){
            if (argv[i][1] == 'o'){
                out_path = argv[i + 1];
            }
            else if (argv[i][1] == 't'){
                theme = argv[i + 1];
            }
            else if (argv[i][1] == 'd'){
                std::cout << json_meta_data.dump(4) << std::endl;
                std::cout << json_ir.dump(4) << std::endl;
                return 0;
            }
            else if (argv[i][1] == 'h'){
                if (argc > 3){
                    std::cerr << "error: -h option is used, but other options are used." << std::endl;
                    return 1;
                }
                std::cout << "usage: almo [options] [input file]" << std::endl;
                std::cout << "options:" << std::endl;
                std::cout << "  -o [output file] : output file name" << std::endl;
                std::cout << "  -t [theme] : theme name" << std::endl;
                std::cout << "  -d : show debug info" << std::endl;
                std::cout << "  -h : show this help" << std::endl;
                return 0;
            }
        }
    }

    json_meta_data["theme"] = theme;
    almo::render(json_ir, json_meta_data, out_path);
    return 0;
}