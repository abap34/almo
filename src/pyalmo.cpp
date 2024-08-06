#include <pybind11/pybind11.h>
#include <pybind11/stl.h> 

#include "almo.cpp"

almo::Markdown parse_md(std::string md_content) {
    almo::Markdown ast;
    std::vector<std::string> content_splitted = split(md_content, "\n");
    almo::MarkdownParser parser(content_splitted);
    parser.process(ast);
    return ast;
}

namespace py = pybind11;

PYBIND11_MODULE(almo, m) {
    m.doc() = "almo interface for python.";

    m.def("parse_md", &parse_md, "Parse markdown to AST.");

    py::class_<almo::Markdown>(m, "Markdown")
        .def("to_html", &almo::Markdown::to_html)
        .def("to_dot", &almo::Markdown::to_dot)
        .def("to_json", &almo::Markdown::to_json)
        .def("nodes_byclass", &almo::Markdown::nodes_byclass);

    #ifdef VERSION_INFO
        m.attr("__version__") = VERSION_INFO;
    #else
        m.attr("__version__") = "dev";
    #endif
}