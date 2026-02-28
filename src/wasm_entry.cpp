#include <emscripten.h>

#include <string>
#include <vector>

#include "render.hpp"

extern "C" {

EMSCRIPTEN_KEEPALIVE
const char* almo_render(const char* md, int len) {
    static std::string result_buf;
    std::string input(md, len);

    std::vector<std::string> lines = split(input, "\n");

    almo::Markdown ast;
    almo::MarkdownParser parser(lines);
    parser.process(ast);

    almo::move_footnote_to_end(ast);

    result_buf = ast.to_html();
    return result_buf.c_str();
}

}
