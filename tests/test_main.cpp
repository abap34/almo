#include <vector>

#include "cases/error_tests.hpp"
#include "cases/parser_tests.hpp"
#include "cases/parser_api_tests.hpp"
#include "cases/incremental_parser_tests.hpp"
#include "cases/syntax_match_tests.hpp"
#include "cases/reader_utils_tests.hpp"
#include "cases/render_tests.hpp"
#include "cases/dollar_tests.hpp"
#include "support/framework.hpp"

int main() {
    std::vector<almo_test::TestCase> tests;

    almo_test::register_reader_and_utils_tests(tests);
    almo_test::register_parser_tests(tests);
    almo_test::register_render_tests(tests);
    almo_test::register_error_tests(tests);
    almo_test::register_syntax_match_tests(tests);
    almo_test::register_parser_api_tests(tests);
    almo_test::register_incremental_parser_tests(tests);
    almo_test::register_dollar_tests(tests);

    return almo_test::run_tests(tests);
}
