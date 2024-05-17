#include <pybind11/pybind11.h>
#include <pybind11/stl.h> 

#include "almo.cpp"


std::string md_to_html(std::string md_content, std::map<std::string, std::string> meta_data) {
    almo::Block ast = almo::parse_rest(split(md_content, "\n"));
    return ast.render(meta_data);
}


std::string md_to_json(std::string md_content) {
    almo::Block ast = almo::parse_rest(split(md_content, "\n"));
    return ast.to_json();
}


std::string md_to_dot(std::string md_content) {
    almo::Block ast = almo::parse_rest(split(md_content, "\n"));
    return ast.to_dot(true);
}


namespace py = pybind11;


PYBIND11_MODULE(almo, m) {
    m.doc() = "almo interface for python.";

    m.def("md_to_html", &md_to_html, "md to html");
    m.def("md_to_json", &md_to_json, "md to json");
    m.def("md_to_dot", &md_to_dot, "md to dot");


    #ifdef VERSION_INFO
        m.attr("__version__") = VERSION_INFO;
    #else
        m.attr("__version__") = "dev";
    #endif
}