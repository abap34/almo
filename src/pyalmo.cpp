#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "almo.cpp"

// Macro to bind node
#define BIND_NODE(CLASS, BIND_CLASSNAME, CONSTRUCTOR_ARGS...)                \
    py::class_<CLASS, almo::ASTNode, std::shared_ptr<CLASS>>(m,              \
                                                             BIND_CLASSNAME) \
        .def(py::init<CONSTRUCTOR_ARGS>())

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

    m.def("parse", &parse_md, "Parse markdown to AST.");
    m.def("move_footnote_to_end", &almo::move_footnote_to_end,
          "Move footnote definition to the end of the document.");
    m.def("required_pyodide", &almo::required_pyodide,
          "Check if pyodide is required.");    

    py::class_<almo::ASTNode, std::shared_ptr<almo::ASTNode>>(
        m, "ASTNode", py::dynamic_attr())
        .def("to_html", &almo::ASTNode::to_html)
        .def("to_json", &almo::ASTNode::to_json)
        .def("get_properties", &almo::ASTNode::get_properties)
        .def("get_classname", &almo::ASTNode::get_classname)
        .def("get_uuid_str", &almo::ASTNode::get_uuid_str)
        .def("set_uuid", &almo::ASTNode::set_uuid)
        .def("pushback_child", &almo::ASTNode::pushback_child)
        .def("remove_child", &almo::ASTNode::remove_child)
        .def("move_node", &almo::ASTNode::move_node)
        .def("concatenated_childs_html",
             &almo::ASTNode::concatenated_childs_html)
        .def("nodes_byclass", &almo::ASTNode::nodes_byclass)
        .def_readwrite("childs", &almo::ASTNode::childs)
        .def("get_childs", &almo::ASTNode::get_childs)
        .def("__repr__", [](const almo::ASTNode &ast_node) {
            return "<almo." + ast_node.get_classname() +
                   " (uuid: " + ast_node.get_uuid_str() + ")>";
        });

    py::class_<almo::Markdown, almo::ASTNode, std::shared_ptr<almo::Markdown>>(
        m, "Markdown")
        .def(py::init<>())
        .def("to_html", &almo::Markdown::to_html)
        .def("to_dot", &almo::Markdown::to_dot)
        .def("to_json", &almo::Markdown::to_json)
        .def("__repr__", [](const almo::Markdown &md_ast) {
            return "<almo.Markdown (uuid: " + md_ast.get_uuid_str() + ")>";
        });

    // CLASS, CONSTRUCTOR_ARGS
    BIND_NODE(almo::CodeBlock, "CodeBlock", std::string, std::string);
    BIND_NODE(almo::DivBlock, "DivBlock", std::string);
    BIND_NODE(almo::EnumerateBlock, "EnumerateBlock");
    BIND_NODE(almo::ExecutableCodeBlock, "ExecutableCodeBlock", std::string,
              std::string);
    BIND_NODE(almo::FootnoteDefinition, "FootnoteDefinition", std::string);
    BIND_NODE(almo::Header, "Header", int);
    BIND_NODE(almo::HorizontalLine, "HorizontalLine");
    BIND_NODE(almo::InlineCodeBlock, "InlineCodeBlock", std::string);
    BIND_NODE(almo::InlineFootnoteReference, "InlineFootnoteReference",
              std::string);
    BIND_NODE(almo::InlineImage, "InlineImage", std::string, std::string);
    BIND_NODE(almo::InlineItalic, "InlineItalic");
    BIND_NODE(almo::InlineMath, "InlineMath", std::string);
    BIND_NODE(almo::InlineOverline, "InlineOverline");
    BIND_NODE(almo::InlineStrong, "InlineStrong");
    BIND_NODE(almo::InlineUrl, "InlineUrl", std::string, std::string);
    BIND_NODE(almo::Item, "Item");
    BIND_NODE(almo::Judge, "Judge", std::string, std::string, std::string,
              std::string, std::string, std::string, std::string, std::string);
    BIND_NODE(almo::ListBlock, "ListBlock");
    BIND_NODE(almo::LoadLib, "LoadLib", std::vector<std::string>);
    BIND_NODE(almo::MathBlock, "MathBlock", std::string);
    BIND_NODE(almo::NewLine, "NewLine");
    BIND_NODE(almo::Quote, "Quote");
    BIND_NODE(almo::RawText, "RawText", std::string);
    BIND_NODE(almo::Table, "Table",
              std::vector<std::shared_ptr<almo::Markdown>>, int, int,
              std::vector<std::string>, std::vector<int>);

#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif
}