#include <pybind11/pybind11.h>
#include <pybind11/stl.h> 

#include "almo.cpp"


std::string md_to_html(std::string md_content, std::map<std::string, std::string> meta_data) {
    almo::Block ast = almo::parse_rest(split(md_content, "\n"));
    return ast.render(meta_data);
}

namespace py = pybind11;

PYBIND11_MODULE(almo, m) {
    m.doc() = "almo library"; 

    m.def("md_to_html", &md_to_html, "md to html");
}