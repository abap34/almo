#include "makejson.hpp"
#include "parse.hpp"
#include "render.hpp"
#include <iostream>
#include <fstream>

int main(int argc, char *argv[]){
    auto [meta_data, asts] = almo::parse_md_file(argv[1]);
    nlohmann::json json_meta_data = almo::make_meta_data_json(meta_data);
    // if --theme option is specified, use it.
    if (argc == 4 && std::string(argv[2]) == "--theme"){
        json_meta_data["theme"] = argv[3];    
    } else {
        json_meta_data["theme"] = "dark";
    }
    nlohmann::json json_ir = almo::make_json(asts);
    almo::render(json_ir, json_meta_data);
}