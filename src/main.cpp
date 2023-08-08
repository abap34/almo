#include "makejson.hpp"
#include "parse.hpp"
#include "render.hpp"
#include <iostream>
#include <fstream>

int main(int argc, char *argv[]){
    nlohmann::json json_ir = almo::make_json(almo::parse_md_file(argv[1]));
    almo::render(json_ir);
}