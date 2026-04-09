#pragma once

#include <vector>

#include "../support/almo_test_support.hpp"
#include "../support/test_documents.hpp"

namespace almo_test {

inline void register_render_tests(std::vector<TestCase>& tests) {
    tests.push_back({
        "loadlib and executable blocks",
        []() {
            const auto summary =
                summarize_markdown(loadlib_executable_document());

            expect_eq(summary.ast.nodes_byclass("LoadLib").size(),
                      static_cast<std::size_t>(1),
                      "LoadLib blocks should be parsed");
            expect_eq(summary.ast.nodes_byclass("ExecutableCodeBlock").size(),
                      static_cast<std::size_t>(1),
                      "Executable code blocks should be parsed");
            expect_contains(summary.html, "use_libs.push(\"numpy\")",
                            "LoadLib should push requested libraries");
            expect_contains(summary.html, "use_libs.push(\"matplotlib\")",
                            "LoadLib should push all requested libraries");
            expect_contains(summary.html, "pyodide.js",
                            "Executable code blocks should require Pyodide");
            expect_contains(summary.html, "runBlock(",
                            "Executable code blocks should render a run button");
        },
    });

    tests.push_back({
        "footnotes and summary layout",
        []() {
            const auto summary = summarize_markdown(footnote_document());
            const auto children = summary.ast.get_childs();

            expect_true(!children.empty(),
                        "Summary AST should contain top-level children");
            expect_eq(children.back()->get_classname(), std::string("DivBlock"),
                      "Footnotes should be moved into a trailing div");
            expect_eq(children.back()->get_properties().at("div_class"),
                      std::string("footnote"),
                      "Footnotes should be wrapped in the footnote container");
            expect_eq(children.back()->nodes_byclass("FootnoteDefinition").size(),
                      static_cast<std::size_t>(1),
                      "Footnote container should contain definitions");

            const auto body_pos = summary.html.find("Footnote ref");
            const auto footnote_pos = summary.html.find("class=\"footnote\"");
            expect_true(body_pos != std::string::npos &&
                            footnote_pos != std::string::npos,
                        "Rendered HTML should contain both body and footnote");
            expect_true(body_pos < footnote_pos,
                        "Footnotes should be moved to the end of the document");
        },
    });

    tests.push_back({
        "judge rendering",
        []() {
            const auto summary = summarize_markdown(judge_document());

            expect_eq(summary.ast.nodes_byclass("Judge").size(),
                      static_cast<std::size_t>(1),
                      "Judge blocks should be parsed");
            expect_contains(summary.html, "Run Sample",
                            "Judge blocks should render the sample runner button");
            expect_contains(summary.html, "Submit",
                            "Judge blocks should render the submit button");
            expect_contains(summary.html, "s = input()",
                            "Judge blocks should load the starter source code");
            expect_contains(summary.html, "all_input[",
                            "Judge blocks should embed full judge inputs");
            expect_contains(summary.html, "all_sample_output[",
                            "Judge blocks should embed sample outputs");
            expect_contains(summary.html, "judge_types[",
                            "Judge blocks should expose the judge type to JS");
            expect_contains(summary.html, "pyodide.js",
                            "Judge blocks should require Pyodide");
        },
    });

    tests.push_back({
        "template loading",
        []() {
            const std::string css_path = fixture_path("styles/custom.css");

            const std::string custom_html =
                almo::load_html_template("__default__", css_path, false);
            expect_contains(custom_html, "papayawhip",
                            "Custom CSS files should be embedded in the template");
            expect_not_contains(custom_html, "pyodide.js",
                                "Pyodide should be omitted when not required");

            const std::string dark_html =
                almo::load_html_template("__default__", "dark", true);
            expect_contains(dark_html, "pyodide.js",
                            "Pyodide should be injected when required");
            expect_contains(dark_html, "<style>",
                            "Builtin themes should embed CSS");

            auto meta = default_meta();
            const std::string replaced =
                almo::replace_template(custom_html, meta, "<p>body</p>");
            expect_contains(replaced, "<p>body</p>",
                            "Template replacement should inject rendered contents");
            expect_contains(replaced, "ALMO Test",
                            "Template replacement should inject metadata");
        },
    });
}

}  // namespace almo_test
