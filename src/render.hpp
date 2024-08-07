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
std::string LIGHT_THEME = {
#include "light.css"
};
std::string DARK_THEME = {
#include "dark.css"
};

std::string RUNNER = {
#include "runner.js"
};

std::string TEMPLATE = {
#include "template.html"
};

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
        result = std::regex_replace(html, std::regex("\\{\\{style\\}\\}"), css);
    } else if (css_setting == "dark") {
        std::string css = "<style>" + DARK_THEME + "</style>";
        result = std::regex_replace(html, std::regex("\\{\\{style\\}\\}"), css);
    } else if (css_setting.ends_with(".css")) {
        std::string css =
            "<style>" + join(read_file(css_setting), "\n") + "</style>";

        result = std::regex_replace(html, std::regex("\\{\\{style\\}\\}"), css);
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
    result =
        std::regex_replace(result, std::regex("\\{\\{runner\\}\\}"), runner);

    return result;
}

std::string replace_template(std::string html_template,
                             std::map<std::string, std::string> meta_data,
                             std::string content) {
    std::string output_html = html_template;

    for (auto [key, value] : meta_data) {
        std::string replace_key = "\\{\\{" + key + "\\}\\}";
        output_html =
            std::regex_replace(output_html, std::regex(replace_key), value);
    }

    std::string syntax_theme = meta_data["syntax_theme"];

    output_html = std::regex_replace(
        output_html, std::regex("\\{\\{syntax_theme\\}\\}"), syntax_theme);
    output_html = std::regex_replace(
        output_html, std::regex("\\{\\{contents\\}\\}"), content);

    return output_html;
}

void move_footnote_to_end(Markdown& ast) {
    std::vector<std::shared_ptr<ASTNode>> footnote_defs =
        ast.nodes_byclass("FootnoteDefinition");

    std::shared_ptr<DivBlock> footnote_div =
        std::make_shared<DivBlock>("footnote");

    ast.pushback_child(footnote_div);

    for (auto node : footnote_defs) {
        ast.move_node(node, footnote_div);
    }
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
    std::vector<std::shared_ptr<ASTNode>> footnote_defs =
        ast.nodes_byclass("FootnoteDefinition");

    std::shared_ptr<DivBlock> footnote_div =
        std::make_shared<DivBlock>("footnote");

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

    render(ast, meta_data);
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
