#include <pybind11/pybind11.h>
#include <pybind11/stl.h> 

#include "almo.cpp"


std::string md_to_html(std::vector<std::string> md_content, std::map<std::string, std::string> meta_data) {
    almo::Markdown ast;
    almo::MarkdownParser parser(md_content, meta_data);
    parser.process(ast);
    return ast.to_html();
}

almo::Markdown parse_md(std::vector<std::string> md_content, std::map<std::string, std::string> meta_data) {
    almo::Markdown ast;
    almo::MarkdownParser parser(md_content, meta_data);
    parser.process(ast);
    return ast;
}

namespace py = pybind11;


PYBIND11_MODULE(almo, m) {
    m.doc() = "almo interface for python.";

    m.def("md_to_html", &md_to_html, "Convert markdown to html.");
    m.def("parse_md", &parse_md, "Parse markdown to AST.");

    py::class_<almo::Markdown>(m, "Markdown")
        .def("to_html", &almo::Markdown::to_html)
        .def("to_dot", &almo::Markdown::to_dot)
        .def("to_json", &almo::Markdown::to_json);

    #ifdef VERSION_INFO
        m.attr("__version__") = VERSION_INFO;
    #else
        m.attr("__version__") = "dev";
    #endif
}