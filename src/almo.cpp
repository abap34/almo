#include "makejson.hpp"
#include "parse.hpp"
#include "render.hpp"
#include <iostream>
#include <fstream>
#include <string>

int main(int argc, char *argv[]){
    auto [meta_data, asts] = almo::parse_md_file(argv[1]);
    nlohmann::json json_meta_data = almo::make_meta_data_json(meta_data);
    nlohmann::json json_ir = almo::make_json(asts);

    // default
    std::string theme = "light";
    std::string out_path = "index.html";
    bool debug = false;

    for (int i = 2; i < argc; i++){
        if (argv[i][0] == '-'){
            if (strlen(argv[i]) > 3){
                std::cerr << "Unkown command line arguments is passed." << std::endl;
                std::cerr << "Please check the option by `almo -h`" << std::endl;
                exit(1);
            }
            if (argv[i][1] == 'o'){
                out_path = argv[i + 1];
            }
            else if (argv[i][1] == 't'){
                theme = argv[i + 1];
            }
            else if (argv[i][1] == 'd'){
                debug = true;
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
            else {
                std::cerr << "Unkown command line arguments is passed." << std::endl;
                std::cerr << "Please check the option by `almo -h`" << std::endl;
                exit(1);
            }
        }
    }

    json_meta_data["theme"] = theme;
    json_meta_data["out_path"] = out_path;

    if (debug) {
        nlohmann::json all_data;
        all_data["meta"] = json_meta_data;
        all_data["ir"] = json_ir;
        std::cout << all_data.dump(4) << std::endl;
    }

    almo::render(json_ir, json_meta_data, out_path);
    return 0;
}