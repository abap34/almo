#include <emscripten.h>

#include <string>
#include <vector>

#include "render.hpp"

extern "C" {

EMSCRIPTEN_KEEPALIVE
const char* almo_render(const char* md, int len) {
    static std::string result_buf;
    std::string input(md, len);

    auto result = almo::Parser{}.parse(input);
    auto& ast = result.ast;

    almo::move_footnote_to_end(ast);

    result_buf = ast.to_html();
    return result_buf.c_str();
}

}
