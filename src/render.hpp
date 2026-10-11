#include <glob.h>

#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "parse.hpp"
#include "utils.hpp"

namespace almo {

namespace detail {

inline std::string replace_template_values(
    std::string_view text, const std::map<std::string, std::string>& values) {
    // Scan only the original template. Inserted bytes are never interpreted as
    // replacement syntax or as another template placeholder.
    std::string result;
    result.reserve(text.size());
    std::size_t cursor = 0;
    while (cursor < text.size()) {
        const auto begin = text.find("{{", cursor);
        if (begin == text.npos) break;
        const auto end = text.find("}}", begin + 2);
        if (end == text.npos) break;
        result.append(text.substr(cursor, begin - cursor));
        const auto value = values.find(std::string(text.substr(begin + 2, end - begin - 2)));
        if (value == values.end())
            result.append(text.substr(begin, end + 2 - begin));
        else
            result += value->second;
        cursor = end + 2;
    }
    result.append(text.substr(cursor));
    return result;
}

inline void extract_footnotes(ASTNode& node, ASTNode& footnotes) {
    std::size_t kept = 0;
    for (std::size_t i = 0; i < node.childs.size(); ++i) {
        auto& child = node.childs[i];
        const bool is_footnote = child->get_classname() == "FootnoteDefinition";
        // Preorder matches nodes_byclass(), including nested definitions in
        // caller-built ASTs. Compact each child list once instead of searching
        // the whole document and shifting its children for every definition.
        if (is_footnote) footnotes.pushback_child(child);
        extract_footnotes(*child, footnotes);
        if (!is_footnote) {
            if (kept != i) node.childs[kept] = std::move(child);
            ++kept;
        }
    }
    node.childs.resize(kept);
}

inline std::string read_html_template(const std::string& html_path) {
    return html_path == "__default__" ? TEMPLATE : join(read_file(html_path), "\n");
}

inline void add_template_assets(std::map<std::string, std::string>& values,
                                const std::string& css_setting, bool required_pyodide) {
    const std::string pyodide_loader =
        "<script "
        "src=\"https://cdn.jsdelivr.net/pyodide/v0.24.0/full/pyodide.js\"></"
        "script>";

    if (css_setting == "light") {
        values["style"] = "<style>" + LIGHT_THEME + "</style>";
    } else if (css_setting == "dark") {
        values["style"] = "<style>" + DARK_THEME + "</style>";
    } else if (css_setting.ends_with(".css")) {
        values["style"] =
            "<style>" + join(read_file(css_setting), "\n") + "</style>";
    } else {
        throw InvalidCommandLineArgumentsError(
            "不正なCSSの設定です。 `light`, `dark` もしくは `.css` "
            "で終了するファイル名を指定してください。");
    }

    std::string runner = "<script>" + RUNNER + "</script>";

    if (required_pyodide) {
        // runnner の先頭に　　pyodide を挿入
        runner = pyodide_loader + runner;
    } else {
        runner = "<!-- Runner is not required. Skip this. -->";
    }
    values["runner"] = std::move(runner);
}

}  // namespace detail

std::string load_html_template(std::string html_path, std::string css_setting,
                               bool required_pyodide) {
    std::map<std::string, std::string> values;
    detail::add_template_assets(values, css_setting, required_pyodide);
    return detail::replace_template_values(detail::read_html_template(html_path), values);
}

std::string replace_template(std::string html_template,
                             std::map<std::string, std::string> meta_data,
                             std::string content) {
    meta_data.try_emplace("syntax_theme", "");
    meta_data["contents"] = std::move(content);
    return detail::replace_template_values(html_template, meta_data);
}

void move_footnote_to_end(Markdown& ast) {
    std::shared_ptr<DivBlock> footnote_div =
        std::make_shared<DivBlock>("footnote");
    detail::extract_footnotes(ast, *footnote_div);
    ast.pushback_child(footnote_div);
}

bool required_pyodide(Markdown& ast) {
    if (ast.nodes_byclass("ExecutableCodeBlock").size() > 0) {
        return true;
    }

    if (ast.nodes_byclass("Judge").size() > 0) {
        return true;
    }

    return false;
}

std::string render(Markdown ast, std::map<std::string, std::string> meta_data) {
    auto html_template = detail::read_html_template(meta_data["template_file"]);
    // Assets and document data are inserted together so placeholders inside
    // CSS or JavaScript cannot be substituted during a later rendering pass.
    detail::add_template_assets(meta_data, meta_data["css_setting"], required_pyodide(ast));
    return replace_template(std::move(html_template), std::move(meta_data), ast.to_html());
}

// Call from python to get html from lines without metadata `md_content` and
// metadata `meta_data`.
//
// - md_to_html
// - md_to_json
// - md_to_dot
// - md_to_ast
// - md_to_summary
//
// note : meta_data may change

std::string md_to_html(const std::vector<std::string>& md_content,
                       std::map<std::string, std::string>& meta_data) {
    Markdown ast;
    MarkdownParser parser(md_content);
    parser.process(ast);
    return render(ast, meta_data);
}

std::string md_to_json(const std::vector<std::string>& md_content,
                       std::map<std::string, std::string>& meta_data) {
    Markdown ast;
    MarkdownParser parser(md_content);
    parser.process(ast);
    return ast.to_json();
}

std::string md_to_dot(const std::vector<std::string>& md_content,
                      std::map<std::string, std::string>& meta_data) {
    Markdown ast;
    MarkdownParser parser(md_content);
    parser.process(ast);
    std::string dot = ast.to_dot();
    dot = "digraph G {\n graph [labelloc=\"t\"; \n ]\n" + dot + "}";
    return dot;
}

Markdown md_to_ast(const std::vector<std::string>& md_content,
                   std::map<std::string, std::string>& meta_data) {
    Markdown ast;
    MarkdownParser parser(md_content);
    parser.process(ast);
    return ast;
}

struct ParseSummary {
    Markdown ast;
    std::string html, json, dot;
};

ParseSummary md_to_summary(const std::vector<std::string>& md_content,
                           std::map<std::string, std::string>& meta_data) {
    Markdown ast;
    MarkdownParser parser(md_content);
    parser.process(ast);

    move_footnote_to_end(ast);

    ParseSummary summary = {
        .ast = ast,
        .html = render(ast, meta_data),
        .json = ast.to_json(),
        .dot =
            "digraph G {\n graph [labelloc=\"t\"; \n ]\n" + ast.to_dot() + "}",
    };
    return summary;
}

}  // namespace almo
