#pragma once

#include <vector>

#include "benchmark_documents.hpp"
#include "benchmark_support.hpp"

namespace almo_test {

inline std::vector<BenchmarkScenario> make_benchmark_scenarios() {
    const auto small_document = build_basic_document(10);
    const auto large_document = build_basic_document(200);
    const auto inline_heavy_document = build_inline_heavy_document(120);
    const auto realistic_article_document = load_realistic_article_fixture_document();
    const auto release_notes_document = load_release_notes_fixture_document();
    const auto table_heavy_document = build_table_heavy_document(30, 24);
    const auto footnote_heavy_document = build_footnote_heavy_document(160);
    const auto structured_document = build_structured_document(60);
    const auto interactive_document = build_interactive_document(25);
    const auto judge_document = build_judge_document(8);

    return {
        make_parse_scenario("parse_small_document", small_document),
        make_parse_scenario("parse_large_document", large_document),
        make_parse_scenario("parse_realistic_article_document",
                            realistic_article_document),
        make_parse_scenario("parse_release_notes_document",
                            release_notes_document),
        make_parse_scenario("parse_inline_heavy_document",
                            inline_heavy_document),
        make_parse_scenario("parse_table_heavy_document", table_heavy_document),
        make_parse_scenario("parse_footnote_heavy_document",
                            footnote_heavy_document),
        make_parse_scenario("parse_structured_document", structured_document),
        make_parse_scenario("parse_judge_document", judge_document),
        make_render_scenario("render_small_document", small_document),
        make_render_scenario("render_large_document", large_document),
        make_render_scenario("render_realistic_article_document",
                             realistic_article_document),
        make_render_scenario("render_release_notes_document",
                             release_notes_document),
        make_render_scenario("render_structured_document", structured_document),
        make_render_scenario("render_table_heavy_document",
                             table_heavy_document),
        make_render_scenario("render_footnote_heavy_document",
                             footnote_heavy_document),
        make_render_scenario(
            "render_interactive_document", interactive_document,
            [](const almo::ParseSummary& summary) {
                if (summary.html.find("runBlock(") == std::string::npos) {
                    throw std::runtime_error(
                        "interactive HTML must include run buttons");
                }
            }),
        make_render_scenario(
            "render_judge_document", judge_document,
            [](const almo::ParseSummary& summary) {
                if (summary.html.find("Run Sample") == std::string::npos) {
                    throw std::runtime_error(
                        "judge HTML must include sample runners");
                }
            }),
        make_summary_scenario("summary_inline_heavy_document",
                              inline_heavy_document),
        make_summary_scenario("summary_realistic_article_document",
                              realistic_article_document),
        make_summary_scenario("summary_large_document", structured_document),
    };
}

}  // namespace almo_test
