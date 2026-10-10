#include <glob.h>

#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "parse.hpp"
#include "utils.hpp"

namespace almo {

namespace detail {

inline std::string replace_template_value(std::string text,
                                         const std::string& key,
                                         const std::string& value) {
    // Preserve the former regex API for unusual metadata keys and replacement
    // tokens such as $&, $1, $$, $` and $', including its handling of embedded
    // NUL bytes. Ordinary values need only a literal search.
    bool regex_format = key.find_first_of(R"(\.^$|()[]*+?{})") != key.npos ||
                        value.find('\0') != value.npos;
    for (std::size_t i = value.find('$'); !regex_format && i != value.npos;
         i = value.find('$', i + 1)) {
        if (i + 1 < value.size()) {
            const char next = value[i + 1];
            regex_format = next == '$' || next == '&' || next == '`' || next == '\'' ||
                           (next >= '0' && next <= '9');
        }
    }
    if (regex_format)
        return std::regex_replace(text, std::regex("\\{\\{" + key + "\\}\\}"), value);
    const auto token = "{{" + key + "}}";
    auto position = text.find(token);
    if (position == text.npos) return text;
    std::string result;
    result.reserve(text.size());
    std::size_t previous = 0;
    do {
        result.append(text, previous, position - previous);
        result += value;
        previous = position + token.size();
        position = text.find(token, previous);
    } while (position != text.npos);
    result.append(text, previous, text.npos);
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

}  // namespace detail

std::string load_html_template(std::string html_path, std::string css_setting,
                               bool required_pyodide) {
    const std::string pyodide_loader =
        "<script "
        "src=\"https://cdn.jsdelivr.net/pyodide/v0.24.0/full/pyodide.js\"></"
        "script>";

    std::string html;

    if (html_path == "__default__") {
        html = TEMPLATE;
    } else {
        html = join(read_file(html_path), "\n");
    }

    std::string result;
    if (css_setting == "light") {
        std::string css = "<style>" + LIGHT_THEME + "</style>";
        result = detail::replace_template_value(std::move(html), "style", css);
    } else if (css_setting == "dark") {
        std::string css = "<style>" + DARK_THEME + "</style>";
        result = detail::replace_template_value(std::move(html), "style", css);
    } else if (css_setting.ends_with(".css")) {
        std::string css =
            "<style>" + join(read_file(css_setting), "\n") + "</style>";

        result = detail::replace_template_value(std::move(html), "style", css);
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
    // runner を挿入
    result = detail::replace_template_value(std::move(result), "runner", runner);

    return result;
}

std::string replace_template(std::string html_template,
                             std::map<std::string, std::string> meta_data,
                             std::string content) {
    std::string output_html = std::move(html_template);

    for (const auto& [key, value] : meta_data) {
        output_html = detail::replace_template_value(std::move(output_html), key, value);
    }

    std::string syntax_theme = meta_data["syntax_theme"];

    output_html = detail::replace_template_value(std::move(output_html), "syntax_theme", syntax_theme);
    output_html = detail::replace_template_value(std::move(output_html), "contents", content);

    return output_html;
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
    std::string content = ast.to_html();

    std::string html_template =
        load_html_template(meta_data["template_file"], meta_data["css_setting"],
                           required_pyodide(ast));

    std::string output_html =
        replace_template(html_template, meta_data, content);

    return output_html;
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
