#include "makejson.hpp"
#include "parse.hpp"

int main(){
    almo::make_json(almo::parse_md_file("example.md"));
}