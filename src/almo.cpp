#include "makejson.hpp"
#include "parse.hpp"
#include "render.hpp"
#include <iostream>
#include <fstream>

int main(int argc, char *argv[]){
    auto [meta_data, asts] = almo::parse_md_file(argv[1]);
    nlohmann::json json_meta_data = almo::make_meta_data_json(meta_data);
    nlohmann::json json_ir = almo::make_json(asts);
    almo::render(json_ir);
}